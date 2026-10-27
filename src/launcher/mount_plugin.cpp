#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "LauncherConnection.h"
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include "MinHook.h"

static HMODULE plugin;
static volatile LONG started=0;
static bool LauncherSessionActive() { return ConnectLauncherSession(); }
static wchar_t directory[32768];
static volatile LONG lastPatchState=-1;
static ULONGLONG diagnosticStart=0;
static volatile LONG automaticRequested=0;
static SRWLOCK stateLock=SRWLOCK_INIT;
struct ManagerState { void* manager; bool sawMount; };
static ManagerState managers[16]={};
using PatchTick=bool(__fastcall*)(void*);
static PatchTick originalPatchTick=nullptr;
using SetPatchState=void(__fastcall*)(void*,const void*);
static SetPatchState originalStateSetter=nullptr;

static void Log(const char* message) {
    wchar_t path[32768];
    if(swprintf_s(path,L"%sstardive-mount.log",directory)<0) return;
    HANDLE h=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
                         nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(h==INVALID_HANDLE_VALUE) return;
    DWORD written;
    WriteFile(h,message,static_cast<DWORD>(strlen(message)),&written,nullptr);
    CloseHandle(h);
}

static bool Read(const void* address,void* output,SIZE_T size) {
    MEMORY_BASIC_INFORMATION info;
    if(!VirtualQuery(address,&info,sizeof(info)) || info.State!=MEM_COMMIT ||
       (info.Protect&(PAGE_GUARD|PAGE_NOACCESS)) ||
       reinterpret_cast<uintptr_t>(address)+size >
       reinterpret_cast<uintptr_t>(info.BaseAddress)+info.RegionSize) return false;
    __try { memcpy(output,address,size);return true; }
    __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static const char* StateName(unsigned char state) {
    const char* names[]={"NONE","CONFIG_REQUEST","MANIFEST_LIST_REQUEST",
        "CONTENT_REQUEST","CONTENT_INSTALL_READY","CONTENT_INSTALL",
        "CONTENT_MOUNT","SUCCESSED","FAILED"};
    return state<9?names[state]:"UNKNOWN";
}

static void RecordPatchState(void* manager) {
    unsigned char state=255;
    if(!Read(static_cast<unsigned char*>(manager)+0x390,&state,1))return;
    LONG previous=InterlockedExchange(&lastPatchState,state);
    if(previous!=state) {
        char line[180];
        sprintf_s(line,"PATCH_STATE: t=%llu ms, manager=%p, value=%u, name=%s\r\n",
            GetTickCount64()-diagnosticStart,manager,unsigned(state),StateName(state));
        Log(line);
    }
}

static bool __fastcall ObservePatchTick(void* manager) {
    RecordPatchState(manager);
    bool result=originalPatchTick(manager);
    RecordPatchState(manager);
    return result;
}

static void UpdateCompletion(void* manager,unsigned char state) {
    bool request=false;
    AcquireSRWLockExclusive(&stateLock);
    ManagerState* slot=nullptr;
    for(auto& candidate:managers)if(candidate.manager==manager){slot=&candidate;break;}
    if(!slot)for(auto& candidate:managers)if(!candidate.manager){candidate.manager=manager;slot=&candidate;break;}
    if(slot) {
        if(state<6 || state==8)slot->sawMount=false;
        if(state==6)slot->sawMount=true;
        if(state==7 && slot->sawMount) { request=true;slot->sawMount=false; }
    }
    ReleaseSRWLockExclusive(&stateLock);
    if(request && !InterlockedCompareExchange(&automaticRequested,1,0))
        Log("AUTO_TRIGGER: same patch manager completed CONTENT_MOUNT -> SUCCESSED.\r\n");
}

static void __fastcall ObserveStateSetter(void* manager,const void* info) {
    unsigned char requested=255;
    Read(info,&requested,1);
    originalStateSetter(manager,info);
    RecordPatchState(manager);
    unsigned char actual=255;
    if(Read(static_cast<unsigned char*>(manager)+0x390,&actual,1) && actual==requested)
        UpdateCompletion(manager,actual);
}

static void InstallObserver(unsigned char* base) {
    const unsigned char tickPrefix[]={0x40,0x53,0x55,0x41,0x57,0x48,0x83,0xec,
        0x30,0x0f,0xb6,0x91,0x90,0x03,0x00,0x00};
    unsigned char code[sizeof(tickPrefix)]={};
    if(!Read(base+0x72b14c0,code,sizeof(code)) || memcmp(code,tickPrefix,sizeof(code))) {
        Log("OBSERVER_REJECTED: status-tick signature differs. F8 remains available.\r\n");return;
    }
    const unsigned char setterPrefix[]={0x48,0x89,0x5c,0x24,0x10,0x48,0x89,0x6c,0x24,0x18,
        0x48,0x89,0x74,0x24,0x20,0x57,0x48,0x83,0xec,0x20};
    unsigned char setterCode[sizeof(setterPrefix)]={};
    if(!Read(base+0x72b6220,setterCode,sizeof(setterCode)) || memcmp(setterCode,setterPrefix,sizeof(setterCode))) {
        Log("OBSERVER_REJECTED: state-setter signature differs. F8 remains available.\r\n");return;
    }
    MH_STATUS status=MH_Initialize();
    if(status==MH_OK || status==MH_ERROR_ALREADY_INITIALIZED) {
        status=MH_CreateHook(base+0x72b14c0,reinterpret_cast<void*>(&ObservePatchTick),
                            reinterpret_cast<void**>(&originalPatchTick));
        if(status==MH_OK)status=MH_CreateHook(base+0x72b6220,reinterpret_cast<void*>(&ObserveStateSetter),
                                             reinterpret_cast<void**>(&originalStateSetter));
        if(status==MH_OK)status=MH_QueueEnableHook(base+0x72b14c0);
        if(status==MH_OK)status=MH_QueueEnableHook(base+0x72b6220);
        if(status==MH_OK)status=MH_ApplyQueued();
    }
    if(status!=MH_OK) {
        char line[160];sprintf_s(line,"OBSERVER_ERROR: %s; F8 remains available.\r\n",MH_StatusToString(status));Log(line);
    } else Log("OBSERVER_READY: automatic loading armed for verified content-mount completion; F8 fallback available.\r\n");
}

static bool VerifyBuild(unsigned char* base) {
    __try {
        auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        if(dos->e_magic!=IMAGE_DOS_SIGNATURE) return false;
        auto nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
        if(nt->Signature!=IMAGE_NT_SIGNATURE || nt->FileHeader.Machine!=IMAGE_FILE_MACHINE_AMD64 ||
           nt->FileHeader.TimeDateStamp!=2634424296u || nt->OptionalHeader.SizeOfImage!=235642880u)
            return false;
        const unsigned char prefix[]={0x40,0x55,0x53,0x56,0x57,0x41,0x54,0x41,
            0x55,0x41,0x56,0x41,0x57,0x48,0x8d,0xac,0x24,0x58,0xfc,0xff,
            0xff,0x48,0x81,0xec,0xa8,0x04,0x00,0x00,0x48,0x8b,0x05,0x9d};
        const unsigned char nameAccessor[]={0x48,0x8d,0x05,0x69,0xdf,0x3f,0x07,0xc3};
        return !memcmp(base+0x36fce40,prefix,sizeof(prefix)) &&
               !memcmp(base+0x36ed0f0,nameAccessor,sizeof(nameAccessor));
    } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static void* FindPak(unsigned char* base) {
    void* object=nullptr;
    if(!Read(base+0xd383a68,&object,sizeof(object)))return nullptr;
    for(unsigned depth=0;object && depth<16;++depth) {
        uintptr_t vtable=0;
        if(!Read(object,&vtable,sizeof(vtable))) return nullptr;
        if(vtable==reinterpret_cast<uintptr_t>(base+0xada42c0)) {
            uintptr_t nameFn=0;
            if(!Read(reinterpret_cast<void*>(vtable+0x70),&nameFn,sizeof(nameFn)) ||
               nameFn!=reinterpret_cast<uintptr_t>(base+0x36ed0f0)) return nullptr;
            return object;
        }
        // Traverse only a positively identified, trivial GetLowerLevel getter.
        uintptr_t lowerFn=0;
        unsigned char code[5]={};
        const unsigned char expected[]={0x48,0x8b,0x41,0x08,0xc3};
        if(!Read(reinterpret_cast<void*>(vtable+0x60),&lowerFn,sizeof(lowerFn)) ||
           !Read(reinterpret_cast<void*>(lowerFn),code,sizeof(code)) ||
           memcmp(code,expected,sizeof(code))) return nullptr;
        if(!Read(static_cast<unsigned char*>(object)+8,&object,sizeof(object)))return nullptr;
    }
    return nullptr;
}

static bool MountOnce(unsigned char* base,void* pak,const wchar_t* path,DWORD* exception) {
    // Verified against the game's mount-delegate caller and UE 5.5 source.
    using Mount=bool(__fastcall*)(void*,const wchar_t*,uint32_t,const wchar_t*,bool,void*);
    __try {
        return reinterpret_cast<Mount>(base+0x36fce40)(pak,path,10000,nullptr,true,nullptr);
    } __except((*exception=GetExceptionCode()),EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static void CollectMods(const std::wstring& root,const std::wstring& relative,std::vector<std::wstring>& names) {
    WIN32_FIND_DATAW entry;
    HANDLE search=FindFirstFileW((root+relative+L"*").c_str(),&entry);
    if(search==INVALID_HANDLE_VALUE)return;
    do {
        if(!wcscmp(entry.cFileName,L".")||!wcscmp(entry.cFileName,L".."))continue;
        if(entry.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)continue;
        std::wstring name=relative+entry.cFileName;
        if(entry.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)CollectMods(root,name+L"\\",names);
        else if(name.size()>=6&&!_wcsicmp(name.c_str()+name.size()-6,L"_P.pak"))names.push_back(name);
    }while(FindNextFileW(search,&entry));
    FindClose(search);
}
static DWORD WINAPI Worker(void*) {
    wchar_t exe[32768];
    GetModuleFileNameW(nullptr,exe,32768);
    const wchar_t* exeName=wcsrchr(exe,L'\\');exeName=exeName?exeName+1:exe;
    if(_wcsicmp(exeName,L"BigCat-Win64-Shipping.exe")) return 0;
    diagnosticStart=GetTickCount64();
    Log("\r\nPLUGIN_STARTED: StardiveMount 1.8.3 compatibility update, launcher session required, recursive mods x64\r\n");
    if(!LauncherSessionActive()){Log("LAUNCHER_SESSION_REQUIRED: start through Star Dive Mod Launcher; no hooks or mounts enabled.\r\n");return 0;}
    unsigned char* base=reinterpret_cast<unsigned char*>(GetModuleHandleW(nullptr));
    if(!VerifyBuild(base)) { Log("BUILD_REJECTED: executable guards do not match; no engine calls made.\r\n");return 0; }
    Log("BUILD_VERIFIED: target executable and mount function signatures match.\r\n");
    InstallObserver(base);
    Log("WAITING_FOR_COMPLETION: automatic trigger armed; press F8 only as a fallback.\r\n");
    bool released=false;
    for(;;) {
        if(InterlockedCompareExchange(&automaticRequested,0,0))break;
        SHORT key=GetAsyncKeyState(VK_F8);
        if(!(key&0x8000))released=true;
        if(released && (key&0x8000)){Log("MANUAL_TRIGGER: F8 fallback.\r\n");break;}
        Sleep(10);
    }
    if(!LauncherSessionActive()){Log("LAUNCHER_SESSION_ENDED: no mod mounts attempted.\r\n");return 0;}
    void* pak=FindPak(base);
    if(!pak) { Log("PAK_OBJECT_NOT_FOUND: platform-file chain could not be verified; no mount attempted.\r\n");return 0; }
    Log("PAK_OBJECT_VERIFIED: exact PakFile vtable and name accessor match.\r\n");
    std::vector<std::wstring> names;
    CollectMods(std::wstring(directory)+L"Mods\\",L"",names);
    if(names.empty()) { Log("MODS_NOT_FOUND: no *_P.pak files in Mods or its subfolders.\r\n");return 0; }
    std::sort(names.begin(),names.end(),[](const std::wstring& a,const std::wstring& b){return _wcsicmp(a.c_str(),b.c_str())<0;});
    unsigned mounted=0,skipped=0,failed=0;
    for(const auto& name:names) {
        std::wstring path=std::wstring(directory)+L"Mods\\"+name;
        std::wstring stem=path.substr(0,path.size()-4);
        bool complete=true;
        for(const wchar_t* ext:{L".utoc",L".ucas"}) {
            DWORD attributes=GetFileAttributesW((stem+ext).c_str());
            if(attributes==INVALID_FILE_ATTRIBUTES||(attributes&FILE_ATTRIBUTE_DIRECTORY))complete=false;
        }
        char line[2048];
        if(!complete) {
            sprintf_s(line,"MOD_SKIPPED: %ls missing .utoc or .ucas companion.\r\n",name.c_str());Log(line);++skipped;continue;
        }
        sprintf_s(line,"MOUNT_ATTEMPT: %ls, requested priority 10000.\r\n",name.c_str());Log(line);
        DWORD exception=0;
        bool result=MountOnce(base,pak,path.c_str(),&exception);
        if(exception) {
            sprintf_s(line,"MOUNT_EXCEPTION: %ls 0x%08lx; stopping further mounts.\r\n",name.c_str(),exception);Log(line);++failed;break;
        }
        sprintf_s(line,"MOUNT_RESULT: %s (%ls).\r\n",result?"true":"false",name.c_str());Log(line);
        if(result)++mounted;else ++failed;
    }
    char summary[160];sprintf_s(summary,"MOD_SUMMARY: mounted=%u skipped=%u failed=%u.\r\n",mounted,skipped,failed);Log(summary);
    return 0;
}

extern "C" __declspec(dllexport) BOOL WINAPI StartStardivePlugin() {
    if(InterlockedCompareExchange(&started,1,0))return TRUE;
    DWORD n=GetModuleFileNameW(nullptr,directory,32768);
    if(!n||n>=32768)return FALSE;
    while(n && directory[n-1]!=L'\\')--n;
    directory[n]=0;
    HANDLE thread=CreateThread(nullptr,0,Worker,nullptr,0,nullptr);
    if(!thread)return FALSE;
    CloseHandle(thread);return TRUE;
}

BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID) {
    if(reason==DLL_PROCESS_ATTACH)plugin=module;
    return TRUE;
}

#ifdef STARDIVE_OBSERVER_TEST
static __declspec(noinline) void __fastcall TestSetState(void* object,const void* info) {
    *static_cast<volatile unsigned char*>(static_cast<void*>(static_cast<unsigned char*>(object)+0x390))=
        *static_cast<const unsigned char*>(info);
}
static __declspec(noinline) bool __fastcall TestTick(void* object) {
    auto state=static_cast<unsigned char*>(object)+0x390;
    *state=static_cast<unsigned char>(*state+1);
    return *state<7;
}
int wmain() {
    plugin=GetModuleHandleW(nullptr);
    DWORD n=GetModuleFileNameW(plugin,directory,32768);
    while(n && directory[n-1]!=L'\\')--n;
    directory[n]=0;
    diagnosticStart=GetTickCount64();
    unsigned char manager[0x400]={};
    if(MH_Initialize()!=MH_OK)return 1;
    if(MH_CreateHook(reinterpret_cast<void*>(&TestTick),reinterpret_cast<void*>(&ObservePatchTick),
                      reinterpret_cast<void**>(&originalPatchTick))!=MH_OK)return 2;
    if(MH_EnableHook(reinterpret_cast<void*>(&TestTick))!=MH_OK)return 3;
    using Call=bool(__fastcall*)(void*);
    volatile Call invoke=&TestTick;
    for(unsigned i=0;i<8;++i) {
        bool value=invoke(manager);
        if(manager[0x390]!=i+1 || value!=((i+1)<7))return 4;
    }
    if(MH_DisableHook(reinterpret_cast<void*>(&TestTick))!=MH_OK)return 5;
    if(MH_CreateHook(reinterpret_cast<void*>(&TestSetState),reinterpret_cast<void*>(&ObserveStateSetter),
                    reinterpret_cast<void**>(&originalStateSetter))!=MH_OK)return 7;
    if(MH_EnableHook(reinterpret_cast<void*>(&TestSetState))!=MH_OK)return 8;
    unsigned char otherManager[0x400]={};
    using Setter=void(__fastcall*)(void*,const void*);
    volatile Setter set=&TestSetState;
    unsigned char state=6;set(manager,&state);
    state=7;set(otherManager,&state);
    if(automaticRequested)return 9; // Another manager's success must not trigger.
    state=8;set(manager,&state);
    state=7;set(manager,&state);
    if(automaticRequested)return 10; // Failure must disarm the mount sequence.
    state=1;set(manager,&state);
    state=6;set(manager,&state);
    state=7;set(manager,&state);
    if(automaticRequested!=1 || manager[0x390]!=7)return 11;
    if(MH_DisableHook(reinterpret_cast<void*>(&TestSetState))!=MH_OK)return 12;
    if(MH_Uninitialize()!=MH_OK)return 6;
    std::printf("Observer and automatic-trigger tests passed: original behavior preserved; only same-manager mount-to-success triggers.\n");
    return 0;
}
#endif
#ifdef STARDIVE_SESSION_TEST
int wmain(int argc,wchar_t** argv) {
    if(argc<2)return 2;
    bool expected=!wcscmp(argv[1],L"active");
    if(LauncherSessionActive()!=expected)return 3;
    if(argc==3){std::vector<std::wstring> names;CollectMods(std::wstring(argv[2])+L"\\",L"",names);if(names.size()!=2)return 4;}
    return 0;
}
#endif


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "LauncherConnection.h"
#include <cstdint>
#include <cstdio>

static HMODULE selfModule;
static INIT_ONCE once = INIT_ONCE_STATIC_INIT;
static FARPROC targets[17];
static bool CameraLauncherSessionActive() { return ConnectLauncherSession(); }
static const char* names[17] = {"GetFileVersionInfoA", "GetFileVersionInfoW", "GetFileVersionInfoExA", "GetFileVersionInfoExW", "GetFileVersionInfoSizeA", "GetFileVersionInfoSizeW", "GetFileVersionInfoSizeExA", "GetFileVersionInfoSizeExW", "VerFindFileA", "VerFindFileW", "VerInstallFileA", "VerInstallFileW", "VerLanguageNameA", "VerLanguageNameW", "VerQueryValueA", "VerQueryValueW", "GetFileVersionInfoByHandle"};

static void Log(const char* message) {
    wchar_t path[32768];
    DWORD length = GetModuleFileNameW(selfModule, path, 32768);
    if (!length || length >= 32768) return;
    DWORD separator = length;
    while (separator && path[separator - 1] != L'\\') --separator;
    const wchar_t file[] = L"stardive-loader-probe.log";
    if (separator + sizeof(file)/sizeof(wchar_t) >= 32768) return;
    for (DWORD i = 0; i < sizeof(file)/sizeof(wchar_t); ++i) path[separator+i] = file[i];
    HANDLE handle = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                               nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) return;
    DWORD written;
    WriteFile(handle, message, static_cast<DWORD>(lstrlenA(message)), &written, nullptr);
    CloseHandle(handle);
}

static void LogNumber(const char* label, DWORD number) {
    char line[128];
    DWORD p=0;
    while (label[p] && p<90) { line[p]=label[p]; ++p; }
    char digits[11]; DWORD n=0;
    do { digits[n++]=static_cast<char>('0'+number%10); number/=10; } while(number);
    while(n) line[p++]=digits[--n];
    line[p++]='\r';line[p++]='\n';line[p]=0;
    Log(line);
}

static BOOL CALLBACK Initialize(PINIT_ONCE, PVOID, PVOID*) {
    wchar_t systemPath[32768];
    UINT size=GetSystemDirectoryW(systemPath,32768);
    if (!size || size + 12 >= 32768) return FALSE;
    const wchar_t suffix[]=L"\\version.dll";
    for(UINT i=0;i<sizeof(suffix)/sizeof(wchar_t);++i) systemPath[size+i]=suffix[i];
    HMODULE real=LoadLibraryExW(systemPath,nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!real || real == selfModule) {
        LogNumber("ERROR loading original version DLL: ",GetLastError());
        return FALSE;
    }
    // Resolve every export before publishing the table to other threads.
    for(unsigned i=0;i<17;++i) {
        targets[i]=GetProcAddress(real,names[i]);
        if (!targets[i]) {
            Log("OPTIONAL_EXPORT_MISSING: ");Log(names[i]);Log("\r\n");
        }
    }
    Log("FORWARDING_READY: original System32 version.dll loaded; available version exports resolved.\r\n");
    wchar_t pluginPath[32768];
    DWORD len=GetModuleFileNameW(selfModule,pluginPath,32768);
    if(len && len<32768) {
        while(len && pluginPath[len-1]!=L'\\')--len;
        const wchar_t pluginName[]=L"StardiveMount.asi";
        if(len+sizeof(pluginName)/sizeof(wchar_t)<32768) {
            for(DWORD i=0;i<sizeof(pluginName)/sizeof(wchar_t);++i)pluginPath[len+i]=pluginName[i];
            if(GetFileAttributesW(pluginPath)!=INVALID_FILE_ATTRIBUTES) {
                HMODULE extension=LoadLibraryExW(pluginPath,nullptr,
                    LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
                if(extension) {
                    auto start=reinterpret_cast<BOOL(WINAPI*)()>(GetProcAddress(extension,"StartStardivePlugin"));
                    Log(start && start()?"PLUGIN_READY: StardiveMount.asi started.\r\n":
                        "PLUGIN_ERROR: plugin start failed.\r\n");
                } else LogNumber("PLUGIN_ERROR: LoadLibrary failed: ",GetLastError());
            } else Log("PROXY_ONLY: StardiveMount.asi not present.\r\n");
        }
    }
    // Keep camera activation tied to the launcher's existing session.
    wchar_t cameraPath[32768];
    DWORD cameraLen=GetModuleFileNameW(selfModule,cameraPath,32768);
    if(cameraLen && cameraLen<32768 && CameraLauncherSessionActive()) {
        while(cameraLen && cameraPath[cameraLen-1]!=L'\\') --cameraLen;
        const wchar_t cameraName[]=L"MSDCamera.asi";
        if(cameraLen+sizeof(cameraName)/sizeof(wchar_t)<32768) {
            for(DWORD i=0;i<sizeof(cameraName)/sizeof(wchar_t);++i)cameraPath[cameraLen+i]=cameraName[i];
            if(GetFileAttributesW(cameraPath)!=INVALID_FILE_ATTRIBUTES) {
                HMODULE camera=LoadLibraryExW(cameraPath,nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
                auto start=camera?reinterpret_cast<BOOL(WINAPI*)()>(GetProcAddress(camera,"StartCameraPlugin")):nullptr;
                Log(start && start()?"CAMERA_READY: initialization queued.\r\n":"CAMERA_ERROR: extension could not start.\r\n");
            }
        }
    }
    return TRUE;
}

extern "C" FARPROC Resolve(unsigned index) {
    if(index>=17 || !InitOnceExecuteOnce(&once,Initialize,nullptr,nullptr) || !targets[index]) {
        Log("ERROR: forwarding initialization failed; refusing an invalid call.\r\n");
        RaiseFailFastException(nullptr,nullptr,0);
        ExitProcess(ERROR_PROC_NOT_FOUND);
    }
    return targets[index];
}

BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID) {
    if(reason==DLL_PROCESS_ATTACH) {
        selfModule=module;
        // Only bounded Kernel32 file operations here. Dynamic loading happens
        // on the first forwarded call, outside the loader's attach callback.
        Log("\r\nPROCESS_ATTACH: Stardive launcher version proxy v1.0 x64\r\n");
        LogNumber("PID: ",GetCurrentProcessId());
        LogNumber("TickCount: ",GetTickCount());
    }
    return TRUE;
}


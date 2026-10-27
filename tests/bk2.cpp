#define WIN32_LEAN_AND_MEAN
#include "../src/launcher/Bk2Files.h"
#include <cassert>
#include <filesystem>
#include <iostream>
namespace fs=std::filesystem;
static void Put(const std::wstring& path,char value,bool valid=true){HANDLE h=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,0,nullptr);assert(h!=INVALID_HANDLE_VALUE);char bytes[64];memset(bytes,value,sizeof(bytes));if(valid)memcpy(bytes,"KB2n",4);DWORD n;assert(WriteFile(h,bytes,64,&n,nullptr));CloseHandle(h);}
static char Value(const std::wstring& path){HANDLE h=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);assert(h!=INVALID_HANDLE_VALUE);char bytes[64];DWORD n;assert(ReadFile(h,bytes,64,&n,nullptr)&&n==64);CloseHandle(h);return bytes[10];}
int wmain(int argc,wchar_t** argv){
    if(argc==3&&!wcscmp(argv[1],L"--apply")){Bk2::Apply(argv[2]);return 0;}
    assert(argc==2);std::wstring testRoot=argv[1];assert(!fs::exists(testRoot));
    for(auto edition:{L"STARDIVE",L"STARDIVEE"}){
        auto root=Bk2::Join(testRoot,std::wstring(edition)+L"\\BigCat\\Binaries\\Win64");
        auto expectedMovies=Bk2::Join(testRoot,std::wstring(edition)+L"\\BigCat\\Saved\\PersistentDownloadDir\\M2\\moviechunk800");
        assert(Bk2::Movies(root)==expectedMovies);
        assert(Bk2::Movies(root+L"\\")==expectedMovies);
        assert(Bk2::Movies(root+L"/")==expectedMovies);
        auto movies=expectedMovies,mods=Bk2::Join(root,L"Mods");
        fs::create_directories(root);fs::create_directories(movies);fs::create_directories(Bk2::Join(mods,L"Nested"));fs::create_directories(Bk2::Join(mods,L"Duplicate"));
        auto target=Bk2::Join(movies,L"Bink_Recruit_Pickup_CH_Nagi.bk2"),source=Bk2::Join(mods,L"Nested\\Bink_Recruit_Pickup_CH_Nagi.bk2");Put(target,'O');Put(source,'R');
        Put(Bk2::Join(movies,L"Duplicate.bk2"),'D');Put(Bk2::Join(mods,L"Duplicate.bk2"),'1');Put(Bk2::Join(mods,L"Duplicate\\Duplicate.bk2"),'2');
        Put(Bk2::Join(movies,L"Disabled.bk2"),'E');Put(Bk2::Join(mods,L"Disabled.bk2.disabled"),'N');Put(Bk2::Join(mods,L"Missing.bk2"),'M');
        Put(Bk2::Join(movies,L"Invalid.bk2"),'I');Put(Bk2::Join(mods,L"Invalid.bk2"),'!',false);
        assert(Value(target)=='O');
        // Run the same routine in an actual separate process, then verify it persists after exit.
        wchar_t exe[32768],cmd[33000];GetModuleFileNameW(nullptr,exe,32768);swprintf_s(cmd,L"\"%s\" --apply \"%s\"",exe,(root+L"\\/").c_str());STARTUPINFOW si={sizeof(si)};PROCESS_INFORMATION pi={};assert(CreateProcessW(exe,cmd,nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&si,&pi));assert(WaitForSingleObject(pi.hProcess,10000)==WAIT_OBJECT_0);DWORD code;GetExitCodeProcess(pi.hProcess,&code);assert(code==0);CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
        assert(Value(target)=='R'&&Value(source)=='R');assert(Value(Bk2::Join(movies,L"Duplicate.bk2"))=='D');assert(Value(Bk2::Join(movies,L"Disabled.bk2"))=='E');assert(Value(Bk2::Join(movies,L"Invalid.bk2"))=='I');assert(!Bk2::File(Bk2::Join(movies,L"Missing.bk2")));assert(!fs::exists(Bk2::Join(root,L"Bk2Originals")));
        Put(target,'O');Bk2::Apply(root);assert(Value(target)=='R'); // Simulate a game repair before next load.
        Put(source,'S');Bk2::Apply(root);assert(Value(target)=='S'); // Updated mod applied on the next load.
        Put(target,'L');HANDLE locked=CreateFileW(target.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);assert(locked!=INVALID_HANDLE_VALUE);Bk2::Apply(root);assert(Value(target)=='L');CloseHandle(locked);Bk2::Apply(root);assert(Value(target)=='S');
        for(auto& entry:fs::directory_iterator(movies))assert(entry.path().extension()==L".bk2");
        std::wcout<<edition<<L": direct replacement, persistence after process exit, repeated loads, skip rules, locked target preservation, temporary-file cleanup, no backups PASS\n";
    }
}

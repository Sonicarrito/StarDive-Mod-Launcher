#pragma once
#include <windows.h>
#include <cstdint>
#include <cstdio>

// The launcher serves one client at a time. Its camera and asset extensions
// can start together, so a busy pipe or the gap between instances is transient.
static bool ConnectLauncherSession() {
    wchar_t exe[32768];DWORD count=GetModuleFileNameW(nullptr,exe,32768);
    if(!count||count>=32768)return false;
    CharLowerBuffW(exe,count);uint64_t hash=14695981039346656037ull;
    for(DWORD i=0;i<count;++i){hash^=static_cast<uint16_t>(exe[i]);hash*=1099511628211ull;}
    wchar_t pipe[128];swprintf_s(pipe,L"\\\\.\\pipe\\Sonicarrito.StarDive.%016llx",static_cast<unsigned long long>(hash));
    const ULONGLONG deadline=GetTickCount64()+3000;
    do {
        HANDLE handle=CreateFileW(pipe,GENERIC_READ,0,nullptr,OPEN_EXISTING,0,nullptr);
        if(handle!=INVALID_HANDLE_VALUE) {
            DWORD response=0,read=0;
            bool received=ReadFile(handle,&response,sizeof(response),&read,nullptr)&&read==sizeof(response);
            DWORD error=received?ERROR_SUCCESS:GetLastError();
            CloseHandle(handle);
            if(received)return response==0x53444d31;
            if(error!=ERROR_BROKEN_PIPE&&error!=ERROR_PIPE_NOT_CONNECTED&&error!=ERROR_NO_DATA)return false;
        } else {
            DWORD error=GetLastError();
            if(error!=ERROR_PIPE_BUSY&&error!=ERROR_FILE_NOT_FOUND)return false;
        }
        if(GetTickCount64()>=deadline)break;
        WaitNamedPipeW(pipe,50);
        Sleep(5);
    } while(GetTickCount64()<deadline);
    return false;
}

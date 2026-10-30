#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include "MinHook.h"
#include "GameCompatibility.h"
static HMODULE module;
static volatile LONG started=0;
static wchar_t folder[32768];
static float angle=90.f, closeDistance=25.f;
static bool closeZoom=true, preventFade=true;
using InputFn=void(__fastcall*)(void*,unsigned char,bool,float);
static InputFn originalInput=nullptr;
static volatile LONG inputSeen=0;
static void Log(const char* text){
 wchar_t p[32768]; swprintf_s(p,L"%sMSDCamera.log",folder);
 HANDLE f=CreateFileW(p,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(f==INVALID_HANDLE_VALUE)return;DWORD n;WriteFile(f,text,(DWORD)strlen(text),&n,nullptr);CloseHandle(f);
}
static void __fastcall OnInput(void* self,unsigned char axis,bool valueType,float value){
 auto p=static_cast<unsigned char*>(self);
 // Offsets verified from AM2PlayerCamera's reflected property descriptors.
 if(InterlockedCompareExchange(&inputSeen,1,0)==0){
  char line[256];sprintf_s(line,"INPUT_HOOK_HIT: original pitch=[%.2f,%.2f], minDistance=%.2f; requested pitch=[%.2f,%.2f], minDistance=%.2f\r\n",*(float*)(p+0xb84),*(float*)(p+0xb80),*(float*)(p+0xc04),-angle,angle,closeDistance);Log(line);
 }
 *(float*)(p+0xb80)=angle;
 *(float*)(p+0xb84)=-angle;
 if(closeZoom)*(float*)(p+0xc04)=closeDistance;
 if(preventFade)*(bool*)(p+0xba4)=false;
 originalInput(self,axis,valueType,value);
}
static DWORD WINAPI Initialize(void*){
 Log("\r\nMSD camera 1.10.2 pattern compatibility: starting.\r\n");
 wchar_t ini[32768];swprintf_s(ini,L"%sMSDCamera.ini",folder);
 if(!GetPrivateProfileIntW(L"Camera",L"Enabled",1,ini)){Log("DISABLED: configuration.\r\n");return 0;}
 angle=(float)GetPrivateProfileIntW(L"Camera",L"TiltDegrees",90,ini);
 if(angle<50.f || angle>90.f)angle=90.f;
 closeDistance=(float)GetPrivateProfileIntW(L"Camera",L"MinimumDistance",25,ini);
 if(closeDistance<10.f || closeDistance>100.f)closeDistance=25.f;
 closeZoom=GetPrivateProfileIntW(L"Camera",L"CloseZoom",1,ini)!=0;
 preventFade=GetPrivateProfileIntW(L"Camera",L"PreventCameraFade",1,ini)!=0;
 Compat::Image image;
 if(!Compat::OpenImage(image,GetModuleHandleW(nullptr))){Log("INCOMPATIBLE: executable image could not be inspected. Camera disabled; mod loading is separate.\r\n");return 0;}
 auto target=Compat::Camera(image);
 if(!target){Log("INCOMPATIBLE: camera function or reflected field layout did not match uniquely. Camera disabled; mod loading is separate.\r\n");return 0;}
 MH_STATUS s=MH_Initialize();if(s!=MH_OK&&s!=MH_ERROR_ALREADY_INITIALIZED){Log("FAILED: hook library initialization.\r\n");return 0;}
 if(MH_CreateHook(target,(void*)&OnInput,(void**)&originalInput)!=MH_OK){Log("FAILED: creating camera-input hook.\r\n");return 0;}
 if(MH_EnableHook(target)!=MH_OK){MH_RemoveHook(target);Log("FAILED: enabling camera-input hook.\r\n");return 0;}
 Log("HOOK_INSTALLED: gameplay camera input; tilt +/-90 available. Waiting for camera input.\r\n");return 0;
}
extern "C" __declspec(dllexport) BOOL WINAPI StartCameraPlugin(){
 if(InterlockedCompareExchange(&started,1,0))return TRUE;
 DWORD n=GetModuleFileNameW(module,folder,32768);if(!n||n>=32768)return FALSE;
 while(n&&folder[n-1]!=L'\\')--n;folder[n]=0;
 HANDLE t=CreateThread(nullptr,0,Initialize,nullptr,0,nullptr);if(!t)return FALSE;CloseHandle(t);return TRUE;
}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH){module=h;DisableThreadLibraryCalls(h);}return TRUE;}

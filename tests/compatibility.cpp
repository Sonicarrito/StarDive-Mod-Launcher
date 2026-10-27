#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <vector>
#include "../src/launcher/GameCompatibility.h"
#include "MinHook.h"
static int failures=0;
static void Check(bool ok,const char* name){printf("%s: %s\n",ok?"PASS":"FAIL",name);if(!ok)++failures;}
struct Change {
 unsigned char* p;DWORD old=0;std::vector<unsigned char> bytes;
 Change(unsigned char* address,size_t n):p(address),bytes(address,address+n){VirtualProtect(p,n,PAGE_EXECUTE_READWRITE,&old);}
 ~Change(){memcpy(p,bytes.data(),bytes.size());DWORD ignored;VirtualProtect(p,bytes.size(),old,&ignored);FlushInstructionCache(GetCurrentProcess(),p,bytes.size());}
};
static void Test(const wchar_t* path) {
 HMODULE h=LoadLibraryExW(path,nullptr,DONT_RESOLVE_DLL_REFERENCES);Check(h!=nullptr,"Game image maps without running game code");if(!h)return;
 Compat::Image image;Check(Compat::OpenImage(image,h),"64-bit image and unwind data verified");
 Compat::Layout l;char reason[512]={};bool core=Compat::Core(image,l,reason,sizeof(reason));Check(core,reason);
 if(!core){FreeLibrary(h);return;}
 Check(l.Automatic(),"Both content-completion functions discovered");auto camera=Compat::Camera(image);Check(camera!=nullptr,"Camera function and reflected fields verified");
 printf("DISCOVERY: mount=%llx tick=%llx setter=%llx camera=%llx\n",(unsigned long long)(l.mount-image.base),(unsigned long long)(l.tick-image.base),(unsigned long long)(l.setter-image.base),camera?(unsigned long long)(camera-image.base):0);
 auto nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(image.base+reinterpret_cast<IMAGE_DOS_HEADER*>(image.base)->e_lfanew);
 {Change c(reinterpret_cast<unsigned char*>(&nt->FileHeader.TimeDateStamp),4);nt->FileHeader.TimeDateStamp^=0x12345678;Compat::Layout test;Check(Compat::Core(image,test,reason,sizeof(reason)),"Changed build timestamp does not reject matching game code");}
 {Change c(l.mount,1);*l.mount^=1;Compat::Layout test;Check(!Compat::Core(image,test,reason,sizeof(reason)),"Missing mount signature fails closed");}
 for(auto& span:image.sections)if((span.flags&IMAGE_SCN_MEM_EXECUTE)&&span.n>8192){
  auto target=span.p+span.n-4096;Change c(target,256);memcpy(target,l.mount,127);Compat::Layout test;
  Check(!Compat::Core(image,test,reason,sizeof(reason)),"Duplicate mount signatures fail closed");break;
 }
 {Change c(l.tick,1);*l.tick^=1;Compat::Layout test;bool ok=Compat::Core(image,test,reason,sizeof(reason));Check(ok&&!test.Automatic(),"Missing automatic-loading signature preserves manual mod loading");}
 if(camera){Change c(camera,1);*camera^=1;Compat::Layout test;Check(!Compat::Camera(image)&&Compat::Core(image,test,reason,sizeof(reason)),"Missing camera signature disables only camera");}
 bool propertyTested=false;
 for(auto& span:image.sections)if(image.ReadOnly(span.p,span.n))for(size_t i=0;i+64<=span.n&&!propertyTested;i+=8){
  auto d=span.p+i;uintptr_t name=0;uint32_t gen=0;uint16_t off=0;memcpy(&name,d,8);memcpy(&gen,d+24,4);memcpy(&off,d+50,2);
  if(gen==0xa&&off==0xc04&&Compat::Text(image,reinterpret_cast<void*>(name),"distanceMin")){
   Change c(d+50,2);uint16_t altered=0xc08;memcpy(d+50,&altered,2);Compat::Layout test;
   Check(!Compat::Camera(image)&&Compat::Core(image,test,reason,sizeof(reason)),"Changed camera property layout disables only camera");propertyTested=true;
  }
 }
 Check(propertyTested,"Camera-layout negative test exercised");
 // All native hooks must be relocatable before using them in a live game.
 for(auto target:{l.tick,l.setter,camera})if(target){void* original=nullptr;MH_STATUS st=MH_CreateHook(target,reinterpret_cast<void*>(&Test),&original);Check(st==MH_OK&&original,"Hook trampoline can be created for discovered function");if(st==MH_OK)MH_RemoveHook(target);}
 FreeLibrary(h);
}
int wmain(int argc,wchar_t** argv){
 if(argc<2){puts("Usage: compatibility.exe <game executable> [older executable]");return 2;}
 if(MH_Initialize()!=MH_OK)return 3;
 for(int i=1;i<argc;++i)Test(argv[i]);MH_Uninitialize();return failures?1:0;
}

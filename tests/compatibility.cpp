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
static void Test(const wchar_t* path,bool resource=false) {
 void* reservation=nullptr;
 if(resource){
  FILE* file=nullptr;_wfopen_s(&file,path,L"rb");unsigned char header[4096]={};size_t count=file?fread(header,1,sizeof(header),file):0;if(file)fclose(file);
  auto dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(header);
  if(count>=sizeof(IMAGE_DOS_HEADER)&&dos->e_magic==IMAGE_DOS_SIGNATURE&&dos->e_lfanew>=0&&size_t(dos->e_lfanew)+sizeof(IMAGE_NT_HEADERS64)<=count){
   auto nt=reinterpret_cast<const IMAGE_NT_HEADERS64*>(header+dos->e_lfanew);
   reservation=VirtualAlloc(reinterpret_cast<void*>(nt->OptionalHeader.ImageBase),nt->OptionalHeader.SizeOfImage,MEM_RESERVE,PAGE_NOACCESS);
  }
 }
 HMODULE h=LoadLibraryExW(path,nullptr,resource?(LOAD_LIBRARY_AS_IMAGE_RESOURCE|LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE):DONT_RESOLVE_DLL_REFERENCES);
 Check(h!=nullptr,"Game image maps without running game code");if(!h){if(reservation)VirtualFree(reservation,0,MEM_RELEASE);return;}
 auto base=reinterpret_cast<unsigned char*>(reinterpret_cast<uintptr_t>(h)&~uintptr_t(3));
 bool stored=(reinterpret_cast<uintptr_t>(h)&3)==2;
 Check(!resource||stored,"Resource-only inspector mapping uses on-disk pointers");
 Compat::Image image;Check(Compat::OpenImage(image,base,stored),"64-bit image and unwind data verified");
 if(resource){
  Check(reinterpret_cast<uintptr_t>(base)!=image.preferredBase,"Resource mapping is forced away from its preferred address");
  Compat::Image legacy=image;legacy.storedPointers=false;Compat::Layout legacyLayout;char why[512]={};
  Check(!Compat::Core(legacy,legacyLayout,why,sizeof(why))&&strstr(why,"PakFile virtual-method table"),"Reproduces the previous Wine PakFile rejection before pointer translation");
 }

 Compat::Layout l;char reason[512]={};bool core=Compat::Core(image,l,reason,sizeof(reason));Check(core,reason);
 if(!core){FreeLibrary(h);if(reservation)VirtualFree(reservation,0,MEM_RELEASE);return;}
 Check(l.Automatic(),"Both content-completion functions discovered");auto camera=Compat::Camera(image);Check(camera!=nullptr,"Camera function and reflected fields verified");
 printf("DISCOVERY: mount=%llx tick=%llx setter=%llx camera=%llx\n",(unsigned long long)(l.mount-image.base),(unsigned long long)(l.tick-image.base),(unsigned long long)(l.setter-image.base),camera?(unsigned long long)(camera-image.base):0);
 if(resource){FreeLibrary(h);if(reservation)VirtualFree(reservation,0,MEM_RELEASE);return;}
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
  if(gen==0xa&&off==0xc04&&Compat::Text(image,image.Pointer(name),"distanceMin")){
   Change c(d+50,2);uint16_t altered=0xc08;memcpy(d+50,&altered,2);Compat::Layout test;
   Check(!Compat::Camera(image)&&Compat::Core(image,test,reason,sizeof(reason)),"Changed camera property layout disables only camera");propertyTested=true;
  }
 }
 Check(propertyTested,"Camera-layout negative test exercised");
 {Change c(l.vtable+0x60,8);uintptr_t invalid=image.storedPointers?image.preferredBase+image.size+8:reinterpret_cast<uintptr_t>(image.base)+image.size+8;memcpy(l.vtable+0x60,&invalid,8);Compat::Layout test;
  Check(!Compat::Core(image,test,reason,sizeof(reason)),"Invalid PakFile method pointers are still rejected");}
 // All native hooks must be relocatable before using them in a live game.
 if(!resource)for(auto target:{l.tick,l.setter,camera})if(target){void* original=nullptr;MH_STATUS st=MH_CreateHook(target,reinterpret_cast<void*>(&Test),&original);Check(st==MH_OK&&original,"Hook trampoline can be created for discovered function");if(st==MH_OK)MH_RemoveHook(target);}
 FreeLibrary(h);if(reservation)VirtualFree(reservation,0,MEM_RELEASE);
}
int wmain(int argc,wchar_t** argv){
 SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);setvbuf(stdout,nullptr,_IONBF,0);
 if(argc<2){puts("Usage: compatibility.exe <game executable> [older executable]");return 2;}
 if(MH_Initialize()!=MH_OK)return 3;
 for(int i=1;i<argc;++i){Test(argv[i]);Test(argv[i],true);}MH_Uninitialize();return failures?1:0;
}

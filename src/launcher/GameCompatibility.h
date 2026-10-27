#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <vector>
#include "GamePatterns.h"

namespace Compat {
struct Span { unsigned char* p; size_t n; DWORD flags; };
struct Image {
    unsigned char* base=nullptr; size_t size=0;
    std::vector<Span> sections;
    const RUNTIME_FUNCTION* functions=nullptr; size_t functionCount=0;
    bool Range(const void* ptr,size_t n) const {
        auto a=reinterpret_cast<uintptr_t>(ptr),b=reinterpret_cast<uintptr_t>(base);
        return a>=b && n<=size && a-b<=size-n;
    }
    const Span* Section(const void* ptr,size_t n=1) const {
        for(auto& s:sections) {
            auto a=reinterpret_cast<uintptr_t>(ptr),b=reinterpret_cast<uintptr_t>(s.p);
            if(a>=b && n<=s.n && a-b<=s.n-n)return &s;
        }
        return nullptr;
    }
    bool Code(const void* p,size_t n=1) const {auto s=Section(p,n);return s&&(s->flags&IMAGE_SCN_MEM_EXECUTE);}
    bool Data(const void* p,size_t n=1) const {auto s=Section(p,n);return s&&(s->flags&IMAGE_SCN_MEM_READ)&&!(s->flags&IMAGE_SCN_MEM_EXECUTE);}
    bool ReadOnly(const void* p,size_t n=1) const {auto s=Section(p,n);return s&&(s->flags&IMAGE_SCN_MEM_READ)&&!(s->flags&(IMAGE_SCN_MEM_EXECUTE|IMAGE_SCN_MEM_WRITE));}
    bool Writable(const void* p,size_t n=1) const {auto s=Section(p,n);return s&&(s->flags&IMAGE_SCN_MEM_WRITE)&&!(s->flags&IMAGE_SCN_MEM_EXECUTE);}
    bool Init(void* module) {
        base=static_cast<unsigned char*>(module);sections.clear();
        auto dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
        if(!base || dos->e_magic!=IMAGE_DOS_SIGNATURE || dos->e_lfanew<64 || dos->e_lfanew>0x800)return false;
        auto nt=reinterpret_cast<const IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
        if(nt->Signature!=IMAGE_NT_SIGNATURE || nt->FileHeader.Machine!=IMAGE_FILE_MACHINE_AMD64 ||
           nt->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR64_MAGIC || nt->FileHeader.NumberOfSections>96 ||
           nt->FileHeader.SizeOfOptionalHeader<sizeof(IMAGE_OPTIONAL_HEADER64))return false;
        size=nt->OptionalHeader.SizeOfImage;
        if(size<0x1000 || size>0x40000000)return false;
        auto sh=IMAGE_FIRST_SECTION(nt);
        if(!Range(sh,size_t(nt->FileHeader.NumberOfSections)*sizeof(*sh)))return false;
        for(unsigned i=0;i<nt->FileHeader.NumberOfSections;++i) {
            size_t n=sh[i].Misc.VirtualSize;
            if(!n || (sh[i].Characteristics&IMAGE_SCN_MEM_DISCARDABLE))continue;
            if(sh[i].VirtualAddress>size || n>size-sh[i].VirtualAddress)return false;
            sections.push_back({base+sh[i].VirtualAddress,n,sh[i].Characteristics});
        }
        if(nt->OptionalHeader.NumberOfRvaAndSizes<=IMAGE_DIRECTORY_ENTRY_EXCEPTION)return false;
        auto e=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
        if(!e.VirtualAddress || !e.Size || e.Size%sizeof(RUNTIME_FUNCTION) || e.VirtualAddress>=size || e.Size>size-e.VirtualAddress)return false;
        functions=reinterpret_cast<const RUNTIME_FUNCTION*>(base+e.VirtualAddress);
        functionCount=e.Size/sizeof(RUNTIME_FUNCTION);
        return ReadOnly(functions,e.Size);
    }
    uintptr_t Function(const void* ptr) const {
        if(!Code(ptr))return 0;
        DWORD rva=DWORD(static_cast<const unsigned char*>(ptr)-base);
        size_t lo=0,hi=functionCount;
        while(lo<hi){size_t mid=lo+(hi-lo)/2;if(functions[mid].BeginAddress<=rva)lo=mid+1;else hi=mid;}
        if(!lo)return 0;auto& f=functions[lo-1];
        if(f.EndAddress<=f.BeginAddress || f.EndAddress>size || !Code(base+f.BeginAddress))return 0;
        return rva<f.EndAddress?reinterpret_cast<uintptr_t>(base+f.BeginAddress):0;
    }
    bool Start(const void* ptr) const {return Function(ptr)==reinterpret_cast<uintptr_t>(ptr);}
};
struct Pattern {
    std::vector<unsigned char> value,mask;
    explicit Pattern(const char* s) {
        while(*s){if(*s==' '){++s;continue;}if(s[0]=='?'&&s[1]=='?'){value.push_back(0);mask.push_back(0);s+=2;}
        else {unsigned x=0;if(sscanf_s(s,"%2x",&x)!=1){value.clear();return;}value.push_back((unsigned char)x);mask.push_back(255);s+=2;}}
    }
    bool Match(const unsigned char* p) const {for(size_t i=0;i<value.size();++i)if(mask[i]&&p[i]!=value[i])return false;return true;}
};
inline std::vector<unsigned char*> Scan(const Image& im,const char* pattern,bool code=true) {
    Pattern p(pattern);std::vector<unsigned char*> hits;
    if(p.value.empty())return hits;
    for(auto& s:im.sections) {
        bool executable=(s.flags&IMAGE_SCN_MEM_EXECUTE)!=0;
        if(!(s.flags&IMAGE_SCN_MEM_READ)||executable!=code||s.n<p.value.size())continue;
        auto end=s.p+s.n-p.value.size()+1;auto cur=s.p;
        while(cur<end) {
            if(p.mask[0]){cur=static_cast<unsigned char*>(memchr(cur,p.value[0],size_t(end-cur)));if(!cur)break;}
            if(p.Match(cur))hits.push_back(cur);
            ++cur;
        }
    }
    return hits;
}
inline unsigned char* UniqueCode(const Image& im,const char* pattern,bool start=true) {
    auto hits=Scan(im,pattern);
    return hits.size()==1 && (!start||im.Start(hits[0]))?hits[0]:nullptr;
}
inline unsigned char* Relative(const Image& im,const unsigned char* ins,size_t disp,size_t length) {
    if(!im.Code(ins,length)||disp+4>length)return nullptr;
    int32_t d=0;memcpy(&d,ins+disp,4);
    intptr_t target=reinterpret_cast<intptr_t>(ins+length)+d;
    auto p=reinterpret_cast<unsigned char*>(target);
    return im.Range(p,1)?p:nullptr;
}
inline bool Text(const Image& im,const void* p,const char* text) {
    size_t n=strlen(text)+1;return im.ReadOnly(p,n)&&!memcmp(p,text,n);
}
inline bool WideText(const Image& im,const void* p,const wchar_t* text) {
    size_t n=(wcslen(text)+1)*sizeof(wchar_t);return im.ReadOnly(p,n)&&!memcmp(p,text,n);
}
struct Layout {
    unsigned char* mount=nullptr;unsigned char* name=nullptr;
    unsigned char* vtable=nullptr;unsigned char* platform=nullptr;
    unsigned char* tick=nullptr;unsigned char* setter=nullptr;
    unsigned char* camera=nullptr;
    unsigned char mountState=255,successState=255;
    const char* stateNames[256]={};
    bool Core()const{return mount&&name&&vtable&&platform;}
    bool Automatic()const{return tick&&setter&&mountState!=255&&successState!=255;}
};
inline void ResolveStates(const Image& im,Layout& out) {
    unsigned valid=0;Layout candidate;
    const char* prefix="EM2PatchStatus::";size_t prefixLength=strlen(prefix);
    for(auto& span:im.sections) {
        if(!im.ReadOnly(span.p,span.n))continue;
        for(size_t i=0;i+16<=span.n;i+=8) {
            uintptr_t name=0;uint64_t value=0;memcpy(&name,span.p+i,8);memcpy(&value,span.p+i+8,8);
            if(value || !Text(im,reinterpret_cast<void*>(name),"EM2PatchStatus::NONE"))continue;
            Layout found;bool failed=false;unsigned count=0;
            for(size_t j=i;j+16<=span.n&&count<64;j+=16,++count) {
                memcpy(&name,span.p+j,8);memcpy(&value,span.p+j+8,8);auto text=reinterpret_cast<const char*>(name);
                if(!im.ReadOnly(text,prefixLength)||memcmp(text,prefix,prefixLength))break;
                if(value>=255 || found.stateNames[value]){failed=true;break;}
                unsigned len=0;for(;len<96;++len){if(!im.ReadOnly(text+len,1)){failed=true;break;}if(!text[len])break;}
                if(failed||len==96){failed=true;break;}
                found.stateNames[value]=text+prefixLength;
                if(Text(im,text,"EM2PatchStatus::CONTENT_MOUNT"))found.mountState=(unsigned char)value;
                if(Text(im,text,"EM2PatchStatus::SUCCESSED"))found.successState=(unsigned char)value;
            }
            if(!failed&&count>=8&&found.mountState!=255&&found.successState!=255&&found.mountState!=found.successState){candidate=found;++valid;}
        }
    }
    if(valid==1){out.mountState=candidate.mountState;out.successState=candidate.successState;memcpy(out.stateNames,candidate.stateNames,sizeof(out.stateNames));}
}
inline bool ResolveCore(const Image& im,Layout& out,char* reason,size_t n) {
    out.mount=UniqueCode(im,MountPattern);
    if(!out.mount){strcpy_s(reason,n,"Mod mount function is missing, ambiguous, or not a verified function entry.");return false;}
    unsigned char* pakString=nullptr;unsigned matches=0;
    for(auto p:Scan(im,"48 8D 05 ?? ?? ?? ?? C3")) {
        auto text=Relative(im,p,3,7);
        if(text&&WideText(im,text,L"PakFile")){out.name=p;pakString=text;++matches;}
    }
    if(matches!=1){strcpy_s(reason,n,"PakFile name accessor could not be identified uniquely.");return false;}
    auto globals=Scan(im,PlatformAnchorPattern);unsigned globalCount=0;
    for(auto p:globals) {
        if(Relative(im,p+31,3,7)!=pakString)continue;
        auto global=Relative(im,p,3,7);
        if(!global||!im.Writable(global,sizeof(void*))||reinterpret_cast<uintptr_t>(global)%sizeof(void*))continue;
        out.platform=global;++globalCount;
    }
    if(globalCount!=1){strcpy_s(reason,n,"Platform-file pointer could not be verified uniquely.");return false;}
    auto caller=UniqueCode(im,MountCallerPattern,false);
    if(!caller || Relative(im,caller+26,1,5)!=out.mount || !im.Function(caller)) {
        strcpy_s(reason,n,"The game's own mod-mount caller does not match the required arguments.");return false;
    }
    bool sameFunction=false;
    for(auto p:globals)if(Relative(im,p,3,7)==out.platform && im.Function(p)==im.Function(caller))sameFunction=true;
    if(!sameFunction){strcpy_s(reason,n,"Mount caller and platform-file reference do not belong to the same verified function.");return false;}
    unsigned vtCount=0;
    for(auto& s:im.sections) {
        if(!im.ReadOnly(s.p,s.n)||s.n<0x90)continue;
        size_t first=(sizeof(void*)-reinterpret_cast<uintptr_t>(s.p)%sizeof(void*))%sizeof(void*);
        for(size_t i=first+0x70;i+sizeof(void*)<=s.n;i+=sizeof(void*)) {
            uintptr_t ptr=0;memcpy(&ptr,s.p+i,sizeof(ptr));
            if(ptr!=reinterpret_cast<uintptr_t>(out.name))continue;
            auto vt=s.p+i-0x70;uintptr_t lower=0;memcpy(&lower,vt+0x60,sizeof(lower));
            const unsigned char getter[]={0x48,0x8b,0x41,0x08,0xc3};
            if(im.ReadOnly(vt,0x90)&&im.Code(reinterpret_cast<void*>(lower),5)&&!memcmp(reinterpret_cast<void*>(lower),getter,5)){
                bool codePointers=true;
                for(unsigned j=0;j<0x90;j+=8){uintptr_t fn=0;memcpy(&fn,vt+j,8);if(!im.Code(reinterpret_cast<void*>(fn)))codePointers=false;}
                if(codePointers){out.vtable=vt;++vtCount;}
            }
        }
    }
    if(vtCount!=1){strcpy_s(reason,n,"PakFile virtual-method table could not be verified uniquely.");return false;}
    out.tick=UniqueCode(im,PatchTickPattern);out.setter=UniqueCode(im,StateSetterPattern);ResolveStates(im,out);
    strcpy_s(reason,n,out.Automatic()?"Mod loading and automatic content-completion detection verified.":"Mod loading verified; automatic detection unavailable. Use F8 after the content check.");
    return true;
}
inline const unsigned char* Descriptor(const Image& im,uintptr_t ptr) {
    auto p=reinterpret_cast<const unsigned char*>(ptr);
    if(!im.ReadOnly(p,64))return nullptr;
    uintptr_t name=0;memcpy(&name,p,8);auto text=reinterpret_cast<const char*>(name);
    if(!im.ReadOnly(text,1))return nullptr;
    unsigned len=0;for(;len<80;++len){if(!im.ReadOnly(text+len,1))return nullptr;if(!text[len])break;if((unsigned char)text[len]<32||(unsigned char)text[len]>126)return nullptr;}
    if(!len||len==80)return nullptr;
    uint32_t gen=0;uint16_t dim=0;memcpy(&gen,p+24,4);memcpy(&dim,p+48,2);
    return gen>0&&gen<256&&dim<100?p:nullptr;
}
inline bool CameraFields(const Image& im) {
    // Validate all fields in the same generated property-pointer array.
    std::vector<uintptr_t> candidates;
    for(auto& s:im.sections) {
        if(!im.ReadOnly(s.p,s.n))continue;
        for(size_t i=0;i+64<=s.n;i+=8) {
            auto d=s.p+i;uintptr_t name=0;memcpy(&name,d,8);
            uint32_t gen=0;uint16_t off=0;memcpy(&gen,d+24,4);memcpy(&off,d+50,2);
            if(gen==0xa&&off==0xb80&&Text(im,reinterpret_cast<void*>(name),"pitchMaxLimit"))candidates.push_back(reinterpret_cast<uintptr_t>(d));
        }
    }
    unsigned valid=0;
    for(auto desc:candidates)for(auto& s:im.sections) {
        if(!im.ReadOnly(s.p,s.n))continue;
        for(size_t i=0;i+8<=s.n;i+=8) {
            uintptr_t ptr=0;memcpy(&ptr,s.p+i,8);if(ptr!=desc)continue;
            size_t begin=i,back=0;
            while(begin>=8&&back<128){memcpy(&ptr,s.p+begin-8,8);if(!Descriptor(im,ptr))break;begin-=8;++back;}
            if(back==128)continue;
            bool pitchMax=false,pitchMin=false,distance=false,fade=false;
            for(size_t j=begin,count=0;j+8<=s.n&&count<128;j+=8,++count) {
                memcpy(&ptr,s.p+j,8);auto d=Descriptor(im,ptr);if(!d)break;
                uintptr_t np=0;memcpy(&np,d,8);auto name=reinterpret_cast<const char*>(np);
                uint32_t gen=0;uint16_t off=0;memcpy(&gen,d+24,4);memcpy(&off,d+50,2);
                if(gen==0xa&&off==0xb80&&Text(im,name,"pitchMaxLimit"))pitchMax=true;
                if(gen==0xa&&off==0xb84&&Text(im,name,"pitchMinLimit"))pitchMin=true;
                if(gen==0xa&&off==0xc04&&Text(im,name,"distanceMin"))distance=true;
                if(gen==0x4c&&Text(im,name,"useDistanceFade")) {
                    uintptr_t set=0;memcpy(&set,d+56,8);
                    const unsigned char expected[]={0xc6,0x81,0xa4,0x0b,0,0,1,0xc3};
                    fade=im.Code(reinterpret_cast<void*>(set),8)&&!memcmp(reinterpret_cast<void*>(set),expected,8);
                }
            }
            if(pitchMax&&pitchMin&&distance&&fade)++valid;
        }
    }
    return valid==1;
}
inline unsigned char* ResolveCamera(const Image& im) {
    auto p=UniqueCode(im,CameraInputPattern);
    return p&&CameraFields(im)?p:nullptr;
}
inline bool OpenImage(Image& im,void* module) {
    // Keep SEH outside functions with C++ objects that require unwinding.
    __try {return im.Init(module);}__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
inline bool Core(Image& im,Layout& out,char* reason,size_t n) {
    __try{return ResolveCore(im,out,reason,n);}__except(EXCEPTION_EXECUTE_HANDLER){strcpy_s(reason,n,"Executable image could not be read safely.");return false;}
}
inline unsigned char* Camera(Image& im) {
    __try{return ResolveCamera(im);}__except(EXCEPTION_EXECUTE_HANDLER){return nullptr;}
}
}

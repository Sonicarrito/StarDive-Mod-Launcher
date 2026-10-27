#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <cwctype>
#include <cstdio>

namespace Bk2 {
inline std::wstring Join(const std::wstring& a,const std::wstring& b){return a+L"\\"+b;}
inline std::wstring Lower(std::wstring s){for(auto& c:s)c=static_cast<wchar_t>(towlower(c));return s;}
inline bool Ends(const std::wstring& s,const wchar_t* ext){std::wstring e=ext;return s.size()>=e.size()&&Lower(s.substr(s.size()-e.size()))==e;}
inline bool Directory(const std::wstring& p){DWORD a=GetFileAttributesW(p.c_str());return a!=INVALID_FILE_ATTRIBUTES&&(a&FILE_ATTRIBUTE_DIRECTORY)&&!(a&FILE_ATTRIBUTE_REPARSE_POINT);}
inline bool File(const std::wstring& p){DWORD a=GetFileAttributesW(p.c_str());return a!=INVALID_FILE_ATTRIBUTES&&!(a&(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT));}
inline std::wstring Parent(std::wstring p){auto i=p.find_last_of(L"\\/");return i==p.npos?L"":p.substr(0,i);}
inline std::wstring Movies(std::wstring win64){while(!win64.empty()&&(win64.back()==L'\\'||win64.back()==L'/'))win64.pop_back();return Join(Parent(Parent(win64)),L"Saved\\PersistentDownloadDir\\M2\\moviechunk800");}
inline void Log(const std::wstring& root,const wchar_t* action,const std::wstring& name=L""){
    HANDLE h=CreateFileW(Join(root,L"stardive-bk2.log").c_str(),FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(h==INVALID_HANDLE_VALUE)return;
    std::wstring line=std::wstring(action)+L": "+name+L"\r\n";
    int n=WideCharToMultiByte(CP_UTF8,0,line.c_str(),static_cast<int>(line.size()),nullptr,0,nullptr,nullptr);
    std::string bytes(n,' ');WideCharToMultiByte(CP_UTF8,0,line.c_str(),static_cast<int>(line.size()),&bytes[0],n,nullptr,nullptr);
    DWORD written;WriteFile(h,bytes.data(),static_cast<DWORD>(bytes.size()),&written,nullptr);CloseHandle(h);
}
inline std::vector<std::wstring> Files(const std::wstring& dir,bool recurse){
    std::vector<std::wstring> result;if(!Directory(dir))return result;
    WIN32_FIND_DATAW d;HANDLE h=FindFirstFileW(Join(dir,L"*").c_str(),&d);if(h==INVALID_HANDLE_VALUE)return result;
    do{if(!wcscmp(d.cFileName,L".")||!wcscmp(d.cFileName,L"..")||(d.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT))continue;
        auto p=Join(dir,d.cFileName);
        if(d.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){if(recurse){auto children=Files(p,true);result.insert(result.end(),children.begin(),children.end());}}
        else if(Ends(d.cFileName,L".bk2"))result.push_back(p);
    }while(FindNextFileW(h,&d));FindClose(h);return result;
}
inline bool IsBink(const std::wstring& p){
    if(!File(p))return false;HANDLE h=CreateFileW(p.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);if(h==INVALID_HANDLE_VALUE)return false;
    char b[4]={};DWORD n=0;ReadFile(h,b,4,&n,nullptr);LARGE_INTEGER size={};GetFileSizeEx(h,&size);CloseHandle(h);
    return n==4&&size.QuadPart>=44&&b[0]=='K'&&b[1]=='B'&&b[2]=='2';
}
inline bool Flush(const std::wstring& p){HANDLE h=CreateFileW(p.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);if(h==INVALID_HANDLE_VALUE)return false;bool ok=FlushFileBuffers(h)!=FALSE;CloseHandle(h);return ok;}

inline void Apply(const std::wstring& root){
    auto movies=Movies(root);
    if(!Directory(movies)){Log(root,L"MOVIE_FOLDER_MISSING",movies);return;}
    std::map<std::wstring,std::vector<std::wstring>> byName;
    for(auto& p:Files(Join(root,L"Mods"),true))byName[Lower(p.substr(p.find_last_of(L'\\')+1))].push_back(p);
    for(auto& pair:byName){
        auto source=pair.second[0];auto name=source.substr(source.find_last_of(L'\\')+1);
        if(pair.second.size()!=1){Log(root,L"DUPLICATE_NAME_SKIPPED",name);continue;}
        auto target=Join(movies,name);
        if(!File(target)){Log(root,L"NO_EXACT_TARGET_SKIPPED",name);continue;}
        if(!IsBink(source)){Log(root,L"INVALID_BINK2_SKIPPED",name);continue;}
        wchar_t stage[MAX_PATH];
        if(!GetTempFileNameW(movies.c_str(),L"BK2",0,stage)){Log(root,L"TEMP_FILE_FAILED",name);continue;}
        // Stage the replacement on the same volume; the original is not copied or backed up.
        bool ok=CopyFileW(source.c_str(),stage,FALSE)&&Flush(stage)&&MoveFileExW(stage,target.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
        if(!ok)DeleteFileW(stage);
        Log(root,ok?L"REPLACED_PERSISTENT":L"REPLACEMENT_FAILED",name);
    }
}
}

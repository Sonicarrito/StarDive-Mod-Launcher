using System;
using System.IO;
using System.IO.Pipes;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Win32.SafeHandles;

sealed class LauncherSession : IDisposable {
    [DllImport("kernel32.dll",SetLastError=true)] static extern bool GetNamedPipeClientProcessId(SafePipeHandle pipe,out uint id);
    [DllImport("kernel32.dll",SetLastError=true)] static extern IntPtr OpenProcess(uint access,bool inherit,uint id);
    [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern bool QueryFullProcessImageName(IntPtr process,uint flags,StringBuilder path,ref uint size);
    [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr handle);
    readonly string expected, name;
    readonly object sync=new object();
    readonly ManualResetEvent ready=new ManualResetEvent(false);
    NamedPipeServerStream pipe;
    volatile bool active=true;
    Exception failure;
    public static string PipeName(string exe) {
        ulong hash=14695981039346656037UL;unchecked{foreach(char c in Path.GetFullPath(exe).ToLowerInvariant()){hash^=c;hash*=1099511628211UL;}}
        return "Sonicarrito.StarDive."+hash.ToString("x16");
    }
    public LauncherSession(string exe) {
        expected=Path.GetFullPath(exe);name=PipeName(expected);Task.Run((Action)Serve);
        if(!ready.WaitOne(3000)){Dispose();throw new IOException("Launcher session could not start.");}
        if(failure!=null){Dispose();throw new IOException("Launcher session failed: "+failure.Message);}
    }
    bool ValidClient(NamedPipeServerStream server) {
        uint id;if(!GetNamedPipeClientProcessId(server.SafePipeHandle,out id))return false;
        IntPtr process=OpenProcess(0x1000,false,id);if(process==IntPtr.Zero)return false;
        try{var path=new StringBuilder(32768);uint size=32768;return QueryFullProcessImageName(process,0,path,ref size)&&string.Equals(Path.GetFullPath(path.ToString()),expected,StringComparison.OrdinalIgnoreCase);}finally{CloseHandle(process);}
    }
    void Serve() {
        try {
            while(active) {
                using(var server=new NamedPipeServerStream(name,PipeDirection.Out,1,PipeTransmissionMode.Byte,PipeOptions.Asynchronous)) {
                    lock(sync){if(!active)return;pipe=server;}
                    ready.Set();server.WaitForConnection();
                    uint response=active&&ValidClient(server)?0x53444d31U:0U;
                    byte[] bytes=BitConverter.GetBytes(response);server.Write(bytes,0,bytes.Length);server.Flush();
                    lock(sync){if(pipe==server)pipe=null;}
                }
            }
        }catch(Exception e){if(active)failure=e;ready.Set();}
    }
    public void Dispose(){active=false;lock(sync){if(pipe!=null){pipe.Dispose();pipe=null;}}}
}

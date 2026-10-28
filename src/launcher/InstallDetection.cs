using System;
using System.IO;
using System.Collections.Generic;
using System.Web.Script.Serialization;
using System.Reflection;
using System.Runtime.InteropServices;
using Microsoft.Win32;

class DetectedInstallation {public string gameRoot="";public string officialLauncher="";}
static class InstallDetection {
    public static bool GameValid(string root){return !string.IsNullOrWhiteSpace(root)&&File.Exists(Path.Combine(root,"BigCat","Binaries","Win64","BigCat-Win64-Shipping.exe"));}
    public static bool IsEpic(string path){return string.Equals(Path.GetFileName(path),"Netmarble Launcher_EPIC.exe",StringComparison.OrdinalIgnoreCase);}
    public static bool LauncherValid(string path){return !string.IsNullOrWhiteSpace(path)&&File.Exists(path)&&(IsEpic(path)||string.Equals(Path.GetFileName(path),"Netmarble Launcher.exe",StringComparison.OrdinalIgnoreCase));}
    static void Candidate(DetectedInstallation result,string path){if(string.IsNullOrWhiteSpace(path))return;if(result.gameRoot==""&&GameValid(path))result.gameRoot=path;if(result.officialLauncher==""&&LauncherValid(path))result.officialLauncher=path;}
    public static DetectedInstallation Find(){var native=Find(false);if(GameValid(native.gameRoot)&&LauncherValid(native.officialLauncher))return native;var epic=Find(true);if(GameValid(epic.gameRoot)&&LauncherValid(epic.officialLauncher))return epic;return GameValid(native.gameRoot)||LauncherValid(native.officialLauncher)?native:epic;}
    public static DetectedInstallation FindFor(string gameRoot,string launcher){
        var native=Find(false);var epic=Find(true);
        // A selected game takes priority; never pair it with the other distribution's launcher.
        if(GameValid(gameRoot)){
            string root=Path.GetFullPath(gameRoot).TrimEnd('\\','/');
            bool isEpic=string.Equals(root,epic.gameRoot.TrimEnd('\\','/'),StringComparison.OrdinalIgnoreCase)||root.IndexOf("Netmarble Game_EPIC",StringComparison.OrdinalIgnoreCase)>=0||string.Equals(Path.GetFileName(root),"STARDIVEE",StringComparison.OrdinalIgnoreCase);
            var found=isEpic?epic:native;found.gameRoot=gameRoot;
            if(LauncherValid(launcher)&&IsEpic(launcher)==isEpic)found.officialLauncher=launcher;return found;
        }
        if(LauncherValid(launcher)){var found=IsEpic(launcher)?epic:native;found.officialLauncher=launcher;return found;}
        return Find();
    }
    public static DetectedInstallation Find(bool epic) {
        var result=new DetectedInstallation();var roots=new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        string launcherName=epic?"Netmarble Launcher_EPIC":"Netmarble Launcher";
        try {
            string config=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),launcherName,"config.json");
            if(File.Exists(config)){
                var json=new JavaScriptSerializer {MaxJsonLength=16*1024*1024};var data=json.Deserialize<Dictionary<string,object>>(File.ReadAllText(config));object games;
                if(data.TryGetValue("game",out games))foreach(var item in (Dictionary<string,object>)games){if(!item.Key.StartsWith("monster2",StringComparison.OrdinalIgnoreCase))continue;var game=item.Value as Dictionary<string,object>;object path;if(game!=null&&game.TryGetValue("installPath",out path)&&path is string)Candidate(result,(string)path);}
            }
        }catch{}
        try {
            foreach(string shortcut in new[]{Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.CommonPrograms),launcherName+".lnk"),Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Programs),launcherName+".lnk")}){
                if(!File.Exists(shortcut))continue;object shell=null,link=null;
                try{Type type=Type.GetTypeFromProgID("WScript.Shell");shell=Activator.CreateInstance(type);link=type.InvokeMember("CreateShortcut",BindingFlags.InvokeMethod,null,shell,new object[]{shortcut});string target=(string)link.GetType().InvokeMember("TargetPath",BindingFlags.GetProperty,null,link,null);Candidate(result,target);}
                finally{if(link!=null)Marshal.ReleaseComObject(link);if(shell!=null)Marshal.ReleaseComObject(shell);}
            }
        }catch{}
        foreach(var hive in new[]{Registry.CurrentUser,Registry.LocalMachine}){
            try{using(var key=hive.OpenSubKey(@"Software\Netmarble Corp"))if(key!=null)foreach(string name in new[]{"GameDrive","GamePath","InstallPath"}){string value=key.GetValue(name) as string;if(!string.IsNullOrWhiteSpace(value))roots.Add(value);}}catch{}
        }
        roots.Add(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles));roots.Add(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86));
        foreach(var drive in DriveInfo.GetDrives()){try{if(drive.DriveType==DriveType.Fixed&&drive.IsReady){roots.Add(drive.RootDirectory.FullName);roots.Add(Path.Combine(drive.RootDirectory.FullName,"Games"));}}catch{}}
        foreach(string root in roots){
            string gameName=epic?"STARDIVEE":"STARDIVE",gameFolder=epic?"Netmarble Game_EPIC":"Netmarble Game";
            try{foreach(string relative in new[]{gameName,Path.Combine(gameFolder,gameName),Path.Combine("Netmarble",gameFolder,gameName)})Candidate(result,Path.Combine(root,relative));
                foreach(string relative in new[]{Path.Combine(launcherName,launcherName+".exe"),Path.Combine("Netmarble",launcherName,launcherName+".exe")})Candidate(result,Path.Combine(root,relative));
            }catch{}
        }
        return result;
    }
    public static string EpicLaunchUri(string launcher){
        using(var key=Registry.ClassesRoot.OpenSubKey(@"com.epicgames.launcher\shell\open\command")){
            if(key==null||string.IsNullOrWhiteSpace(key.GetValue(null) as string))
                throw new IOException("Epic Games Launcher is not registered for game launch requests. Open or repair the Epic Games Launcher, then try again.");
        }
        string folder=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData),"Epic","EpicGamesLauncher","Data","Manifests");
        return EpicLaunchUri(launcher,folder);
    }
    internal static string EpicLaunchUri(string launcher,string folder){
        if(!LauncherValid(launcher)||!IsEpic(launcher))
            throw new IOException("Select Netmarble Launcher_EPIC.exe for this Epic installation.");
        string selected=Path.GetFullPath(launcher);
        // Use a record only when its actual executable matches the selected launcher.
        // Titles can be translated, and a different installation may have the same title.
        try {
            if(Directory.Exists(folder))foreach(string file in Directory.GetFiles(folder,"*.item"))try{
                var data=new JavaScriptSerializer().Deserialize<Dictionary<string,object>>(File.ReadAllText(file));
                string install=Field(data,"InstallLocation"),executable=Field(data,"LaunchExecutable");
                if(install==""||executable==""||!string.Equals(Path.GetFullPath(Path.Combine(install,executable)),selected,StringComparison.OrdinalIgnoreCase))continue;
                string app=Field(data,"AppName"),ns=Field(data,"CatalogNamespace"),item=Field(data,"CatalogItemId");
                if(app!=""&&ns!=""&&item!="")return "com.epicgames.launcher://apps/"+Uri.EscapeDataString(ns+":"+item+":"+app)+"?action=launch&silent=true";
            }catch{}
        }catch(IOException){}catch(UnauthorizedAccessException){}
        // Epic also accepts any path inside its installed app, including its launcher EXE.
        // Let the registered Epic client locate the app using its own installation database.
        return "com.epicgames.launcher://apps/"+Uri.EscapeDataString(selected.Replace('\\','/'))+"?action=launch&silent=true";
    }
    static string Field(Dictionary<string,object> data,string name){
        object value;return data!=null&&data.TryGetValue(name,out value)&&value is string?((string)value).Trim():"";
    }
}

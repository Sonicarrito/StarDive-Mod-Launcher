using System;
using System.IO;
using System.Collections.Generic;
using System.Web.Script.Serialization;
class EpicLaunchTests {
 static void Check(bool ok,string name){if(!ok)throw new Exception(name);Console.WriteLine("PASS: "+name);}
 static string App(string uri){const string prefix="com.epicgames.launcher://apps/";Check(uri.StartsWith(prefix)&&uri.EndsWith("?action=launch&silent=true"),"Epic launch scheme and action");return Uri.UnescapeDataString(uri.Substring(prefix.Length,uri.IndexOf('?')-prefix.Length));}
 static void Record(string file,string launcher,string title){File.WriteAllText(file,new JavaScriptSerializer().Serialize(new Dictionary<string,object>{{"DisplayName",title},{"InstallLocation",Path.GetDirectoryName(launcher)},{"LaunchExecutable",Path.GetFileName(launcher)},{"AppName","artifact"},{"CatalogNamespace","sandbox"},{"CatalogItemId","catalog"}}));}
 static int Main(string[] args){try{
  string root=Path.GetFullPath(args[0]);Check(!Directory.Exists(root),"Fresh disposable Epic fixture");Directory.CreateDirectory(root);
  string launcher=Path.Combine(root,"AppData","Local","Custom Games \u00e7\u00e3 # &","Netmarble Launcher_EPIC.exe");Directory.CreateDirectory(Path.GetDirectoryName(launcher));File.WriteAllBytes(launcher,new byte[]{0});
  string folder=Path.Combine(root,"missing-records"),expected=launcher.Replace('\\','/');
  Check(App(InstallDetection.EpicLaunchUri(launcher,folder))==expected,"Missing ProgramData records use selected app path; Unicode and special characters round-trip");
  Directory.CreateDirectory(folder);Check(App(InstallDetection.EpicLaunchUri(launcher,folder))==expected,"Empty records use selected app path");
  File.WriteAllText(Path.Combine(folder,"corrupt.item"),"not JSON");Check(App(InstallDetection.EpicLaunchUri(launcher,folder))==expected,"Malformed record does not block startup");
  Record(Path.Combine(folder,"other.item"),Path.Combine(root,"Other App","Netmarble Launcher_EPIC.exe"),"MONGIL STAR DIVE");Check(App(InstallDetection.EpicLaunchUri(launcher,folder))==expected,"Same title in another installation cannot launch the wrong app");
  Record(Path.Combine(folder,"selected.item"),launcher,"Localized title without English game name");Check(App(InstallDetection.EpicLaunchUri(launcher,folder))=="sandbox:catalog:artifact","Exact launcher match works regardless of display name");
  File.Delete(Path.Combine(folder,"selected.item"));string notDirectory=Path.Combine(root,"file-not-directory");File.WriteAllText(notDirectory,"");Check(App(InstallDetection.EpicLaunchUri(launcher,notDirectory))==expected,"Unavailable record directory uses selected app path");
  string native=Path.Combine(root,"Netmarble Launcher.exe");File.WriteAllBytes(native,new byte[]{0});bool rejected=false;try{InstallDetection.EpicLaunchUri(native,folder);}catch(IOException){rejected=true;}Check(rejected,"Native distribution launcher cannot be used for Epic startup");
  rejected=false;try{InstallDetection.EpicLaunchUri(Path.Combine(root,"missing","Netmarble Launcher_EPIC.exe"),folder);}catch(IOException){rejected=true;}Check(rejected,"Missing selected executable is rejected clearly");
  return 0;
 }catch(Exception e){Console.Error.WriteLine(e);return 1;}}
}

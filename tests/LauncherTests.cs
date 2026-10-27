using System;using System.IO;using System.Reflection;using System.Windows.Forms;
class LauncherTests {
 const BindingFlags S=BindingFlags.Static|BindingFlags.NonPublic|BindingFlags.Public;
 const BindingFlags I=BindingFlags.Instance|BindingFlags.NonPublic|BindingFlags.Public;
 static void Check(bool ok,string name){if(!ok)throw new Exception(name);Console.WriteLine("PASS: "+name);}
 [STAThread] static int Main(string[] args){try{
  if(args.Length<3)return 2;
  var a=Assembly.LoadFile(Path.GetFullPath(args[0]));var launcher=a.GetType("Launcher");
  string root=Path.GetFullPath(args[1]);Check(!Directory.Exists(root),"Fresh disposable fixture");Directory.CreateDirectory(root);
  launcher.GetField("SettingsDir",S).SetValue(null,Path.Combine(root,"settings"));
  var check=launcher.GetMethod("CheckGameCompatibility",S);
  for(int i=2;i<args.Length;++i){uint flags=(uint)check.Invoke(null,new object[]{Path.GetFullPath(args[i])});Check(flags==7,"Managed/native probe verifies mods, automatic loading, and camera");}
  string log=File.ReadAllText(Path.Combine(root,"settings","compatibility.log"));Check(log.Contains("verified"),"Native probe diagnostics marshal correctly");
  string game=Path.Combine(root,"game"),target=Path.Combine(game,"BigCat","Binaries","Win64");Directory.CreateDirectory(target);
  File.Copy(args[2],Path.Combine(target,"BigCat-Win64-Shipping.exe"));
  using(var form=(Form)Activator.CreateInstance(launcher)){
   ((TextBox)launcher.GetField("game",I).GetValue(form)).Text=game;
   object settings=launcher.GetField("settings",I).GetValue(form);var camera=settings.GetType().GetField("enableCameraUnlock");camera.SetValue(settings,false);
   launcher.GetMethod("Install",I).Invoke(form,null);
   var payload=launcher.GetMethod("Payload",S);
   foreach(string n in new[]{"version.dll","StardiveMount.asi","MSDCamera.asi"})Check(Convert.ToBase64String(File.ReadAllBytes(Path.Combine(target,n)))==Convert.ToBase64String((byte[])payload.Invoke(null,new object[]{n})),"Installer deploys verified bundled "+n);
   Check(File.ReadAllText(Path.Combine(target,"MSDCamera.ini")).Contains("Enabled=0"),"Camera disabled setting preserved");
   camera.SetValue(settings,true);launcher.GetMethod("Install",I).Invoke(form,null);
   Check(File.ReadAllText(Path.Combine(target,"MSDCamera.ini")).Contains("Enabled=1"),"Camera enabled setting preserved");
   launcher.GetMethod("StopSession",I).Invoke(form,null);
  }
  // Reject unsupported images in preflight without touching the game fixture.
  bool rejected=false;try{check.Invoke(null,new object[]{Path.GetFullPath(args[0])});}catch(TargetInvocationException){rejected=true;}
  Check(rejected,"Unsupported executable fails compatibility preflight");
  return 0;
 }catch(Exception e){Console.Error.WriteLine(e);return 1;}}
}

using System;
using System.IO;
using System.Linq;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Windows.Forms;

using System.Security.Cryptography;
using System.Web.Script.Serialization;

using System.Threading.Tasks;
using System.Reflection;
using System.Threading;

class Branding {
    public string creator = "Sonicarrito";
    public string discordUrl = "https://discord.gg/8FNdxa9Zb";
    public string patreonUrl = "https://www.patreon.com/c/Sonicarrito";

}
class Settings {
    public string gameRoot = "";
    public string officialLauncher = "";
    public bool enableCameraUnlock = true;

}
class LauncherButton : Control, IButtonControl {
    bool hovered,pressed;
    public Image Image {get;set;}
    public DialogResult DialogResult {get;set;}
    public LauncherButton(){SetStyle(ControlStyles.UserPaint|ControlStyles.AllPaintingInWmPaint|ControlStyles.OptimizedDoubleBuffer|ControlStyles.SupportsTransparentBackColor|ControlStyles.Selectable|ControlStyles.StandardClick,true);BackColor=Color.Transparent;ForeColor=Color.White;TabStop=true;AccessibleRole=AccessibleRole.PushButton;}
    public void NotifyDefault(bool value){Invalidate();}
    public void PerformClick(){if(Enabled&&CanSelect){OnClick(EventArgs.Empty);if(DialogResult!=DialogResult.None){var form=FindForm();if(form!=null)form.DialogResult=DialogResult;}}}
    protected override bool IsInputKey(Keys key){return key==Keys.Space||key==Keys.Enter||base.IsInputKey(key);}
    protected override void OnKeyDown(KeyEventArgs e){if(e.KeyCode==Keys.Space||e.KeyCode==Keys.Enter){pressed=true;e.Handled=true;Invalidate();}base.OnKeyDown(e);}
    protected override void OnKeyUp(KeyEventArgs e){if(pressed&&(e.KeyCode==Keys.Space||e.KeyCode==Keys.Enter)){pressed=false;e.Handled=true;Invalidate();PerformClick();}base.OnKeyUp(e);}
    protected override void OnLostFocus(EventArgs e){pressed=false;Invalidate();base.OnLostFocus(e);}
    protected override void OnGotFocus(EventArgs e){Invalidate();base.OnGotFocus(e);}
    static GraphicsPath Outline(RectangleF r){var p=new GraphicsPath();float d=10;p.AddArc(r.X,r.Y,d,d,180,90);p.AddArc(r.Right-d,r.Y,d,d,270,90);p.AddArc(r.Right-d,r.Bottom-d,d,d,0,90);p.AddArc(r.X,r.Bottom-d,d,d,90,90);p.CloseFigure();return p;}
    protected override void OnMouseEnter(EventArgs e){hovered=true;Invalidate();base.OnMouseEnter(e);}
    protected override void OnMouseLeave(EventArgs e){hovered=false;pressed=false;Invalidate();base.OnMouseLeave(e);}
    protected override void OnMouseDown(MouseEventArgs e){pressed=e.Button==MouseButtons.Left;if(pressed)Focus();Invalidate();base.OnMouseDown(e);}
    protected override void OnMouseUp(MouseEventArgs e){pressed=false;Invalidate();base.OnMouseUp(e);}
    protected override void OnEnabledChanged(EventArgs e){Invalidate();base.OnEnabledChanged(e);}
    protected override void OnPaintBackground(PaintEventArgs e){
        var launcher=FindForm() as Launcher;
        if(launcher==null){e.Graphics.Clear(Parent==null?Color.FromArgb(22,25,36):Parent.BackColor);return;}
        e.Graphics.SetClip(ClientRectangle,CombineMode.Intersect);
        Point position=launcher.PointToClient(PointToScreen(Point.Empty));var state=e.Graphics.Save();
        e.Graphics.TranslateTransform(-position.X,-position.Y);
        launcher.DrawBackdrop(e.Graphics);
        if(Parent!=launcher)using(var header=new SolidBrush(Color.FromArgb(242,16,18,24)))e.Graphics.FillRectangle(header,new Rectangle(0,0,launcher.ClientSize.Width,launcher.HeaderHeight));
        e.Graphics.Restore(state);
    }
    protected override void OnPaint(PaintEventArgs e){
        if(Width<2||Height<2)return;
        e.Graphics.SmoothingMode=SmoothingMode.AntiAlias;
        using(var path=Outline(new RectangleF(.5f,.5f,Width-1,Height-1)))using(var fill=new SolidBrush(Color.FromArgb(238,pressed?35:hovered?43:22,pressed?37:hovered?45:24,pressed?49:hovered?57:32)))using(var border=new Pen(Color.FromArgb(165,67,69,80))) {e.Graphics.FillPath(fill,path);e.Graphics.DrawPath(border,path);}
        if(Image!=null){float scale=Height/54f*Math.Min(1,36f/Image.Width);float imageWidth=Image.Width*scale,imageHeight=Image.Height*scale;e.Graphics.DrawImage(Image,(Width-imageWidth)/2,(Height-imageHeight)/2,imageWidth,imageHeight);}
        if(!string.IsNullOrEmpty(Text))using(var ink=new SolidBrush(Enabled?ForeColor:Color.FromArgb(145,150,160)))using(var format=new StringFormat{Alignment=StringAlignment.Center,LineAlignment=StringAlignment.Center,FormatFlags=StringFormatFlags.NoWrap}){e.Graphics.TextRenderingHint=System.Drawing.Text.TextRenderingHint.AntiAliasGridFit;e.Graphics.DrawString(Text,Font,ink,ClientRectangle,format);}
        if(Focused&&ShowFocusCues)ControlPaint.DrawFocusRectangle(e.Graphics,new Rectangle(5,5,Width-10,Height-10),ForeColor,Color.Transparent);
    }
}
class LauncherHeader : Panel {
    public LauncherHeader(){SetStyle(ControlStyles.UserPaint|ControlStyles.AllPaintingInWmPaint|ControlStyles.OptimizedDoubleBuffer|ControlStyles.ResizeRedraw|ControlStyles.SupportsTransparentBackColor,true);}
    protected override CreateParams CreateParams {get{var parameters=base.CreateParams;parameters.ExStyle|=0x02000000;return parameters;}}
    protected override void OnPaintBackground(PaintEventArgs e){
        var launcher=FindForm() as Launcher;if(launcher==null){base.OnPaintBackground(e);return;}
        e.Graphics.SetClip(ClientRectangle,CombineMode.Intersect);launcher.DrawBackdrop(e.Graphics);
        using(var shade=new SolidBrush(BackColor))e.Graphics.FillRectangle(shade,ClientRectangle);
    }
}
class LauncherMenuColors : ProfessionalColorTable {
    public override Color ToolStripDropDownBackground {get{return Color.FromArgb(23,24,29);}}
    public override Color ImageMarginGradientBegin {get{return ToolStripDropDownBackground;}}
    public override Color ImageMarginGradientMiddle {get{return ToolStripDropDownBackground;}}
    public override Color ImageMarginGradientEnd {get{return ToolStripDropDownBackground;}}
    public override Color MenuItemSelected {get{return Color.FromArgb(48,50,61);}}
    public override Color MenuItemBorder {get{return Color.FromArgb(65,68,81);}}
    public override Color MenuBorder {get{return Color.FromArgb(59,61,72);}}
}
class Launcher : Form {
    const string AppVersion = "1.8.3";
    static readonly string Home = AppDomain.CurrentDomain.BaseDirectory;
    static readonly JavaScriptSerializer Json = new JavaScriptSerializer();
    static readonly string SettingsDir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "MongilModLauncher");
    Branding branding;
    Settings settings;
    TextBox game, official;
    Label status;
    LauncherButton play;
    Image artwork;
    internal int HeaderHeight=48;
    Panel titleBar;
    readonly System.Collections.Generic.Dictionary<Control,Rectangle> originalBounds=new System.Collections.Generic.Dictionary<Control,Rectangle>();
    readonly System.Collections.Generic.Dictionary<Control,Font> originalFonts=new System.Collections.Generic.Dictionary<Control,Font>();
    readonly System.Collections.Generic.Dictionary<string,Font> scaledFonts=new System.Collections.Generic.Dictionary<string,Font>();
    bool arranging;
    void ArrangeWindow(){
        if(titleBar==null||originalBounds.Count==0||arranging||WindowState==FormWindowState.Minimized||ClientSize.Width<100||ClientSize.Height<100)return;
        arranging=true;SuspendLayout();titleBar.SuspendLayout();
        try{
            float scale=Math.Min(ClientSize.Width/1120f,ClientSize.Height/678f);HeaderHeight=(int)Math.Round(48*scale);titleBar.Height=HeaderHeight;
            foreach(var pair in originalBounds){var control=pair.Key;var r=pair.Value;
                int x=(int)Math.Round(r.X*scale),y=(int)Math.Round(r.Y*scale);
                if(control.Parent==titleBar){if(r.X>=976)x=ClientSize.Width-(int)Math.Round((1120-r.X)*scale);else if(control is Label)x=(ClientSize.Width-(int)Math.Round(r.Width*scale))/2;}
                else {y=ClientSize.Height-(int)Math.Round((678-r.Y)*scale);if(r.X>=844)x=ClientSize.Width-(int)Math.Round((1120-r.X)*scale);}
                control.Bounds=new Rectangle(x,y,(int)Math.Round(r.Width*scale),(int)Math.Round(r.Height*scale));
                if(control==status)control.Width=ClientSize.Width-2*x;
                var original=originalFonts[control];float fontSize=Math.Max(1,(float)Math.Round(original.Size*scale*4)/4);
                string key=original.FontFamily.Name+"|"+fontSize.ToString(System.Globalization.CultureInfo.InvariantCulture)+"|"+(int)original.Style;
                Font font;if(!scaledFonts.TryGetValue(key,out font)){font=new Font(original.FontFamily,fontSize,original.Style);scaledFonts.Add(key,font);}control.Font=font;
            }
            // Font assignment may retain an equal existing Font. Keep cached fonts alive until controls are disposed.
        }finally{titleBar.ResumeLayout(false);ResumeLayout(false);arranging=false;}
        Invalidate(true);
    }
    protected override void OnResize(EventArgs e){base.OnResize(e);ArrangeWindow();Invalidate(true);}
    protected override void WndProc(ref Message message){
        if(message.Msg==0x84&&WindowState==FormWindowState.Normal){
            long packed=message.LParam.ToInt64();var point=PointToClient(new Point((short)(packed&65535),(short)((packed>>16)&65535)));int edge=8;
            bool left=point.X<edge,right=point.X>=ClientSize.Width-edge,top=point.Y<edge,bottom=point.Y>=ClientSize.Height-edge;
            int hit=top?(left?13:right?14:12):bottom?(left?16:right?17:15):left?10:right?11:0;
            if(hit!=0){message.Result=new IntPtr(hit);return;}
        }
        base.WndProc(ref message);
    }
    [DllImport("user32.dll")] static extern bool ReleaseCapture();
    [DllImport("user32.dll")] static extern IntPtr SendMessage(IntPtr window,uint message,IntPtr wParam,IntPtr lParam);

    bool watching;
    bool closing;
    bool explicitExit;
    string installedFolder;
    LauncherSession session;
    NotifyIcon tray;

    static bool Running() { return Process.GetProcessesByName("BigCat-Win64-Shipping").Length != 0; }
    static bool Https(string value) { Uri uri; return Uri.TryCreate(value, UriKind.Absolute, out uri) && uri.Scheme == "https"; }
    static T Read<T>(string path) where T : new() { return File.Exists(path) ? Json.Deserialize<T>(File.ReadAllText(path)) : new T(); }
    static string Digest(byte[] bytes) { using(var hash=SHA256.Create()) return BitConverter.ToString(hash.ComputeHash(bytes)).Replace("-", "").ToLowerInvariant(); }
    static byte[] Payload(string name) { using(var s=Assembly.GetExecutingAssembly().GetManifestResourceStream(name)) using(var m=new MemoryStream()) { if(s==null) throw new IOException("Missing bundled loader: " + name); s.CopyTo(m); return m.ToArray(); } }
    // Hashes of earlier loader installations; no historical executables are embedded.
    static bool RecognizedModule(string name,byte[] bytes) {
        string digest=Digest(bytes);
        if(digest==Digest(Payload(name)))return true;
        if(name=="version.dll")return new[]{"b2190b2b3666027f00734876aba47e970d1d60fe63e8af17eaaedd739a4fec01","2f5460a75b673846a930529de4324dae0130591960f8ef839abdf2e73c6486f2","87445b6028e3ce4560f0002d52345852a56acca30169e19549dfb727931da868","559094e78013e6dda9e26cc8ee64a4166bf2fe8c41e2bb09bd22820428d61954","cf2ebed93442535d32679655ec16d6c5a29d9f86fafb59eaf07de4d719ae1302","5240b0ff2a96cec2680d35df88796279e6f3ed47f0e395252e9c3e4288e6c132"}.Contains(digest);
        if(name=="StardiveMount.asi")return new[]{"0dc0425deb124080ea371dc14bae7bb9d0da7d830f9d6a700176488d4a909b7a","2522cffba1afd791db4cc3479fcc630d781d6dc324d5e700046d2405a41bfa9d","90c2e279106d05c9af4de4f5eab952cb552a605a4fcddd5d9fdd1228cf71b94f","5ceff22c62ef0d60671a58490b1c1ba14aa923dec5d2a5c5cf9de992cdfdfc97","32390d0a4dd0429cebd2d08580f2a5d7138ea2a0586946b286221f5e8775fed8"}.Contains(digest);
        if(name=="MSDCamera.asi")return new[]{"47f2178c0fe6a531b60b0313cf34a936641139d563cec3238c7a127c059e616e","4e43735b4211828a56bb4c30978df8a66dab28eecfd2357bdc09a4ed4d9db019"}.Contains(digest);
        return false;
    }
    void Save() { settings.gameRoot=game.Text.Trim(); settings.officialLauncher=official.Text.Trim(); Directory.CreateDirectory(SettingsDir); File.WriteAllText(Path.Combine(SettingsDir,"settings.json"),Json.Serialize(settings)); }
    Label Caption(string text, int y) { var label=new Label {Text=text,Left=28,Top=y,Width=640,Height=26,ForeColor=Color.FromArgb(210,216,232)}; Controls.Add(label); return label; }
    LauncherButton ActionButton(string text,int x,int y,int width,EventHandler action) { var b=new LauncherButton {Text=text,Left=x,Top=y,Width=width,Height=38,ForeColor=Color.White};b.Click+=action;Controls.Add(b);return b; }
    public Launcher() {
        branding=new Branding();settings=Read<Settings>(Path.Combine(SettingsDir,"settings.json"));
        Text="Star Dive Mod Launcher";Icon=Icon.ExtractAssociatedIcon(Application.ExecutablePath); ClientSize=new Size(1120,678); MinimumSize=new Size(900,570); FormBorderStyle=FormBorderStyle.None;
        SetStyle(ControlStyles.ResizeRedraw,true);AutoScaleMode=AutoScaleMode.None;
        DoubleBuffered=true; BackColor=Color.FromArgb(16,18,24); ForeColor=Color.White; Font=new Font("Segoe UI",10); StartPosition=FormStartPosition.CenterScreen;
        using(var stream=Assembly.GetExecutingAssembly().GetManifestResourceStream("background.jpg"))using(var image=Image.FromStream(stream)) artwork=new Bitmap(image);
        game=new TextBox {Text=settings.gameRoot}; official=new TextBox {Text=settings.officialLauncher};
        Shown+=async delegate {
            if(Environment.GetCommandLineArgs().Contains("--instance-test-host")||Environment.GetCommandLineArgs().Contains("--preview")||Environment.GetCommandLineArgs().Contains("--preview-hover")||Environment.GetCommandLineArgs().Contains("--repaint-test")||Environment.GetCommandLineArgs().Contains("--resize-test")||Environment.GetCommandLineArgs().Contains("--tray-test"))return;
            if(Environment.GetCommandLineArgs().Contains("--quick-start"))HideToTray();
            if(Environment.GetCommandLineArgs().Contains("--video-test"))return;
            if(!InstallDetection.GameValid(game.Text)||!InstallDetection.LauncherValid(official.Text)){
                status.Text="Looking for STAR DIVE and Netmarble…";
                string selectedLauncher=official.Text,selectedGame=game.Text;
                var found=await Task.Run(()=>InstallDetection.FindFor(selectedGame,selectedLauncher));if(closing)return;ApplyDetection(found);
                if(!InstallDetection.GameValid(game.Text)||!InstallDetection.LauncherValid(official.Text)){status.Text="Select the missing installation paths in Settings.";OpenSettings();}
                else status.Text="Installation found. Ready to launch.";
            }
            if(!closing&&Environment.GetCommandLineArgs().Contains("--quick-start"))await LaunchGame();
        };
        var header=new LauncherHeader {Dock=DockStyle.Top,Height=48,BackColor=Color.FromArgb(242,16,18,24)};Controls.Add(header);titleBar=header;
        header.MouseDown+=delegate(object sender,MouseEventArgs e){if(e.Button==MouseButtons.Left){ReleaseCapture();SendMessage(Handle,0xA1,new IntPtr(2),IntPtr.Zero);}};
        var credit=new Label {Text="Star Dive Mod Launcher",BackColor=Color.Transparent,AutoSize=false,Width=400,Height=48,Left=360,Top=0,TextAlign=ContentAlignment.MiddleCenter,Font=new Font("Segoe UI",10,FontStyle.Bold),ForeColor=Color.FromArgb(225,227,236)};
        credit.MouseDown+=delegate(object sender,MouseEventArgs e){if(e.Button==MouseButtons.Left){ReleaseCapture();SendMessage(Handle,0xA1,new IntPtr(2),IntPtr.Zero);}};
        header.Controls.Add(credit);
        var gear=ActionButton("⚙",0,0,52,delegate{OpenSettings();});gear.Height=48;gear.Font=new Font("Segoe UI Symbol",19);Controls.Remove(gear);header.Controls.Add(gear);
        var minimize=ActionButton("—",976,0,48,delegate{HideToTray();});
        var maximize=ActionButton("□",1024,0,48,delegate{WindowState=WindowState==FormWindowState.Maximized?FormWindowState.Normal:FormWindowState.Maximized;});
        var close=ActionButton("×",1072,0,48,delegate{Close();});
        foreach(var control in new[]{minimize,maximize,close}){control.Height=48;control.Anchor=AnchorStyles.Top|AnchorStyles.Right;Controls.Remove(control);header.Controls.Add(control);}
        var discord=ActionButton("",22,596,54,delegate{OpenLink(branding.discordUrl,"Discord");});discord.Height=54;discord.Anchor=AnchorStyles.Bottom|AnchorStyles.Left;
        var patreon=ActionButton("",76,596,54,delegate{OpenLink(branding.patreonUrl,"Patreon");});patreon.Height=54;patreon.Anchor=AnchorStyles.Bottom|AnchorStyles.Left;
        discord.Image=SocialIcon(true);patreon.Image=SocialIcon(false);discord.AccessibleName="Discord";patreon.AccessibleName="Patreon";var tips=new ToolTip();tips.SetToolTip(discord,"Join the modding Discord");tips.SetToolTip(patreon,"Support Sonicarrito on Patreon");
        var gamebanana=ActionButton("",130,596,54,delegate{OpenLink("https://gamebanana.com/tools/24388","GameBanana");});gamebanana.Height=54;gamebanana.AccessibleName="GameBanana";
        using(var stream=Assembly.GetExecutingAssembly().GetManifestResourceStream("gamebanana.png"))using(var image=Image.FromStream(stream))gamebanana.Image=new Bitmap(image);
        tips.SetToolTip(gamebanana,"Star Dive Mod Launcher on GameBanana");
        var manager=ActionButton("Mod Manager",198,596,160,delegate{Guard(OpenModManager);});manager.Height=54;manager.Anchor=AnchorStyles.Bottom|AnchorStyles.Left;
        var menu=new ContextMenuStrip {Renderer=new ToolStripProfessionalRenderer(new LauncherMenuColors()),BackColor=Color.FromArgb(23,24,29),ForeColor=Color.FromArgb(230,231,237),ShowImageMargin=false,Font=new Font("Segoe UI",10),Padding=new Padding(6)};
        menu.Items.Add("Open Mods Folder",null,delegate{Guard(delegate{Process.Start(ModsFolder());});});menu.Items.Add("Mod Manager",null,delegate{Guard(OpenModManager);});menu.Items.Add("Settings",null,delegate{OpenSettings();});menu.Items.Add("Add Desktop Shortcut",null,delegate{Guard(AddShortcut);});menu.Items.Add("Add Quick Start Shortcut",null,delegate{Guard(delegate{AddShortcut(true);});});
        menu.Items.Add("Open Official Launcher",null,delegate{Guard(delegate{if(!File.Exists(settings.officialLauncher))throw new IOException("Choose the official launcher in Settings first.");Process.Start(new ProcessStartInfo(settings.officialLauncher){UseShellExecute=true,WorkingDirectory=Path.GetDirectoryName(settings.officialLauncher)});});});
        foreach(ToolStripItem item in menu.Items){item.Padding=new Padding(12,10,20,10);item.ForeColor=Color.FromArgb(230,231,237);}
        var more=ActionButton("⋮",1054,596,44,delegate(object sender,EventArgs e){var control=(Control)sender;menu.Show(control,new Point(control.Width-menu.PreferredSize.Width,-menu.PreferredSize.Height-8));});more.Height=54;more.Anchor=AnchorStyles.Bottom|AnchorStyles.Right;more.Font=new Font("Segoe UI",18);
        play=ActionButton("▶   Launch",844,596,200,async delegate {await LaunchGame();});
        play.Height=54;play.Font=new Font("Segoe UI",12,FontStyle.Bold);play.Anchor=AnchorStyles.Bottom|AnchorStyles.Right;
        status=Caption("Ready to launch  •  v"+AppVersion,559);status.Left=24;status.Width=1050;status.Height=26;status.BackColor=Color.Transparent;status.Anchor=AnchorStyles.Bottom|AnchorStyles.Left|AnchorStyles.Right;

        foreach(Control control in Controls){if(control==header)continue;originalBounds[control]=control.Bounds;originalFonts[control]=control.Font;control.Anchor=AnchorStyles.Top|AnchorStyles.Left;}
        foreach(Control control in header.Controls){originalBounds[control]=control.Bounds;originalFonts[control]=control.Font;control.Anchor=AnchorStyles.Top|AnchorStyles.Left;}
        ArrangeWindow();
        Disposed+=delegate{foreach(var font in scaledFonts.Values)font.Dispose();};
        tray=new NotifyIcon {Icon=Icon,Text="Star Dive Mod Launcher"};tray.DoubleClick+=delegate{ShowFromTray();};
        var trayMenu=new ContextMenuStrip();trayMenu.Items.Add("Show launcher",null,delegate{ShowFromTray();});
        trayMenu.Items.Add(new ToolStripSeparator());trayMenu.Items.Add("Exit launcher",null,delegate{explicitExit=true;Close();});tray.ContextMenuStrip=trayMenu;
        FormClosing+=delegate(object sender,FormClosingEventArgs e){if(!explicitExit&&watching&&Running()){e.Cancel=true;HideToTray();return;}closing=true;Save();StopSession();if(!Running())Guard(RemoveInstalledLoader);tray.Dispose();};
    }
    delegate bool WindowVisitor(IntPtr window,IntPtr parameter);
    [DllImport("user32.dll")] static extern bool EnumWindows(WindowVisitor visitor,IntPtr parameter);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr window,out uint processId);
    [DllImport("user32.dll")] static extern bool ShowWindow(IntPtr window,int command);
    [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr window);
    static int HideWindowsForExecutable(string executable) {
        if(string.IsNullOrEmpty(executable))return 0;
        var ids=new System.Collections.Generic.HashSet<uint>();
        foreach(var process in Process.GetProcessesByName(Path.GetFileNameWithoutExtension(executable)))using(process){
            try{if(string.Equals(Path.GetFullPath(process.MainModule.FileName),Path.GetFullPath(executable),StringComparison.OrdinalIgnoreCase))ids.Add((uint)process.Id);}catch(System.ComponentModel.Win32Exception){}catch(InvalidOperationException){}
        }
        int hidden=0;
        EnumWindows(delegate(IntPtr window,IntPtr parameter){uint id;GetWindowThreadProcessId(window,out id);if(ids.Contains(id)&&IsWindowVisible(window)){ShowWindow(window,0);hidden++;}return true;},IntPtr.Zero);
        return hidden;
    }
    Rectangle lastVisibleBounds;
    [DllImport("user32.dll")] static extern bool IsIconic(IntPtr window);
    void HideToTray(){
        var normal=WindowState==FormWindowState.Normal?Bounds:RestoreBounds;
        if(normal.Width>=MinimumSize.Width&&normal.Height>=MinimumSize.Height&&Screen.AllScreens.Any(screen=>screen.WorkingArea.IntersectsWith(normal)))lastVisibleBounds=normal;
        tray.Visible=true;
        // Hiding a top-level window already removes its taskbar entry. Keep its handle intact.
        Hide();
    }
    void ShowFromTray(){
        if(closing||IsDisposed)return;
        Rectangle target=lastVisibleBounds;
        if(target.Width<MinimumSize.Width||target.Height<MinimumSize.Height)target=new Rectangle(0,0,1120,678);
        if(!Screen.AllScreens.Any(screen=>screen.WorkingArea.IntersectsWith(target))||lastVisibleBounds.IsEmpty){var area=Screen.PrimaryScreen.WorkingArea;target.Location=new Point(area.Left+Math.Max(0,(area.Width-target.Width)/2),area.Top+Math.Max(0,(area.Height-target.Height)/2));}
        WindowState=FormWindowState.Normal;Bounds=target;
        Show();ShowWindow(Handle,9);WindowState=FormWindowState.Normal;Bounds=target;
        ArrangeWindow();Invalidate(true);Update();BringToFront();Activate();
        tray.Visible=!Visible||IsIconic(Handle)||!IsWindowVisible(Handle);
    }
    protected override void OnPaintBackground(PaintEventArgs e) {
        DrawBackdrop(e.Graphics);
    }
    internal void DrawBackdrop(Graphics graphics){
        var e=new PaintEventArgs(graphics,ClientRectangle);
        using(var background=new SolidBrush(BackColor))e.Graphics.FillRectangle(background,ClientRectangle);if(artwork==null||ClientSize.Height<=HeaderHeight)return;
        var bounds=ClientRectangle;
        float scale=Math.Max((float)bounds.Width/artwork.Width,(float)bounds.Height/artwork.Height);
        float width=artwork.Width*scale,height=artwork.Height*scale;
        e.Graphics.InterpolationMode=InterpolationMode.HighQualityBicubic;
        e.Graphics.DrawImage(artwork,bounds.X+(bounds.Width-width)/2,bounds.Y+(bounds.Height-height)/2,width,height);
        using(var tint=new SolidBrush(Color.FromArgb(30,6,9,18)))e.Graphics.FillRectangle(tint,bounds);
        int footerHeight=(int)(150*Math.Min(ClientSize.Width/1120f,ClientSize.Height/678f));var footer=new Rectangle(0,ClientSize.Height-footerHeight,ClientSize.Width,footerHeight);
        // Extend the gradient beyond the filled area so its tiled endpoint cannot create a dark seam.
        using(var shade=new LinearGradientBrush(new Point(0,footer.Top-2),new Point(0,footer.Bottom+2),Color.FromArgb(0,10,12,20),Color.FromArgb(235,10,12,20))){shade.WrapMode=WrapMode.TileFlipXY;e.Graphics.FillRectangle(shade,footer);}
    }
    static Bitmap SocialIcon(bool discord) {
        var bitmap=new Bitmap(28,28);using(var g=Graphics.FromImage(bitmap)) {
            g.SmoothingMode=SmoothingMode.AntiAlias;
            using(var white=new SolidBrush(Color.FromArgb(232,233,239)))using(var cutout=new SolidBrush(Color.FromArgb(22,24,32)))using(var path=new GraphicsPath()) {
                if(discord) {
                    path.AddBezier(3,9,6,6,9,6,11,6);path.AddLine(11,6,12,8);path.AddLine(12,8,16,8);path.AddLine(16,8,17,6);path.AddBezier(17,6,21,6,23,7,25,10);path.AddBezier(25,10,27,14,27,19,25,21);path.AddLine(25,21,20,23);path.AddLine(20,23,18,20);path.AddBezier(18,20,15,21,12,21,10,20);path.AddLine(10,20,8,23);path.AddLine(8,23,3,21);path.AddBezier(3,21,0,18,1,13,3,9);path.CloseFigure();g.FillPath(white,path);g.FillEllipse(cutout,7,12,4,5);g.FillEllipse(cutout,17,12,4,5);
                }else {
                    path.AddBezier(4,5,10,0,22,1,25,7);path.AddBezier(25,7,29,15,22,20,17,21);path.AddBezier(17,21,12,22,11,29,6,25);path.AddBezier(6,25,1,21,0,10,4,5);path.CloseFigure();g.FillPath(white,path);
                }
            }
        }return bitmap;
    }
    void OpenLink(string url,string name) {if(!Https(url)){MessageBox.Show(name+" link has not been configured yet.");return;}Guard(delegate{Process.Start(url);});}
    async Task LaunchGame(){
        if(watching)return;
        try {if(!File.Exists(settings.officialLauncher)||!Directory.Exists(settings.gameRoot))OpenSettings();if(closing)return;Save();var paired=InstallDetection.FindFor(settings.gameRoot,settings.officialLauncher);if(!InstallDetection.LauncherValid(paired.officialLauncher))throw new IOException("Choose the official launcher that corresponds to this STAR DIVE installation.");official.Text=paired.officialLauncher;Save();Install();
            if(InstallDetection.IsEpic(settings.officialLauncher))Process.Start(new ProcessStartInfo(InstallDetection.EpicLaunchUri()){UseShellExecute=true});
            else Process.Start(new ProcessStartInfo(settings.officialLauncher,"--productcode=/Game/monster2 --buildcode=A --executedbyexternallauncher=true --hidden"){UseShellExecute=true,WindowStyle=ProcessWindowStyle.Minimized,WorkingDirectory=Path.GetDirectoryName(settings.officialLauncher)});
            await WatchGame();
        }catch(Exception e){StopSession();if(!Running())Guard(RemoveInstalledLoader);ShowFromTray();status.Text=e.Message;MessageBox.Show(e.Message,"Launcher");}
    }
    void AddShortcut(){AddShortcut(false);}
    void AddShortcut(bool quickStart) {
        string path=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory),quickStart?"Star Dive Quick Start.lnk":"Star Dive Mod Launcher.lnk");
        WriteShortcut(path,quickStart);status.Text=quickStart?"Quick Start shortcut created.":"Desktop shortcut created.";
    }
    static void WriteShortcut(string path,bool quickStart){
        Type shellType=Type.GetTypeFromProgID("WScript.Shell");object shell=Activator.CreateInstance(shellType);object shortcut=null;
        try {
            shortcut=shellType.InvokeMember("CreateShortcut",BindingFlags.InvokeMethod,null,shell,new object[]{path});Type type=shortcut.GetType();
            type.InvokeMember("TargetPath",BindingFlags.SetProperty,null,shortcut,new object[]{Application.ExecutablePath});
            type.InvokeMember("Arguments",BindingFlags.SetProperty,null,shortcut,new object[]{quickStart?"--quick-start":""});
            type.InvokeMember("WorkingDirectory",BindingFlags.SetProperty,null,shortcut,new object[]{Home});
            type.InvokeMember("IconLocation",BindingFlags.SetProperty,null,shortcut,new object[]{Application.ExecutablePath+",0"});
            type.InvokeMember("Save",BindingFlags.InvokeMethod,null,shortcut,null);
        }finally{if(shortcut!=null)Marshal.ReleaseComObject(shortcut);Marshal.ReleaseComObject(shell);}
    }
    string ModsFolder() {
        Save();string win64=Path.Combine(settings.gameRoot,"BigCat","Binaries","Win64");
        if(!File.Exists(Path.Combine(win64,"BigCat-Win64-Shipping.exe"))){OpenSettings();Save();win64=Path.Combine(settings.gameRoot,"BigCat","Binaries","Win64");}
        if(!File.Exists(Path.Combine(win64,"BigCat-Win64-Shipping.exe")))throw new IOException("Choose a valid STAR DIVE installation in Settings.");
        string folder=Path.Combine(win64,"Mods");Directory.CreateDirectory(folder);return folder;
    }
    static System.Collections.Generic.IEnumerable<string> ModFiles(string folder) {
        foreach(string file in Directory.GetFiles(folder))if((File.GetAttributes(file)&FileAttributes.ReparsePoint)==0)yield return file;
        foreach(string child in Directory.GetDirectories(folder)) {
            if((File.GetAttributes(child)&FileAttributes.ReparsePoint)!=0)continue;
            foreach(string file in ModFiles(child))yield return file;
        }
    }
    void OpenModManager() {
        string folder=ModsFolder();
        using(var dialog=new Form {Text="Mod Manager",ClientSize=new Size(710,440),StartPosition=FormStartPosition.CenterParent,BackColor=Color.FromArgb(22,25,36),ForeColor=Color.White,Font=Font}) {
            var list=new ListView {View=View.Details,FullRowSelect=true,MultiSelect=false,Left=20,Top=48,Width=670,Height=307,Anchor=AnchorStyles.Top|AnchorStyles.Bottom|AnchorStyles.Left|AnchorStyles.Right,BackColor=Color.FromArgb(28,32,44),ForeColor=Color.White};
            list.Columns.Add("Mod",370);list.Columns.Add("Status",150);list.Columns.Add("Files",125);dialog.Controls.Add(list);
            dialog.Controls.Add(new Label {Text="Select a mod to enable or disable it. Restart the game to apply changes.",Left=20,Top=17,Width=680});
            Action refresh=delegate {
                list.Items.Clear();
                foreach(string file in ModFiles(folder).OrderBy(p=>p,StringComparer.OrdinalIgnoreCase)) {
                    bool disabled=file.EndsWith("_P.pak.disabled",StringComparison.OrdinalIgnoreCase);bool enabled=file.EndsWith("_P.pak",StringComparison.OrdinalIgnoreCase);if(!disabled&&!enabled)continue;
                    string suffix=disabled?".disabled":"";string stem=file.Substring(0,file.Length-(disabled?13:4));
                    bool complete=File.Exists(stem+".utoc"+suffix)&&File.Exists(stem+".ucas"+suffix);
                    var item=new ListViewItem(stem.Substring(folder.TrimEnd('\\').Length+1));item.SubItems.Add(disabled?"Disabled":"Enabled");item.SubItems.Add(complete?"Complete":"Missing companion");item.Tag=file;list.Items.Add(item);
                }
            };
            Action toggle=delegate {
                if(Running())throw new IOException("Close STAR DIVE before changing enabled mods.");
                if(list.SelectedItems.Count==0)return;string file=(string)list.SelectedItems[0].Tag;
                bool disabled=file.EndsWith(".disabled",StringComparison.OrdinalIgnoreCase);string suffix=disabled?".disabled":"";
                string stem=file.Substring(0,file.Length-(disabled?13:4));var moves=new System.Collections.Generic.List<string[]>();
                foreach(string extension in new[]{".pak",".utoc",".ucas"}) {
                    string from=stem+extension+suffix;if(!File.Exists(from))continue;string to=stem+extension+(disabled?"":".disabled");
                    if(File.Exists(to))throw new IOException("A file already exists at "+Path.GetFileName(to)+". No changes made.");moves.Add(new[]{from,to});
                }
                int moved=0;try{foreach(var pair in moves){File.Move(pair[0],pair[1]);++moved;}}catch{for(int i=moved-1;i>=0;--i)File.Move(moves[i][1],moves[i][0]);throw;}
                refresh();
            };
            var change=new LauncherButton {Text="Enable / Disable",Left=20,Top=375,Width=150,Height=38,Anchor=AnchorStyles.Bottom|AnchorStyles.Left};change.Click+=delegate{Guard(toggle);};dialog.Controls.Add(change);
            var open=new LauncherButton {Text="Open Mods folder",Left=180,Top=375,Width=150,Height=38,Anchor=AnchorStyles.Bottom|AnchorStyles.Left};open.Click+=delegate{Guard(delegate{Process.Start(folder);});};dialog.Controls.Add(open);
            var reload=new LauncherButton {Text="Refresh",Left=340,Top=375,Width=110,Height=38,Anchor=AnchorStyles.Bottom|AnchorStyles.Left};reload.Click+=delegate{Guard(refresh);};dialog.Controls.Add(reload);
            list.DoubleClick+=delegate{Guard(toggle);};refresh();dialog.ShowDialog(this);
        }
    }
    void ApplyDetection(DetectedInstallation found) {
        if(!InstallDetection.GameValid(game.Text)&&InstallDetection.GameValid(found.gameRoot))game.Text=found.gameRoot;
        if(!InstallDetection.LauncherValid(official.Text)&&InstallDetection.LauncherValid(found.officialLauncher))official.Text=found.officialLauncher;
        Save();
    }
    Form CreateSettingsDialog() {
        var dialog=new Form {Text="Launcher settings",ClientSize=new Size(650,300),StartPosition=FormStartPosition.CenterParent,FormBorderStyle=FormBorderStyle.FixedDialog,MaximizeBox=false,MinimizeBox=false,BackColor=Color.FromArgb(22,25,36),ForeColor=Color.White,Font=Font};
            var gameLabel=new Label {Text=InstallDetection.GameValid(game.Text)?"STAR DIVE folder (contains BigCat)":"STAR DIVE not found — select its folder using Browse.",Left=22,Top=20,Width=600};dialog.Controls.Add(gameLabel);
            game.SetBounds(22,50,492,28);dialog.Controls.Add(game);
            var gameBrowse=new LauncherButton {Text="Browse",Left=525,Top=47,Width=100,Height=30};gameBrowse.Click+=delegate{using(var d=new FolderBrowserDialog())if(d.ShowDialog(dialog)==DialogResult.OK)game.Text=d.SelectedPath;};dialog.Controls.Add(gameBrowse);
            var launcherLabel=new Label {Text=InstallDetection.LauncherValid(official.Text)?"Official Netmarble launcher":"Netmarble not found — select Netmarble Launcher.exe using Browse.",Left=22,Top=101,Width=600};dialog.Controls.Add(launcherLabel);
            official.SetBounds(22,131,492,28);dialog.Controls.Add(official);
            var launcherBrowse=new LauncherButton {Text="Browse",Left=525,Top=128,Width=100,Height=30};launcherBrowse.Click+=delegate{using(var d=new OpenFileDialog {Filter="Launcher executable|*.exe"})if(d.ShowDialog(dialog)==DialogResult.OK)official.Text=d.FileName;};dialog.Controls.Add(launcherBrowse);
            var cameraUnlock=new CheckBox {Name="CameraUnlock",Text="Enable camera unlock (full tilt and close zoom)",Left=22,Top=179,Width=600,Height=26,Checked=settings.enableCameraUnlock,ForeColor=Color.White,BackColor=dialog.BackColor};
            cameraUnlock.CheckedChanged+=delegate{settings.enableCameraUnlock=cameraUnlock.Checked;};dialog.Controls.Add(cameraUnlock);
            dialog.Controls.Add(new Label {Text="Applies the next time you launch STAR DIVE.",Left=43,Top=210,Width=580,Height=23,ForeColor=Color.FromArgb(175,182,196)});
            var detect=new LauncherButton {Text="Auto-detect",Left=22,Top=247,Width=120,Height=32};detect.Click+=async delegate{detect.Enabled=false;string selectedGame=game.Text,selectedLauncher=official.Text;var found=await Task.Run(()=>InstallDetection.FindFor(selectedGame,selectedLauncher));if(dialog.IsDisposed)return;game.Text=found.gameRoot;official.Text=found.officialLauncher;gameLabel.Text=InstallDetection.GameValid(game.Text)?"STAR DIVE found":"STAR DIVE not found — finish installing or use Browse.";launcherLabel.Text=InstallDetection.LauncherValid(official.Text)?"Official launcher found":"Launcher not found — select its executable using Browse.";detect.Enabled=true;};dialog.Controls.Add(detect);
            var done=new LauncherButton {Text="Save",Left=525,Top=247,Width=100,Height=32};done.Click+=delegate{Save();dialog.Close();};dialog.Controls.Add(done);dialog.AcceptButton=done;
        return dialog;
    }
    void OpenSettings() {
        if(watching)return;
        if(!Visible||WindowState==FormWindowState.Minimized)ShowFromTray();
        using(var dialog=CreateSettingsDialog()) {
            try{dialog.ShowDialog(this);Save();}finally{dialog.Controls.Remove(game);dialog.Controls.Remove(official);}
        }
    }
    [DllImport("kernel32.dll",CharSet=CharSet.Unicode,SetLastError=true)] static extern bool WritePrivateProfileString(string section,string key,string value,string file);
    static void ApplyCameraSetting(string target,bool enabled) {
        string ini=Path.GetFullPath(Path.Combine(target,"MSDCamera.ini"));
        if(!File.Exists(ini))File.WriteAllText(ini,"[Camera]\r\nTiltDegrees=90\r\nCloseZoom=1\r\nMinimumDistance=25\r\nPreventCameraFade=1\r\n");
        if(!WritePrivateProfileString("Camera","Enabled",enabled?"1":"0",ini))throw new IOException("Could not save the camera unlock setting.");
    }
    void Guard(Action action) {try {action();} catch(Exception e) {MessageBox.Show(this,e.Message,"MONGIL Mod Launcher",MessageBoxButtons.OK,MessageBoxIcon.Information);status.Text=e.Message;} }
    void Install() {
        Save(); if(Running()) throw new IOException("Close STAR DIVE before installing the loader.");
        string target=Path.Combine(settings.gameRoot,"BigCat","Binaries","Win64");
        string exe=Path.Combine(target,"BigCat-Win64-Shipping.exe");if(!File.Exists(exe))throw new IOException("Select the STAR DIVE folder that contains BigCat.");
        if(Digest(File.ReadAllBytes(exe))!="dc52b31fe35f90439c5b1029b689d3cb4f090f3f941eeda62034c2db8bb5f253") throw new IOException("This game build differs from the tested build. A compatible loader update is required.");
        var names=new[]{"version.dll","StardiveMount.asi","MSDCamera.asi"};
        // Validate all destinations before writing. Preserve unknown modules.
        foreach(string name in names) {
            string path=Path.Combine(target,name);
            if(File.Exists(path)&&!RecognizedModule(name,File.ReadAllBytes(path))) {
                string known=Path.Combine(SettingsDir,"installed-"+name+".sha256");
                if(!File.Exists(known)||File.ReadAllText(known)!=Digest(File.ReadAllBytes(path))) throw new IOException("An existing " + name + " is present. Back it up and remove it before first installation; it may belong to another loader.");
            }
        }
        string legacy=Path.Combine(target,"d3d12.dll");
        if(File.Exists(legacy)&&Digest(File.ReadAllBytes(legacy))!="0e93132f801b64f1f78582efcfdb7d9be0f55999fd4e01df0eed5b48155ce1fe")throw new IOException("An unrecognized d3d12.dll is present. Back it up and check its origin before using the version.dll loader.");
        Directory.CreateDirectory(SettingsDir);
        if(File.Exists(legacy)){Directory.CreateDirectory(Path.Combine(SettingsDir,"proxy-backup"));File.Copy(legacy,Path.Combine(SettingsDir,"proxy-backup","d3d12.dll"),true);File.Delete(legacy);}
        StopSession();session=new LauncherSession(exe);
        string backup=Path.Combine(SettingsDir,"proxy-backup");Directory.CreateDirectory(backup);
        installedFolder=target;
        foreach(string name in names) {
            string path=Path.Combine(target,name);byte[] bytes=Payload(name);
            if(File.Exists(path)&&Digest(File.ReadAllBytes(path))!=Digest(bytes))File.Copy(path,Path.Combine(backup,name),true);
            File.WriteAllBytes(path,bytes);File.WriteAllText(Path.Combine(SettingsDir,"installed-"+name+".sha256"),Digest(bytes));
        }
        ApplyCameraSetting(target,settings.enableCameraUnlock);
        Directory.CreateDirectory(Path.Combine(target,"Mods"));status.Text="Launcher session active. Starting through Netmarble…";
    }
    void StopSession(){if(session!=null){session.Dispose();session=null;}}
    void RemoveInstalledLoader() {
        if(Running()||string.IsNullOrEmpty(installedFolder))return;
        foreach(string name in new[]{"version.dll","StardiveMount.asi","MSDCamera.asi"}) {
            string path=Path.Combine(installedFolder,name);
            if(File.Exists(path)&&Digest(File.ReadAllBytes(path))==Digest(Payload(name)))File.Delete(path);
        }
    }
    async Task WatchGame() {
        if(watching)return;watching=true;play.Enabled=false;
        try {
            status.Text=InstallDetection.IsEpic(settings.officialLauncher)?"Requested STAR DIVE startup through Epic Games. Waiting for the game…":"Waiting for STAR DIVE. If sign-in is needed, use Open Official Launcher in the Launch menu.";
            DateTime deadline=DateTime.UtcNow.AddMinutes(10);
            while(!closing&&DateTime.UtcNow<deadline&&!Running())await Task.Delay(250);
            if(closing)return;
            if(!Running()){status.Text="Game did not start. Click Launch to try again.";return;}
            status.Text="STAR DIVE running. Mods load automatically after the content check.";
            HideToTray();
            // Netmarble's existing tray icon can reopen its hidden window.
            HideWindowsForExecutable(settings.officialLauncher);
            for(int attempt=0;attempt<2&&!closing&&Running();attempt++){await Task.Delay(1000);HideWindowsForExecutable(settings.officialLauncher);}
            while(!closing&&Running())await Task.Delay(1000);
            if(!closing) {
                status.Text="Game closed; ready for next launch.";
                var noticeTimer=new System.Windows.Forms.Timer {Interval=10000};
                noticeTimer.Tick+=delegate {
                    noticeTimer.Stop();noticeTimer.Dispose();
                    if(!closing&&!watching&&status.Text=="Game closed; ready for next launch.")status.Text="";
                };
                noticeTimer.Start();
            }
        }finally{StopSession();if(!Running())Guard(RemoveInstalledLoader);watching=false;if(!closing){play.Enabled=true;ShowFromTray();}}
    }
    [STAThread] static int Main(string[] args) {
        try {
            if(args.Length==2&&args[0]=="--resize-test"){
                Application.SetUnhandledExceptionMode(UnhandledExceptionMode.ThrowException);
                Directory.CreateDirectory(args[1]);using(var form=new Launcher()){form.StartPosition=FormStartPosition.Manual;form.Location=new Point(-30000,-30000);form.Show();
                    foreach(var size in new[]{new Size(1120,678),new Size(1920,1080),new Size(900,570),new Size(1400,900),new Size(1120,678)}){
                        form.ClientSize=size;Application.DoEvents();form.Refresh();
                        if(form.play.Right>form.ClientSize.Width||form.play.Bottom>form.ClientSize.Height||form.play.Width<150) return 17;
                        using(var bitmap=new Bitmap(form.Width,form.Height)){form.DrawToBitmap(bitmap,form.ClientRectangle);bitmap.Save(Path.Combine(args[1],size.Width+"x"+size.Height+".png"));}
                    }
                    for(int step=0;step<200;step++){
                        form.ClientSize=new Size(1120+step%40,678+step%25);Application.DoEvents();form.Refresh();
                        foreach(Control control in form.Controls){control.Refresh();foreach(Control child in control.Controls)child.Refresh();}
                        if(step%25==0){form.WindowState=FormWindowState.Maximized;Application.DoEvents();form.Refresh();form.WindowState=FormWindowState.Normal;Application.DoEvents();form.Refresh();}
                    }
                    Point corner=form.PointToScreen(new Point(form.ClientSize.Width-2,form.ClientSize.Height-2));long packed=(ushort)corner.X|((long)(ushort)corner.Y<<16);
                    if(SendMessage(form.Handle,0x84,IntPtr.Zero,new IntPtr(packed)).ToInt32()!=17)return 18;
                    form.WindowState=FormWindowState.Maximized;Application.DoEvents();form.Refresh();
                    using(var bitmap=new Bitmap(form.Width,form.Height)){form.DrawToBitmap(bitmap,form.ClientRectangle);bitmap.Save(Path.Combine(args[1],"maximized.png"));}
                    form.WindowState=FormWindowState.Normal;Application.DoEvents();form.Refresh();
                    using(var bitmap=new Bitmap(form.Width,form.Height)){form.DrawToBitmap(bitmap,form.ClientRectangle);bitmap.Save(Path.Combine(args[1],"restored.png"));}
                    form.Close();}return 0;
            }
            if(args.Length==1&&args[0]=="--official-tray-test"){
                using(var form=new Form()){form.Show();Application.DoEvents();if(!IsWindowVisible(form.Handle))return 15;if(HideWindowsForExecutable(Application.ExecutablePath)<1||IsWindowVisible(form.Handle))return 16;form.Close();}return 0;
            }
            if(args.Length==2&&args[0]=="--shortcut-test"){
                Directory.CreateDirectory(args[1]);foreach(bool quick in new[]{false,true}){
                    string path=Path.Combine(args[1],quick?"Star Dive Quick Start.lnk":"Star Dive Mod Launcher.lnk");WriteShortcut(path,quick);
                    Type type=Type.GetTypeFromProgID("WScript.Shell");object shell=Activator.CreateInstance(type),link=null;
                    try{link=type.InvokeMember("CreateShortcut",BindingFlags.InvokeMethod,null,shell,new object[]{path});Type properties=link.GetType();
                        if((string)properties.InvokeMember("TargetPath",BindingFlags.GetProperty,null,link,null)!=Application.ExecutablePath)return 10;
                        if((string)properties.InvokeMember("Arguments",BindingFlags.GetProperty,null,link,null)!=(quick?"--quick-start":""))return 11;
                        if((string)properties.InvokeMember("IconLocation",BindingFlags.GetProperty,null,link,null)!=Application.ExecutablePath+",0")return 12;
                    }finally{if(link!=null)Marshal.ReleaseComObject(link);Marshal.ReleaseComObject(shell);}
                }return 0;
            }
            if(args.Length==2&&args[0]=="--detect-test"){var found=InstallDetection.Find();File.WriteAllText(args[1],Json.Serialize(found));return InstallDetection.GameValid(found.gameRoot)&&InstallDetection.LauncherValid(found.officialLauncher)?0:9;}
            if(args.Length==2&&args[0]=="--detect-epic-test"){var found=InstallDetection.Find(true);File.WriteAllText(args[1],Json.Serialize(found)+Environment.NewLine+InstallDetection.EpicLaunchUri());return InstallDetection.GameValid(found.gameRoot)&&InstallDetection.LauncherValid(found.officialLauncher)?0:9;}
            if(args.Length==3&&args[0]=="--session-test") { using(var gate=new LauncherSession(args[1]))using(var child=Process.Start(new ProcessStartInfo(args[1],"active "+"\""+args[2]+"\""){UseShellExecute=false,CreateNoWindow=true})){child.WaitForExit();return child.ExitCode;} }
            if(args.Length==1&&args[0]=="--self-test") {
                if(Payload("version.dll").Length<1000||Payload("StardiveMount.asi").Length<1000||Payload("MSDCamera.asi").Length<1000)return 3;
                if(!Https("https://example.com")||Https("http://example.com"))return 4;
                if(Digest(new byte[0])!="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")return 5;
                using(var gate=new LauncherSession(Application.ExecutablePath))using(var client=new System.IO.Pipes.NamedPipeClientStream(".",LauncherSession.PipeName(Application.ExecutablePath),System.IO.Pipes.PipeDirection.In)) {
                    client.Connect(2000);byte[] response=new byte[4];int n=0;while(n<4){int got=client.Read(response,n,4-n);if(got==0)return 6;n+=got;}if(BitConverter.ToUInt32(response,0)!=0x53444d31U)return 7;
                }
                System.Threading.Thread.Sleep(100);
                using(var client=new System.IO.Pipes.NamedPipeClientStream(".",LauncherSession.PipeName(Application.ExecutablePath),System.IO.Pipes.PipeDirection.In)){try{client.Connect(150);return 8;}catch(TimeoutException){}}
                return 0;
            }
            Application.EnableVisualStyles();Application.SetCompatibleTextRenderingDefault(false);
            if(args.Length==1&&args[0]=="--tray-test"){
                using(var form=new Launcher()){form.StartPosition=FormStartPosition.Manual;form.Location=new Point(-30000,-30000);form.Show();Application.DoEvents();form.HideToTray();Application.DoEvents();if(form.Visible||!form.tray.Visible)return 13;form.ShowFromTray();Application.DoEvents();if(!form.Visible||!form.ShowInTaskbar||form.tray.Visible||form.WindowState!=FormWindowState.Normal)return 14;
                    for(int iteration=0;iteration<30;iteration++){
                        form.WindowState=iteration%2==0?FormWindowState.Minimized:FormWindowState.Maximized;Application.DoEvents();
                        IntPtr originalHandle=form.Handle;form.HideToTray();Application.DoEvents();form.ShowFromTray();Application.DoEvents();if(form.Handle!=originalHandle)return 20;
                        if(!form.Visible||!form.ShowInTaskbar||form.tray.Visible||IsIconic(form.Handle)||!IsWindowVisible(form.Handle)||form.ClientSize.Width<800||form.ClientSize.Height<500)return 19;
                    }form.Close();}return 0;
            }
            if(args.Length==2&&args[0]=="--repaint-test"){
                Directory.CreateDirectory(args[1]);using(var form=new Launcher()){
                    form.StartPosition=FormStartPosition.Manual;form.Location=new Point(-30000,-30000);form.Show();Application.DoEvents();
                    var buttons=new System.Collections.Generic.List<LauncherButton>();Action<Control> collect=null;collect=delegate(Control parent){foreach(Control control in parent.Controls){if(control is LauncherButton)buttons.Add((LauncherButton)control);collect(control);}};collect(form);
                    MethodInfo enter=typeof(LauncherButton).GetMethod("OnMouseEnter",BindingFlags.Instance|BindingFlags.NonPublic),leave=typeof(LauncherButton).GetMethod("OnMouseLeave",BindingFlags.Instance|BindingFlags.NonPublic);LauncherButton previous=null;
                    for(int i=0;i<buttons.Count*2;i++){
                        if(previous!=null){leave.Invoke(previous,new object[]{EventArgs.Empty});previous.Refresh();}
                        var current=buttons[i%buttons.Count];enter.Invoke(current,new object[]{EventArgs.Empty});current.Refresh();Application.DoEvents();
                        using(var bitmap=new Bitmap(form.Width,form.Height)){form.DrawToBitmap(bitmap,new Rectangle(0,0,form.Width,form.Height));bitmap.Save(Path.Combine(args[1],"hover-"+i.ToString("00")+".png"));}
                        previous=current;
                    }
                }return 0;
            }
            if(args.Length==2&&(args[0]=="--preview"||args[0]=="--preview-hover")) {
                using(var form=new Launcher()) {form.StartPosition=FormStartPosition.Manual;form.Location=new Point(-30000,-30000);form.Show();Application.DoEvents();
                    if(args[0]=="--preview-hover"){Action<Control> hover=null;hover=delegate(Control parent){foreach(Control control in parent.Controls){if(control is LauncherButton)typeof(LauncherButton).GetMethod("OnMouseEnter",BindingFlags.Instance|BindingFlags.NonPublic).Invoke(control,new object[]{EventArgs.Empty});hover(control);}};hover(form);Application.DoEvents();}
                    using(var bitmap=new Bitmap(form.Width,form.Height)){form.DrawToBitmap(bitmap,new Rectangle(0,0,form.Width,form.Height));bitmap.Save(args[1]);}}return 0;
            }
            bool instanceTest=args.Contains("--instance-test-host")||args.Contains("--instance-test-client");
            string identity=System.Security.Principal.WindowsIdentity.GetCurrent().User.Value;
            string instanceName="Local\\Sonicarrito.StarDiveModLauncher."+identity+(instanceTest?".Test":"");
            using(var mutex=new Mutex(false,instanceName)){
                bool owner;try{owner=mutex.WaitOne(0);}catch(AbandonedMutexException){owner=true;}
                using(var wake=new EventWaitHandle(false,EventResetMode.AutoReset,instanceName+".Show")){
                    if(!owner){wake.Set();return 0;}
                    try{
                        var launcher=new Launcher();
                        using(var wakeTimer=new System.Windows.Forms.Timer {Interval=150}){
                            wakeTimer.Tick+=delegate{if(wake.WaitOne(0)){launcher.ShowFromTray();if(instanceTest){File.WriteAllText(args[1],"Existing instance restored: "+Process.GetCurrentProcess().Id);launcher.explicitExit=true;launcher.Close();}}};
                            wakeTimer.Start();
                            if(instanceTest){launcher.Shown+=delegate{launcher.HideToTray();};var timeout=new System.Windows.Forms.Timer{Interval=15000};timeout.Tick+=delegate{timeout.Stop();timeout.Dispose();launcher.explicitExit=true;launcher.Close();};timeout.Start();}
                            if(args.Contains("--quick-start"))launcher.WindowState=FormWindowState.Minimized;
                            Application.Run(launcher);
                        }
                    }finally{mutex.ReleaseMutex();}
                }
            }return 0;
        }catch(Exception e){MessageBox.Show(e.Message,"Launcher error");return 1;}
    }
}






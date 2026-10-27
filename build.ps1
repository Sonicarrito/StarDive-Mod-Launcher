$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$build = Join-Path $root 'build'
$dist = Join-Path $root 'dist'
$mh = Join-Path $root 'third_party\minhook'
if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) { throw 'Run build.cmd to load the x64 Visual C++ toolchain.' }
function Run([string]$program, [string[]]$arguments) {
    & $program @arguments
    if ($LASTEXITCODE -ne 0) { throw "$program failed with exit code $LASTEXITCODE" }
}
New-Item -ItemType Directory -Force $build,$dist | Out-Null
foreach ($variant in @('launcher')) {
    $src = Join-Path $root "src\$variant"
    $obj = Join-Path $build $variant
    $out = Join-Path $dist $variant
    New-Item -ItemType Directory -Force $obj,$out | Out-Null
    Push-Location $obj
    try {
        Run 'cl.exe' @('/nologo','/c','/O2','/MT','/TC',"/I$mh\include","/I$mh\src","$mh\src\buffer.c","$mh\src\hook.c","$mh\src\trampoline.c","$mh\src\hde\hde64.c")
        Run 'cl.exe' @('/nologo','/c','/O2','/MT','/W4','/EHsc','/Foproxy.obj',"$src\proxy.cpp")
        Run 'ml64.exe' @('/nologo','/c','/Foforwarders.obj',"$src\forwarders.asm")
        Run 'cl.exe' @('/nologo','/LD','/MT','proxy.obj','forwarders.obj','kernel32.lib','user32.lib','/link',"/DEF:$src\exports.def","/OUT:$out\version.dll",'/INCREMENTAL:NO')
        foreach ($pair in @(@('mount_plugin.cpp','StardiveMount.asi'),@('camera.cpp','MSDCamera.asi'))) {
            Run 'cl.exe' @('/nologo','/LD','/O2','/MT','/W4','/EHsc',"/I$mh\include","$src\$($pair[0])",'buffer.obj','hook.obj','trampoline.obj','hde64.obj','user32.lib','/link',"/OUT:$out\$($pair[1])",'/INCREMENTAL:NO')
        }
    } finally { Pop-Location }
}
$csc = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
if (-not (Test-Path $csc)) { throw 'The .NET Framework 4.x compiler is required.' }
$src = Join-Path $root 'src\launcher'
$native = Join-Path $dist 'launcher'
$exe = Join-Path $native 'MongilModLauncher.exe'
$arguments = @('/nologo','/target:winexe','/platform:x64',"/win32icon:$root\assets\launcher.ico","/win32manifest:$src\launcher.manifest","/out:$exe",'/reference:System.Windows.Forms.dll','/reference:System.Drawing.dll','/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll','/reference:System.IO.Compression.FileSystem.dll')
foreach ($name in @('WindowsBase','PresentationCore','PresentationFramework','WindowsFormsIntegration')) {
    $arguments += "/reference:$env:WINDIR\Microsoft.NET\Framework64\v4.0.30319\WPF\$name.dll"
}
$arguments += '/reference:System.Xaml.dll'
foreach ($name in @('version.dll','StardiveMount.asi','MSDCamera.asi')) { $arguments += "/resource:$native\$name,$name" }
$arguments += "/resource:$root\assets\background-edited.png,background.jpg"
$arguments += "/resource:$root\assets\gamebanana.png,gamebanana.png"
foreach ($name in @('Launcher.cs','LauncherSession.cs','InstallDetection.cs')) { $arguments += "$src\$name" }
Run $csc $arguments
foreach ($variant in @('launcher')) {
    $out = Join-Path $dist $variant
    Copy-Item "$mh\LICENSE.txt" "$out\MinHook-LICENSE.txt"
    Get-ChildItem $out -File | Where-Object { $_.Extension -in '.dll','.asi','.exe' } | ForEach-Object {
        $stream = [IO.File]::OpenRead($_.FullName)
        $hash = [Security.Cryptography.SHA256]::Create()
        try { $digest = [BitConverter]::ToString($hash.ComputeHash($stream)).Replace('-','').ToLowerInvariant() }
        finally { $hash.Dispose(); $stream.Dispose() }
        '{0}  {1}' -f $digest,$_.Name
    } | Set-Content "$out\SHA256SUMS.txt"
}
Write-Host "Build complete: $dist"

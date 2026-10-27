@echo off
setlocal
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%i"
if not defined VSROOT exit /b 1
call "%VSROOT%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
cd /d "%~dp0.."
if not exist build\tests mkdir build\tests
cd build\tests
set "MH=..\..\third_party\minhook"
cl /nologo /c /O2 /MT /TC /I"%MH%\include" /I"%MH%\src" "%MH%\src\buffer.c" "%MH%\src\hook.c" "%MH%\src\trampoline.c" "%MH%\src\hde\hde64.c"
if errorlevel 1 exit /b 1
cl /nologo /O2 /MT /EHsc /I"%MH%\include" ..\..\tests\compatibility.cpp buffer.obj hook.obj trampoline.obj hde64.obj /Fe:compatibility.exe
if errorlevel 1 exit /b 1
cl /nologo /O2 /MT /EHsc /DSTARDIVE_OBSERVER_TEST /I"%MH%\include" ..\..\src\launcher\mount_plugin.cpp buffer.obj hook.obj trampoline.obj hde64.obj user32.lib /Fe:observers.exe
if errorlevel 1 exit /b 1
cl /nologo /O2 /MT /EHsc /std:c++17 ..\..\tests\bk2.cpp /Fe:bk2.exe
if errorlevel 1 exit /b 1
"%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\csc.exe" /nologo /reference:System.Windows.Forms.dll /out:LauncherTests.exe ..\..\tests\LauncherTests.cs
if errorlevel 1 exit /b 1
exit /b 0

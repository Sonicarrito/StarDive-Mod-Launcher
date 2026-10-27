# Star Dive Mod Launcher

Source snapshot for version **1.8.3**, a compatibility update based on 1.8.2. 

The launcher installs the native loader, starts the official Netmarble launcher, and manages the launcher session. It loads compatible `_P.pak` mods and their associated `.utoc` and `.ucas` files from `BigCat/Binaries/Win64/Mods`, including subfolders.

## Build on Windows

1. Install Visual Studio 2022 or Visual Studio Build Tools with **Desktop development with C++**, the x64 MSVC tools and a Windows SDK. Enable .NET Framework 4.x if it is unavailable.
2. Download or clone this repository.
3. Run `build.cmd` from its root. Building does not require the game or administrator access.

The script locates Visual Studio automatically, compiles the bundled MinHook source and the native extensions, then builds the C# launcher. Outputs appear in `dist/launcher`. The launcher EXE embeds its native loader modules. All current runtime modules have source under `src/`; no current runtime module is downloaded during the build.

Earlier installations are recognized using SHA-256 hashes stored in the launcher source. Historical loader executables are not included. Compiler versions can affect output, so byte-for-byte reproduction across toolchains is not guaranteed.

## Use

For the launcher, run `dist/launcher/MongilModLauncher.exe` and select the game and official launcher paths if automatic detection does not find them. The EXE requests administrator access to install files in the game folder.

The native loader requires an active mod-launcher session.

The camera extension supports expanded pitch and closer zoom. Set `Enabled=0` in `MSDCamera.ini` to disable it.

## Supported game build

This snapshot targets the installed executable tested on October 26, 2026:

```text
SHA-256: dc52b31fe35f90439c5b1029b689d3cb4f090f3f941eeda62034c2db8bb5f253
```

Game updates can relocate internal functions. The launcher checks the executable hash; the native loader checks image metadata and function signatures; the camera checks the executable hash and input-function signature. Do not remove those guards to make an unsupported build run. Locate and verify its functions and object layouts before updating references.

The native mod loader waits for the game's content-mount completion before mounting mods. F8 is available as a fallback. Logs are written beside the installed modules: `stardive-loader-probe.log`, `stardive-mount.log`, and `MSDCamera.log`. A successful mount log still needs a visual in-game check to confirm that the mod's assets display correctly.

## How it works and antivirus reports

`version.dll` forwards Windows version-library exports to the real System32 library and starts the launcher extensions. `StardiveMount.asi` uses MinHook to observe the game's loading state, locates the verified platform-file object, and calls the game's mount function. `MSDCamera.asi` hooks the verified camera-input function. The launcher uses a local named-pipe session to authorize its own modules.

The launcher embeds and installs executable modules, requests elevation, and contains game hooks. These features can trigger antivirus detections. Publishing source is for inspection and independent builds; it is not proof that any particular binary or detection is safe. Check the exact binary's hash and report suspected false positives to the detecting antivirus vendor.

## Third-party material and licensing

MinHook and its bundled disassembler retain their upstream licenses under `third_party/minhook/`. See `THIRD_PARTY_NOTICES.md`. Game artwork, icons and third-party branding are build assets, and remain the property of their respective owners. No game executable, game assets extracted for mods, account data, or local launcher settings are included.

No open-source license has been selected for the original launcher and loader code in this snapshot. Public source availability does not itself grant redistribution or modification rights beyond applicable law and existing third-party licenses.

# Star Dive Mod Launcher

Version **1.10.0** adds update-tolerant function discovery and BK2 movie replacements.

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

## Game update compatibility

The launcher discovers the required game functions from instruction patterns and checks their related code and data. It no longer rejects a build simply because its executable hash or function addresses changed. Both the previous and October 26, 2026 game executables passed compatibility checks.

This improves tolerance of routine rebuilds, but cannot guarantee support for every future update. Changed function behavior, layouts or movie locations may still require a loader update. Ambiguous or missing essential matches stop mod loading. If only automatic content-completion detection is unavailable, F8 remains available after the game's content check. Camera code and its reflected field layouts are verified separately; an incompatible camera feature does not disable PAK mods.

The native mod loader waits for the game's content-mount completion before loading mods. Logs are written beside the installed modules: `stardive-loader-probe.log`, `stardive-mount.log`, `MSDCamera.log`, and `stardive-bk2.log`. The launcher's compatibility diagnostics are stored under `%LOCALAPPDATA%/MongilModLauncher/compatibility.log`. A successful mount log still needs a visual in-game check to confirm that the mod's assets display correctly.

## BK2 movie replacements

Place replacement `.bk2` files anywhere under `BigCat/Binaries/Win64/Mods`. Each filename must exactly match an existing movie in `BigCat/Saved/PersistentDownloadDir/M2/moviechunk800`. The Mod Manager can enable or disable these files. Duplicate filenames, disabled files, invalid Bink2 headers, missing targets and links are skipped.

Enabled replacements are copied into the downloaded movie cache after the content check. **These changes persist after the game exits. Disabling or deleting the mod prevents future copies but does not restore the original movie.** Use the official launcher's game repair to restore original files. This version includes no restore helper or automatic backups.

## Validation

Run `tests/build-tests.cmd` after building. It produces test executables under `build/tests`:

- `compatibility.exe <game-executable> [older-game-executable]` maps the supplied files without running game code, verifies discovery and hook creation, and checks rejection of missing, duplicate and changed signatures/layouts. Changes are confined to the test process.
- `observers.exe` checks automatic loading triggers and preservation of the game's original return behavior.
- `bk2.exe <new-disposable-directory>` exercises movie replacement, persistence, skip rules and locked-file handling using synthetic files.
- `LauncherTests.exe <built-launcher-exe> <new-disposable-directory> <game-executable> [older-game-executable]` checks the embedded native probe, installation and settings inside a disposable fixture.

The tests need local game executables supplied by the tester. No game binaries are included. Live gameplay and real movie playback must be checked separately before calling a candidate release fully verified.

## How it works and antivirus reports

`version.dll` forwards Windows version-library exports to the real System32 library and starts the launcher extensions. `StardiveMount.asi` uses MinHook to observe the game's loading state, locates the verified platform-file object, and calls the game's mount function. `MSDCamera.asi` hooks the verified camera-input function. The launcher uses a local named-pipe session to authorize its own modules.

The launcher embeds and installs executable modules, requests elevation, and contains game hooks. These features can trigger antivirus detections. Publishing source is for inspection and independent builds; it is not proof that any particular binary or detection is safe. Check the exact binary's hash and report suspected false positives to the detecting antivirus vendor.

## Third-party material and licensing

MinHook and its bundled disassembler retain their upstream licenses under `third_party/minhook/`. See `THIRD_PARTY_NOTICES.md`. Game artwork, icons and third-party branding are build assets, and remain the property of their respective owners. No game executable, game assets extracted for mods, account data, or local launcher settings are included.

No open-source license has been selected for the original launcher and loader code in this snapshot. Public source availability does not itself grant redistribution or modification rights beyond applicable law and existing third-party licenses.

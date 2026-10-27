#pragma once
// Instruction patterns verified against two successive game builds.
// Wildcards cover relative addresses; structure offsets remain checked.
namespace Compat {
static const char* MountPattern = "40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 58 FC FF FF 48 81 EC A8 04 00 00 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 85 90 03 00 00 48 8B 85 18 04 00 00 48 8B D9 48 89 4D 98 4D 8B E1 48 8B 49 08 45 8B E8 48 89 45 C8 4C 8B F2 44 89 44 24 38 C6 44 24 30 01 48 8B 01 FF 50 78 84 C0 0F 84 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? 48 85 C9 75 ?? E8 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? 48 8B 01";
static const char* PatchTickPattern = "40 53 55 41 57 48 83 EC 30 0F B6 91 90 03 00 00 48 8B D9 48 89 74 24 50 48 89 7C 24 58 4C 89 74 24 60";
static const char* StateSetterPattern = "48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 48 83 EC 20 48 8B D9 48 8B FA 48 8D 4C 24 30 FF 15 ?? ?? ?? ?? 0F B6 83 D0 03 00 00 0F 57 C0 F2 48 0F 2A 44 24 30 F2 0F 59 05 ?? ?? ?? ?? F2 0F 58 05 ?? ?? ?? ?? 3A 07 74 ?? F2 0F 11 83 A8 04 00 00 84 C0 75 ?? F2 0F 11 83 A0 04 00 00 0F B6 07 48 8D B3 90 03 00 00 48 8D 4E 08 88 06 48 8D 57 08 E8 ?? ?? ?? ?? 8B 47 18 89 46 18";
static const char* CameraInputPattern = "48 89 5C 24 08 57 48 83 EC 50 80 B9 7C 0A 00 00 00 48 8B D9 0F 29 74 24 40 0F 28 F3 0F B6 FA 75 ?? 80 B9 22 0E 00 00 00 74 ?? 8D 47 FD 3C 01 0F 86 ?? ?? ?? ?? 80 B9 7D 0A 00 00 00 74 ?? 40 80 FF 05 0F 84 ?? ?? ?? ?? 41 80 F8 01 74 ?? 8D 47 FD 3C 01 77 ?? 33 D2 E8 ?? ?? ?? ?? 8B 83 38 0C 00 00 89 83 D0 0D 00 00 33 C0 48 89 83 F8 0D 00 00 8B CF 83 E9 03 0F 84 ?? ?? ?? ?? 83 E9 01 0F 84 ?? ?? ?? ?? 83 F9 01 0F 85 ?? ?? ?? ?? 80 BB B5 0B 00 00 00 0F 84 ?? ?? ?? ?? 48 83 BB 48 0D 00 00 00 0F 84 ?? ?? ?? ??";
static const char* PlatformAnchorPattern = "48 8B 3D ?? ?? ?? ?? 48 85 FF 0F 84 ?? ?? ?? ?? 0F 1F 00 48 8B 07 48 8B CF FF 50 70 4C 8B D0 48 8D 05 ?? ?? ?? ??";
static const char* MountCallerPattern = "48 C7 44 24 28 00 00 00 00 C6 44 24 20 01 4D 8B CC 41 B8 10 27 00 00 48 8B CF E8 ?? ?? ?? ?? 84 C0";
}

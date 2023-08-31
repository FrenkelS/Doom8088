## Doom8088
Doom8088 is a port of Doom for PCs with a 16-bit processor like an 8088 or a 286.
It's based on [GBADoom](https://github.com/doomhack/GBADoom).
Download Doom8088 [here](https://github.com/FrenkelS/Doom8088/releases).

**What's special?:**
 - No sound and music
 - No saving and loading
 - No multiplayer
 - No PWADs
 - Supports only Doom 1 Episode 1
 - Only demo3 is in sync
 - Lots of crashes due to memory issues
 
## Controls:
|Action      |GBA   |DOS                     |
|------------|------|------------------------|
|Fire        |B     |Ctrl                    |
|Use / Sprint|A     |Enter, Space & Shift    |
|Walk        |D-Pad |Arrow keys              |
|Strafe      |L & R |< & >                   |
|Automap     |SELECT|Tab                     |
|Weapon up   |A + R |Enter, Space & Shift + >|
|Weapon down |A + L |Enter, Space & Shift + <|
|Menu        |Start |Esc                     |
|Quit to DOS |      |F10                     |

## Building:
1) Install [Watcom](https://github.com/open-watcom/open-watcom-v2)

2) Run `setenvwc.bat` and then `compwc16.bat`

3) Compress `DOOM8088.EXE` with [LZEXE](https://bellard.org/lzexe.html), just like all the other 16-bit id Software games.

4) Doom8088 needs an IWAD file that has been processed by [GbaWadUtil](https://github.com/doomhack/GbaWadUtil).
   Some lumps in the WAD need to be replaced by the raw pictures from the WAD directory of this repository.

It's possible to build a 32-bit version of Doom8088 with Watcom and [DJGPP](https://github.com/andrewwutw/build-djgpp). For debugging purposes, the Zone memory can be increased significantly this way.

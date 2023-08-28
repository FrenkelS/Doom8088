## Doom8088
Doom8088 is a port of Doom for PCs with a 16-bit processor like an 8088 or a 286.
It's based on [GBADoom](https://github.com/doomhack/GBADoom).

**What's special?:**
 - No sound and music
 - No saving and loading
 - No multiplayer
 - No PWADs
 - Demos don't sync
 - Supports only Doom 1 Episode 1
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

3) Doom8088 needs an IWAD file that has been processed by [GbaWadUtil](https://github.com/doomhack/GbaWadUtil).
   Some lumps in the WAD need to be replaced by the raw pictures from the WAD directory of this repository.

It's possible to build a 32-bit version of Doom8088 with Watcom and [DJGPP](https://github.com/andrewwutw/build-djgpp). For debugging purposes, the Zone memory can be increased significantly this way.

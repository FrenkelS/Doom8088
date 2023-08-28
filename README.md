## Doom8088
Doom8088 is based on [GBADoom](https://github.com/doomhack/GBADoom).
It needs a WAD file that has been processed by [GbaWadUtil](https://github.com/doomhack/GbaWadUtil), and the title picture and status bar should be replaced by raw files.

**What's different?:**
 - No sound and music
 - No saving and loading
 - No multiplayer
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

## Developers:
For debugging purposes, it's possible to build a 32-bit version with Watcom and [DJGPP](https://github.com/andrewwutw/build-djgpp). For example, the Zone memory can be increased significantly.

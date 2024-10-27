## Doom8088
![Doom8088](readme_imgs/doom8088.png?raw=true)

Doom was originally designed in 1993 for 32-bit DOS computers with 4 MB of RAM.
It's mostly written in C code with very little assembly code.
It has been ported to all kinds of systems.
Usually these systems are 32-bit or more and have a flat memory model.

Doom8088 is a port for PCs with a 16-bit processor like an 8088 or a 286, and with VGA or MCGA graphics.
It uses 64 kB of EMS memory, if available.
And 1506 kB of XMS memory, if available.
It's based on [GBADoom](https://github.com/doomhack/GBADoom).
Download Doom8088 [here](https://github.com/FrenkelS/Doom8088/releases).

Watch what it looks like on a real PC [here](https://www.youtube.com/watch?v=oAX1-lNuUBY).

**What's special?:**
 - Supports only Doom 1 Episode 1
 - Rotating overlaid automap
 - Only demo3 is supported
 - 256 and 16 color modes
 - PC speaker sound effects
 - Mouse support
 - No music
 - No texture mapped floors and ceilings
 - No light diminishing
 - No saving and loading
 - No multiplayer
 - No PWADs
 - No screen resizing
 - No joystick support

**Known bugs:**
 - When there's not enough memory for a texture, one color is drawn
 - When there's not enough memory for the intermission screen, the last few frames of gameplay are shown instead
 - When there's not enough memory for a new palette, only the border changes color

## Supported video modes

### VGA 320x200 256 color Mode Y and MCGA 320x200 256 color Mode 13H
![Doom8088 in 256 colors](readme_imgs/doom8088.png?raw=true)

### Text mode 80x25 16 color mode
![Doom8088 in 16 colors](readme_imgs/doomt80.png?raw=true)

### Text mode 40x25 16 color mode
![Doom8088 in 16 colors](readme_imgs/doomt40.png?raw=true)

## Controls:
|Action                 |                       |
|-----------------------|-----------------------|
|Fire                   |Ctrl & any mouse button|
|Use                    |Enter & Space & E      |
|Sprint                 |Shift                  |
|Walk                   |Arrow keys, W & S      |
|Strafe                 |Alt                    |
|Strafe left and right  |< & >, A & D           |
|Automap                |Tab                    |
|Automap zoom in and out|+ & -                  |
|Automap follow mode    |F                      |
|Weapon up and down     |[ & ]                  |
|Menu                   |Esc                    |
|Quit to DOS            |F10                    |

## Cheats:
|Code      |Effects                  |
|----------|-------------------------|
|IDCHOPPERS|Chainsaw                 |
|IDDQD     |God mode                 |
|IDKFA     |Weapons & Keys           |
|IDFA      |Weapons                  |
|IDSPISPOPD|No Clipping              |
|IDBEHOLDV |Invincibility            |
|IDBEHOLDS |Berserk                  |
|IDBEHOLDI |Invisibility             |
|IDBEHOLDR |Radiation shielding suit |
|IDBEHOLDA |Auto-map                 |
|IDBEHOLDL |Lite-Amp Goggles         |
|IDCLEV    |Exit Level               |
|IDROCKET  |Enemy Rockets (GoldenEye)|
|IDRATE    |Toggle FPS counter       |

## Command line arguments:
|Command line argument|Effect       |
|---------------------|-------------|
|-noems               |Disable EMS  |
|-noxms               |Disable XMS  |
|-timedemo demo3      |Run benchmark|

## Building:
1) Install [gcc-ia16](https://launchpad.net/%7Etkchia/+archive/ubuntu/build-ia16) (including [libi86](https://gitlab.com/tkchia/libi86)) and [NASM](https://www.nasm.us) on Ubuntu.

2) Run `compiamy.sh` to compile the Mode Y version, `compia13.sh` for the Mode 13h version, `compiat1.sh` for text mode 40x25 and `compiat3.sh` for text mode 80x25.

3) (Optional) Compress `DOOM8088.EXE` with [LZEXE](https://bellard.org/lzexe.html), just like all the other 16-bit id Software games.

4) Doom8088 needs an IWAD file that has been preprocessed by [jWadUtil](https://github.com/FrenkelS/jWadUtil).

It's possible to build a 32-bit version of Doom8088 with [DJGPP](https://github.com/andrewwutw/build-djgpp) and [Watcom](https://github.com/open-watcom/open-watcom-v2).
First run `setenvdj.bat` once and then `compdj.bat` for DJGPP, and `setenvwc.bat` followed by `compwc32.bat` for Watcom.
For debugging purposes, the Zone memory can be increased significantly this way.

It's also possible to build a 16-bit version with Watcom: Run `setenvwc.bat` followed by `compwc16.bat`.

## Doom8088
![Doom8088](readme_imgs/doom8088.png?raw=true)

Doom was originally designed in 1993 for 32-bit DOS computers with 4 MB of RAM.
It's mostly written in C code with very little assembly code.
It has been ported to all kinds of systems.
Usually these systems are 32-bit or more and have a flat memory model.

Doom8088 is a port for PCs with a 16-bit processor like an 8088 or a 286, and with VGA or MCGA graphics.
It uses 64 kB of EMS memory, if available.
And 2024 kB of XMS memory, if available.
It's based on [GBADoom](https://github.com/doomhack/GBADoom).
Download Doom8088 [here](https://github.com/FrenkelS/Doom8088/releases).

Watch what it looks like on a real PC [here](https://www.youtube.com/watch?v=oAX1-lNuUBY).

**What's special?:**
 - Supports only Doom 1 Episode 1
 - Rotating overlaid automap
 - Only demo3 is supported
 - PC speaker sound effects
 - No music
 - No texture mapped floors and ceilings
 - No light diminishing
 - No saving and loading
 - No multiplayer
 - No PWADs
 - No screen resizing
 - No mouse and joystick support

**Known bugs:**
 - When there's not enough memory for a texture, one color is drawn
 - When there's not enough memory for the intermission screen, the last few frames of gameplay are shown instead
 - When there's not enough memory for a new palette, only the border changes color

## Controls:
|Action                 |             |
|-----------------------|-------------|
|Fire                   |Ctrl         |
|Use                    |Enter & Space|
|Sprint                 |Shift        |
|Walk                   |Arrow keys   |
|Strafe                 |Alt          |
|Strafe left and right  |< & >        |
|Automap                |Tab          |
|Automap zoom in and out|+ & -        |
|Automap follow mode    |F            |
|Weapon up and down     |[ & ]        |
|Menu                   |Esc          |
|Quit to DOS            |F10          |

## Cheats:
|Code      |Effects                  |
|----------|-------------------------|
|IDCHOPPERS|Chainsaw                 |
|IDDQD     |God mode                 |
|IDKFA     |Ammo & Keys              |
|IDKA      |Ammo                     |
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

2) Run `compiamy.sh` to compile the Mode Y version and `compia13.sh` for the Mode 13h version.

3) (Optional) Compress `DOOM8088.EXE` with [LZEXE](https://bellard.org/lzexe.html), just like all the other 16-bit id Software games.

4) Doom8088 needs an IWAD file that has been preprocessed by [jWadUtil](https://github.com/FrenkelS/jWadUtil).

It's possible to build a 32-bit version of Doom8088 with [DJGPP](https://github.com/andrewwutw/build-djgpp) and [Watcom](https://github.com/open-watcom/open-watcom-v2).
First run `setenvdj.bat` once and then `compdj.bat` for DJGPP, and `setenvwc.bat` followed by `compwc32.bat` for Watcom.
For debugging purposes, the Zone memory can be increased significantly this way.

It's also possible to build a 16-bit version with Watcom: Run `setenvwc.bat` followed by `compwc16.bat`.

## Doom8088
![Doom8088](readme_imgs/doomvgah.png?raw=true)

Doom was originally designed in 1993 for 32-bit DOS computers with 4 MB of RAM.
It's mostly written in C code with very little assembly code.
It has been ported to all kinds of systems.
Usually these systems are 32-bit or more and have a flat memory model.

Doom8088 is a port for PCs with a 16-bit processor like an 8088 or a 286, and with MDA, CGA, EGA, VGA or MCGA graphics.
It uses 64 kB of EMS memory, if available, as an upper memory block to store data.
And 1507 kB of XMS memory, if available, as a RAM drive.
It's based on [GBADoom](https://github.com/doomhack/GBADoom), a port of PrBoom to the Game Boy Advance.
Download Doom8088 [here](https://github.com/FrenkelS/Doom8088/releases).

Watch what it looks like on a real PC [here](https://www.youtube.com/watch?v=oAX1-lNuUBY).

**What's special?:**
 - Supports only Doom 1 Episode 1
 - Rotating overlaid automap
 - Only demo3 is supported
 - 256, 16, 4 and 2 color modes
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

## Supported video modes

### VGA 320x200 256 color Mode Y and MCGA 320x200 256 color Mode 13h, effective resolution 240x128
![Doom8088 in 256 colors](readme_imgs/doomvgah.png?raw=true)

### VGA 320x200 256 color Mode Y and MCGA 320x200 256 color Mode 13h, effective resolution 120x128
![Doom8088 in 256 colors](readme_imgs/doomvgam.png?raw=true)

### VGA 320x200 256 color Mode Y and MCGA 320x200 256 color Mode 13h, effective resolution  60x128
![Doom8088 in 256 colors](readme_imgs/doomvgal.png?raw=true)

### EGA 640x200 16 color mode
![Doom8088 in 16 colors](readme_imgs/doomegah.png?raw=true)

### EGA 320x200 16 color mode
![Doom8088 in 16 colors](readme_imgs/doomegam.png?raw=true)

### CGA 320x200 4 color mode
![Doom8088 in 4 colors](readme_imgs/doomcga.png?raw=true)

### CGA 640x200 2 color mode
![Doom8088 in 2 colors](readme_imgs/doomcgabw.png?raw=true)

### Text mode 80x50 16 color mode
![Doom8088 in 16 colors](readme_imgs/doomt80x50.png?raw=true)

### Text mode 80x43 16 color mode
![Doom8088 in 16 colors](readme_imgs/doomt80x43.png?raw=true)

### Text mode 80x25 16 color mode
![Doom8088 in 16 colors](readme_imgs/doomt80x25.png?raw=true)

### Text mode 40x25 16 color mode
![Doom8088 in 16 colors](readme_imgs/doomt40x25.png?raw=true)

### Text mode 80x25 2 color mode
![Doom8088 in 2 colors](readme_imgs/doomt80x25m.png?raw=true)

## Controls:
|Action                           |Keys         |
|---------------------------------|-------------|
|Fire                             |Ctrl         |
|Use                              |Enter & Space|
|Sprint                           |Shift        |
|Walk                             |Arrow keys   |
|Strafe                           |Alt          |
|Strafe left and right            |< & >        |
|Automap                          |Tab          |
|Automap zoom in and out          |+ & -        |
|Automap follow mode              |F            |
|Weapon up and down               |[ & ]        |
|Menu                             |Esc          |
|Switch palette (CGA version only)|F5           |
|Quit to DOS                      |F10          |

## Cheats:
|Code      |Effects                  |Notes                           |
|----------|-------------------------|--------------------------------|
|IDCHOPPERS|Chainsaw                 |                                |
|IDDQD     |God mode                 |                                |
|IDKFA     |Weapons & Keys           |                                |
|IDFA      |Weapons                  |                                |
|IDSPISPOPD|No Clipping              |                                |
|IDBEHOLDV |Invincibility            |                                |
|IDBEHOLDS |Berserk                  |                                |
|IDBEHOLDI |Invisibility             |                                |
|IDBEHOLDR |Radiation shielding suit |                                |
|IDBEHOLDA |Auto-map                 |                                |
|IDBEHOLDL |Lite-Amp Goggles         |                                |
|IDCLEV    |Exit Level               |                                |
|IDEND     |Show end text            |                                |
|IDROCKET  |Enemy Rockets            |(GoldenEye)                     |
|IDRATE    |Toggle FPS counter       |Divide by 10 to get the real FPS|

## Command line arguments:
|Command line argument|Effect               |
|---------------------|---------------------|
|-noems               |Disable EMS          |
|-noxms               |Disable XMS          |
|-nosfx               |Disable sound effects|
|-nosound             |Disable sound effects|
|-timedemo demo3      |Run benchmark        |

## Building:
1) Install [gcc-ia16](https://launchpad.net/%7Etkchia/+archive/ubuntu/build-ia16) (including [libi86](https://gitlab.com/tkchia/libi86)) and [NASM](https://www.nasm.us) on Ubuntu.

2) Run one of the build scripts:

|Build script |Version                  |
|-------------|-------------------------|
|`buildall.sh`|Build all versions       |
|`bmodeyh.sh` |Mode Y 240x128           |
|`bmodeym.sh` |Mode Y 120x128           |
|`bmodeyl.sh` |Mode Y  60x128           |
|`bmode13h.sh`|Mode 13h 240x128         |
|`bmode13m.sh`|Mode 13h 120x128         |
|`bmode13l.sh`|Mode 13h  60x128         |
|`begah.sh`   |EGA 640x200 16 colors    |
|`begam.sh`   |EGA 320x200 16 colors    |
|`bcga.sh`    |CGA 320x200  4 colors    |
|`bcgabw.sh`  |CGA 640x200  2 colors    |
|`bt80x50.sh` |Text mode 80x50 16 colors|
|`bt80x43.sh` |Text mode 80x43 16 colors|
|`bt80x25.sh` |Text mode 80x25 16 colors|
|`bt40x25.sh` |Text mode 40x25 16 colors|
|`bmda.sh`    |Text mode 80x25  2 colors|

It's possible to build a 32-bit version of Doom8088 with [DJGPP](https://github.com/andrewwutw/build-djgpp) and [Watcom](https://github.com/open-watcom/open-watcom-v2).
First run `setenvdj.bat` once and then `bdj32.bat` for DJGPP, and `setenvwc.bat` followed by `bwc32.bat` for Watcom.
For debugging purposes, the Zone memory can be increased significantly this way.

It's also possible to build a 16-bit version with Watcom: Run `setenvwc.bat` followed by `bwc16.bat`.


3) (Optional) Compress `DOOM8088.EXE` with [LZEXE](https://bellard.org/lzexe), just like all the other 16-bit id Software games.

4) Doom8088 needs an IWAD file that has been preprocessed by [jWadUtil](https://github.com/FrenkelS/jWadUtil).

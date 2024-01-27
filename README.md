## Doom8088
![Doom8088](readme_imgs/doom8088.png?raw=true)

Doom was originally designed in 1993 for 32-bit DOS computers with 4 MB of RAM.
It's mostly written in C code with very little assembly code.
It has been ported to all kinds of systems.
Usually these systems are 32-bit or more and have a flat memory model.

Doom8088 is a port for PCs with a 16-bit processor like an 8088 or a 286, and with VGA or MCGA graphics. It uses 64 kB of EMS memory, if available.
It's based on [GBADoom](https://github.com/doomhack/GBADoom).
Download Doom8088 [here](https://github.com/FrenkelS/Doom8088/releases).

Watch what it looks like on a real PC [here](https://www.youtube.com/watch?v=qoGhgmhArKw).

**What's special?:**
 - No texture mapped floors and ceilings
 - No light diminishing
 - No music
 - No saving and loading
 - No multiplayer
 - No PWADs
 - No screen resizing
 - No mouse and joystick support
 - PC speaker sound effects
 - Rotating overlaid automap
 - Supports only Doom 1 Episode 1
 - Only demo3 is in sync

**Known bugs:**
 - Some crashes and lockups due to memory issues and divisions by zero
 - When there's not enough memory for a texture, one color is drawn

## Controls:
|Action                 |                        |
|-----------------------|------------------------|
|Fire                   |Ctrl                    |
|Use / Sprint           |Enter, Space & Shift    |
|Walk                   |Arrow keys              |
|Strafe                 |< & >                   |
|Automap                |Tab                     |
|Automap zoom in and out|+ & -                   |
|Automap follow mode    |F                       |
|Weapon up and down     |[ & ]                   |
|Menu                   |Esc                     |
|Quit to DOS            |F10                     |

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

## Building:
1) Install [gcc-ia16](https://launchpad.net/%7Etkchia/+archive/ubuntu/build-ia16) and [NASM](https://www.nasm.us) on Ubuntu.

2) Run `compiamy.sh` to compile the Mode Y version and `compia13.sh` for the Mode 13h version.

3) (Optional) Compress `DOOM8088.EXE` with [LZEXE](https://bellard.org/lzexe.html), just like all the other 16-bit id Software games.

4) Doom8088 needs an IWAD file that has been preprocessed by [GbaWadUtil](https://github.com/doomhack/GbaWadUtil).
   Some lumps in the WAD need to be replaced by the raw images from the WAD directory in this repository.
   In addition, some manual actions are required. It is best to just use the IWAD of the latest [release](https://github.com/FrenkelS/Doom8088/releases).

It's possible to build a 32-bit version of Doom8088 with [DJGPP](https://github.com/andrewwutw/build-djgpp) and [Watcom](https://github.com/open-watcom/open-watcom-v2).
First run `setenvdj.bat` once and then `compdj.bat` for DJGPP, and `setenvwc.bat` followed by `compwc32.bat` for Watcom.
For debugging purposes, the Zone memory can be increased significantly this way.

It's also possible to build a 16-bit version with Watcom: Run `setenvwc.bat` followed by `compwc16.bat`.

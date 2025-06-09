Doom8088
Doom for 16-bit DOS computers
https://github.com/FrenkelS/Doom8088

Doom was originally designed in 1993 for 32-bit DOS computers with 4 MB of RAM.
It's mostly written in C code with very little assembly code.
It has been ported to all kinds of systems.
Usually these systems are 32-bit or more and have a flat memory model.

Doom8088 is a port for PCs with a 16-bit processor like an 8088 or a 286,
and with MDA, CGA, EGA, VGA or MCGA graphics.
If available it uses 64 kB of EMS memory as an upper memory block to store data.
And if available it uses 1507 kB of XMS memory as a RAM drive.
It's based on doomhack's GBADoom, a port of PrBoom to the Game Boy Advance.

What's special?
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

Known bugs:
 - When there's not enough memory for a texture, one color is drawn
 - When there's not enough memory for the intermission screen,
   the last few frames of gameplay are shown instead
 - When there's not enough memory for a new palette,
   only the border changes color

Supported video modes:
|Video mode                             |Executable 8088|Executable 286|Notes |
|---------------------------------------|---------------|--------------|------|
|VGA  320x200 256 color Mode Y   240x128|D8088MYH.EXE   |D286MYH.EXE   |*1, *2|
|VGA  320x200 256 color Mode Y   120x128|D8088MYM.EXE   |D286MYM.EXE   |*1    |
|VGA  320x200 256 color Mode Y    60x128|D8088MYL.EXE   |D286MYL.EXE   |*1    |
|MCGA 320x200 256 color Mode 13h 240x128|D8M13H.EXE     |D2M13H.EXE    |*3    |
|MCGA 320x200 256 color Mode 13h 120x128|D8M13M.EXE     |D2M13M.EXE    |*3    |
|MCGA 320x200 256 color Mode 13h  60x128|D8M13L.EXE     |D2M13L.EXE    |*3    |
|EGA  640x200  16 color mode            |D8088EH.EXE    |D286EH.EXE    |*4    |
|EGA  320x200  16 color mode            |D8088EM.EXE    |D286EM.EXE    |*4    |
|CGA  320x200   4 color mode            |D8088CGA.EXE   |D286CGA.EXE   |*5    |
|CGA  640x200   2 color mode            |D8088CBW.EXE   |D286CBW.EXE   |*5    |
|Text mode 80x50 16 color mode          |D8T80X50.EXE   |D2T80X50.EXE  |*6    |
|Text mode 80x43 16 color mode          |D8T80X43.EXE   |D2T80X43.EXE  |*7    |
|Text mode 80x25 16 color mode          |D8T80X25.EXE   |D2T80X25.EXE  |      |
|Text mode 40x25 16 color mode          |D8T40X25.EXE   |D2T40X25.EXE  |      |
|Text mode 80x25  2 color mode          |D8088MDA.EXE   |D286MDA.EXE   |*8    |

*1 VGA Mode Y should be faster than MCGA Mode 13h.
   Benchmark it with -timedemo demo3 or IDRATE to be sure.
*2 D386MYH.EXE is a 386 version of the VGA Mode Y 240x128 version.
   It requires CWSDPR0.EXE to be in the same directory.
*3 The MCGA version also runs on VGA.
*4 The EGA version also runs on VGA.
*5 The CGA versions also runs on VGA, MCGA, EGA and Tandy 1000.
   In the 4 color mode version, press F5 to switch to a different palette.
*6 Text mode 80x50 requires VGA.
*7 Text mode 80x43 requires EGA or VGA.
*8 The MDA version also runs on VGA, MCGA, EGA, Tandy 1000 and CGA.

Controls:
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

Cheats:
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

Command line arguments:
|Command line argument|Effect               |
|---------------------|---------------------|
|-noems               |Disable EMS          |
|-noxms               |Disable XMS          |
|-nosfx               |Disable sound effects|
|-nosound             |Disable sound effects|
|-timedemo demo3      |Run benchmark        |

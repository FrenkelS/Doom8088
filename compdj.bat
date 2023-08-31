if "%DJDIR%" == "" goto error

mkdir DJ

set CFLAGS=-march=i386
@rem set CFLAGS=%CFLAGS% -g
@rem set CFLAGS=%CFLAGS% -Ofast -flto -fwhole-program -fomit-frame-pointer -funroll-loops -fgcse-sm -fgcse-las -fipa-pta -mpreferred-stack-boundary=2 -Wno-attributes -Wpedantic
@rem set CFLAGS=%CFLAGS% -Wall -Wextra
@rem set CFLAGS=%CFLAGS% -ffunction-sections -Wl,--gc-sections -Wl,--print-gc-sections

@set GLOBOBJS=
@set GLOBOBJS=%GLOBOBJS% am_map.c
@set GLOBOBJS=%GLOBOBJS% d_client.c
@set GLOBOBJS=%GLOBOBJS% d_items.c
@set GLOBOBJS=%GLOBOBJS% d_main.c
@set GLOBOBJS=%GLOBOBJS% f_finale.c
@set GLOBOBJS=%GLOBOBJS% f_wipe.c
@set GLOBOBJS=%GLOBOBJS% g_game.c
@set GLOBOBJS=%GLOBOBJS% globdata.c
@set GLOBOBJS=%GLOBOBJS% hu_stuff.c
@set GLOBOBJS=%GLOBOBJS% i_audio.c
@set GLOBOBJS=%GLOBOBJS% i_main.c
@set GLOBOBJS=%GLOBOBJS% i_system.c
@set GLOBOBJS=%GLOBOBJS% info.c
@set GLOBOBJS=%GLOBOBJS% m_cheat.c
@set GLOBOBJS=%GLOBOBJS% m_menu.c
@set GLOBOBJS=%GLOBOBJS% m_random.c
@rem set GLOBOBJS=%GLOBOBJS% m_recip.c
@set GLOBOBJS=%GLOBOBJS% p_ceilng.c
@set GLOBOBJS=%GLOBOBJS% p_doors.c
@set GLOBOBJS=%GLOBOBJS% p_enemy.c
@set GLOBOBJS=%GLOBOBJS% p_floor.c
@set GLOBOBJS=%GLOBOBJS% p_genlin.c
@set GLOBOBJS=%GLOBOBJS% p_inter.c
@set GLOBOBJS=%GLOBOBJS% p_lights.c
@set GLOBOBJS=%GLOBOBJS% p_map.c
@set GLOBOBJS=%GLOBOBJS% p_maputl.c
@set GLOBOBJS=%GLOBOBJS% p_mobj.c
@set GLOBOBJS=%GLOBOBJS% p_plats.c
@set GLOBOBJS=%GLOBOBJS% p_pspr.c
@set GLOBOBJS=%GLOBOBJS% p_setup.c
@set GLOBOBJS=%GLOBOBJS% p_sight.c
@set GLOBOBJS=%GLOBOBJS% p_spec.c
@set GLOBOBJS=%GLOBOBJS% p_switch.c
@set GLOBOBJS=%GLOBOBJS% p_telept.c
@set GLOBOBJS=%GLOBOBJS% p_tick.c
@set GLOBOBJS=%GLOBOBJS% p_user.c
@set GLOBOBJS=%GLOBOBJS% r_data.c
@set GLOBOBJS=%GLOBOBJS% r_draw.c
@set GLOBOBJS=%GLOBOBJS% r_main.c
@set GLOBOBJS=%GLOBOBJS% r_things.c
@set GLOBOBJS=%GLOBOBJS% s_sound.c
@set GLOBOBJS=%GLOBOBJS% sounds.c
@set GLOBOBJS=%GLOBOBJS% st_stuff.c
@set GLOBOBJS=%GLOBOBJS% tables.c
@set GLOBOBJS=%GLOBOBJS% v_video.c
@set GLOBOBJS=%GLOBOBJS% w_wad.c
@set GLOBOBJS=%GLOBOBJS% wi_stuff.c
@set GLOBOBJS=%GLOBOBJS% z_bmallo.c
@set GLOBOBJS=%GLOBOBJS% z_zone.c

gcc %GLOBOBJS% %CFLAGS% -o DJ/DOOM32DJ.EXE
strip -s DJ/DOOM32DJ.EXE
stubedit DJ/DOOM32DJ.EXE dpmi=CWSDPR0.EXE

goto end

:error
@echo Set the environment variables before running this script!

:end

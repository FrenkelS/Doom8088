mkdir GCCIA16 

unset CFLAGS

ia16-elf-gcc -c p_mobj.c -march=i8088 -mcmodel=medium -li86 -O0 -fomit-frame-pointer -funroll-loops -fgcse-sm -fgcse-las -fipa-pta -Wno-attributes -Wpedantic
ia16-elf-gcc -c p_spec.c -march=i8088 -mcmodel=medium -li86 -O0 -fomit-frame-pointer -funroll-loops -fgcse-sm -fgcse-las -fipa-pta -Wno-attributes -Wpedantic

export CFLAGS="-march=i8088 -mcmodel=medium -li86"
#export CFLAGS="$CFLAGS -g"
export CFLAGS="$CFLAGS -Ofast -fomit-frame-pointer -funroll-loops -fgcse-sm -fgcse-las -fipa-pta -Wno-attributes -Wpedantic"
#export CFLAGS="$CFLAGS -Ofast -flto -fwhole-program -fomit-frame-pointer -funroll-loops -fgcse-sm -fgcse-las -fipa-pta -Wno-attributes -Wpedantic"
#export CFLAGS="$CFLAGS -Wall -Wextra"
#export CFLAGS="$CFLAGS -ffunction-sections -Wl,--gc-sections -Wl,--print-gc-sections"

unset GLOBOBJS
export GLOBOBJS="$GLOBOBJS am_map.c"
export GLOBOBJS="$GLOBOBJS d_items.c"
export GLOBOBJS="$GLOBOBJS d_main.c"
export GLOBOBJS="$GLOBOBJS f_finale.c"
export GLOBOBJS="$GLOBOBJS f_wipe.c"
export GLOBOBJS="$GLOBOBJS g_game.c"
export GLOBOBJS="$GLOBOBJS hu_stuff.c"
export GLOBOBJS="$GLOBOBJS i_audio.c"
export GLOBOBJS="$GLOBOBJS i_main.c"
export GLOBOBJS="$GLOBOBJS i_system.c"
export GLOBOBJS="$GLOBOBJS info.c"
export GLOBOBJS="$GLOBOBJS m_cheat.c"
export GLOBOBJS="$GLOBOBJS m_menu.c"
export GLOBOBJS="$GLOBOBJS m_random.c"
export GLOBOBJS="$GLOBOBJS p_ceilng.c"
export GLOBOBJS="$GLOBOBJS p_doors.c"
export GLOBOBJS="$GLOBOBJS p_enemy.c"
export GLOBOBJS="$GLOBOBJS p_floor.c"
export GLOBOBJS="$GLOBOBJS p_genlin.c"
export GLOBOBJS="$GLOBOBJS p_inter.c"
export GLOBOBJS="$GLOBOBJS p_lights.c"
export GLOBOBJS="$GLOBOBJS p_map.c"
export GLOBOBJS="$GLOBOBJS p_maputl.c"
#export GLOBOBJS="$GLOBOBJS p_mobj.c"
export GLOBOBJS="$GLOBOBJS p_mobj.o"
export GLOBOBJS="$GLOBOBJS p_plats.c"
export GLOBOBJS="$GLOBOBJS p_pspr.c"
export GLOBOBJS="$GLOBOBJS p_setup.c"
export GLOBOBJS="$GLOBOBJS p_sight.c"
#export GLOBOBJS="$GLOBOBJS p_spec.c"
export GLOBOBJS="$GLOBOBJS p_spec.o"
export GLOBOBJS="$GLOBOBJS p_switch.c"
export GLOBOBJS="$GLOBOBJS p_telept.c"
export GLOBOBJS="$GLOBOBJS p_tick.c"
export GLOBOBJS="$GLOBOBJS p_user.c"
export GLOBOBJS="$GLOBOBJS r_data.c"
export GLOBOBJS="$GLOBOBJS r_draw.c"
export GLOBOBJS="$GLOBOBJS r_plane.c"
export GLOBOBJS="$GLOBOBJS r_sky.c"
export GLOBOBJS="$GLOBOBJS r_things.c"
export GLOBOBJS="$GLOBOBJS s_sound.c"
export GLOBOBJS="$GLOBOBJS sounds.c"
export GLOBOBJS="$GLOBOBJS st_stuff.c"
export GLOBOBJS="$GLOBOBJS tables.c"
export GLOBOBJS="$GLOBOBJS v_video.c"
export GLOBOBJS="$GLOBOBJS w_wad.c"
export GLOBOBJS="$GLOBOBJS wi_stuff.c"
export GLOBOBJS="$GLOBOBJS z_bmallo.c"
export GLOBOBJS="$GLOBOBJS z_zone.c"

ia16-elf-gcc $GLOBOBJS $CFLAGS -o GCCIA16/DOOMIA16.EXE

rm p_mobj.o
rm p_spec.o

mkdir GCCIA16

unset CFLAGS

ia16-elf-gcc -c i_main.c   -march=i286 -mcmodel=medium -Os    -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -fno-function-sections -mregparmcall
ia16-elf-gcc -c i_system.c -march=i286 -mcmodel=medium -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -fno-function-sections -mregparmcall -flto -fwhole-program -funroll-loops
ia16-elf-gcc -c p_map.c    -march=i286 -mcmodel=medium -Os    -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -fno-function-sections -mregparmcall
ia16-elf-gcc -c p_mobj.c   -march=i286 -mcmodel=medium -Og    -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -fno-function-sections -mregparmcall -flto -fwhole-program
ia16-elf-gcc -c p_spec.c   -march=i286 -mcmodel=medium -Og    -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -fno-function-sections -mregparmcall -flto -fwhole-program
ia16-elf-gcc -c p_switch.c -march=i286 -mcmodel=medium -Os    -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -fno-function-sections -mregparmcall
ia16-elf-gcc -c r_draw.c   -march=i286 -mcmodel=medium -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -fno-function-sections -mregparmcall -flto -fwhole-program -funroll-loops
ia16-elf-gcc -c z_bmallo.c -march=i286 -mcmodel=medium -Os    -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -fno-function-sections -mregparmcall
ia16-elf-gcc -c z_zone.c   -march=i286 -mcmodel=medium -Os    -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -fno-function-sections -mregparmcall

export CFLAGS="-march=i286 -mcmodel=medium -li86 -Os -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -fno-function-sections -mregparmcall -flto -fwhole-program"
#export CFLAGS="$CFLAGS -Ofast -flto -fwhole-program -fomit-frame-pointer -funroll-loops -fgcse-sm -fgcse-las -fipa-pta -Wno-attributes -Wpedantic"
#export CFLAGS="$CFLAGS -Wall -Wextra"

export GLOBOBJS="  a_taskmn.c"
export GLOBOBJS+=" a_tsmapi.c"
export GLOBOBJS+=" am_map.c"
export GLOBOBJS+=" d_items.c"
export GLOBOBJS+=" d_main.c"
export GLOBOBJS+=" f_finale.c"
export GLOBOBJS+=" f_wipe.c"
export GLOBOBJS+=" g_game.c"
export GLOBOBJS+=" hu_stuff.c"
export GLOBOBJS+=" i_audio.c"
#export GLOBOBJS+=" i_main.c"
export GLOBOBJS+=" i_main.o"
#export GLOBOBJS+=" i_system.c"
export GLOBOBJS+=" i_system.o"
export GLOBOBJS+=" info.c"
export GLOBOBJS+=" m_cheat.c"
export GLOBOBJS+=" m_menu.c"
export GLOBOBJS+=" m_random.c"
export GLOBOBJS+=" p_ceilng.c"
export GLOBOBJS+=" p_doors.c"
export GLOBOBJS+=" p_enemy.c"
export GLOBOBJS+=" p_floor.c"
export GLOBOBJS+=" p_genlin.c"
export GLOBOBJS+=" p_inter.c"
export GLOBOBJS+=" p_lights.c"
#export GLOBOBJS+=" p_map.c"
export GLOBOBJS+=" p_map.o"
export GLOBOBJS+=" p_maputl.c"
#export GLOBOBJS+=" p_mobj.c"
export GLOBOBJS+=" p_mobj.o"
export GLOBOBJS+=" p_plats.c"
export GLOBOBJS+=" p_pspr.c"
export GLOBOBJS+=" p_setup.c"
export GLOBOBJS+=" p_sight.c"
#export GLOBOBJS+=" p_spec.c"
export GLOBOBJS+=" p_spec.o"
#export GLOBOBJS+=" p_switch.c"
export GLOBOBJS+=" p_switch.o"
export GLOBOBJS+=" p_telept.c"
export GLOBOBJS+=" p_tick.c"
export GLOBOBJS+=" p_user.c"
export GLOBOBJS+=" r_data.c"
#export GLOBOBJS+=" r_draw.c"
export GLOBOBJS+=" r_draw.o"
export GLOBOBJS+=" r_plane.c"
export GLOBOBJS+=" r_sky.c"
export GLOBOBJS+=" r_things.c"
export GLOBOBJS+=" s_sound.c"
export GLOBOBJS+=" sounds.c"
export GLOBOBJS+=" st_stuff.c"
export GLOBOBJS+=" tables.c"
export GLOBOBJS+=" v_video.c"
export GLOBOBJS+=" w_wad.c"
export GLOBOBJS+=" wi_stuff.c"
#export GLOBOBJS+=" z_bmallo.c"
export GLOBOBJS+=" z_bmallo.o"
#export GLOBOBJS+=" z_zone.c"
export GLOBOBJS+=" z_zone.o"

ia16-elf-gcc $GLOBOBJS $CFLAGS -o GCCIA16/DOOM8088.EXE

rm i_main.o
rm i_system.o
rm p_map.o
rm p_mobj.o
rm p_spec.o
rm p_switch.o
rm r_draw.o
rm z_bmallo.o
rm z_zone.o

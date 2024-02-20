mkdir GCCIA16

unset CFLAGS

#export RENDER_OPTIONS="-DONE_WALL_TEXTURE -DFLAT_WALL -DFLAT_SPAN -DFLAT_SKY -DDISABLE_STATUS_BAR"
export RENDER_OPTIONS="-DFLAT_SPAN"

nasm i_vmodya.asm -f elf
nasm m_fixed.asm -f elf

ia16-elf-gcc -c i_vmodey.c $RENDER_OPTIONS -march=i286 -mcmodel=medium -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-loops -freorder-blocks-and-partition
ia16-elf-gcc -c p_maputl.c $RENDER_OPTIONS -march=i286 -mcmodel=medium -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-loops -freorder-blocks-and-partition
ia16-elf-gcc -c r_draw.c   $RENDER_OPTIONS -march=i286 -mcmodel=medium -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-loops -freorder-blocks-and-partition
ia16-elf-gcc -c tables.c   $RENDER_OPTIONS -march=i286 -mcmodel=medium -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-loops -freorder-blocks-and-partition

export CFLAGS="-march=i286 -mcmodel=medium -li86 -Os -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-loops -freorder-blocks-and-partition -foptimize-strlen -freorder-blocks-algorithm=stc -ftree-loop-vectorize -ftree-slp-vectorize"
#export CFLAGS="$CFLAGS -Ofast -flto -fwhole-program -fomit-frame-pointer -funroll-loops -fgcse-sm -fgcse-las -fipa-pta -Wno-attributes -Wpedantic"
#export CFLAGS="$CFLAGS -Wall -Wextra"

export GLOBOBJS="  a_pcfx.c"
export GLOBOBJS+=" a_taskmn.c"
export GLOBOBJS+=" am_map.c"
export GLOBOBJS+=" d_items.c"
export GLOBOBJS+=" d_main.c"
export GLOBOBJS+=" f_finale.c"
export GLOBOBJS+=" g_game.c"
export GLOBOBJS+=" hu_stuff.c"
export GLOBOBJS+=" i_audio.c"
export GLOBOBJS+=" i_main.c"
export GLOBOBJS+=" i_system.c"
#export GLOBOBJS+=" i_vmodey.c"
export GLOBOBJS+=" i_vmodey.o"
export GLOBOBJS+=" i_vmodya.o"
export GLOBOBJS+=" info.c"
export GLOBOBJS+=" m_cheat.c"
export GLOBOBJS+=" m_fixed.o"
export GLOBOBJS+=" m_menu.c"
export GLOBOBJS+=" m_random.c"
export GLOBOBJS+=" p_doors.c"
export GLOBOBJS+=" p_enemy.c"
export GLOBOBJS+=" p_floor.c"
export GLOBOBJS+=" p_inter.c"
export GLOBOBJS+=" p_lights.c"
export GLOBOBJS+=" p_map.c"
#export GLOBOBJS+=" p_maputl.c"
export GLOBOBJS+=" p_maputl.o"
export GLOBOBJS+=" p_mobj.c"
export GLOBOBJS+=" p_plats.c"
export GLOBOBJS+=" p_pspr.c"
export GLOBOBJS+=" p_setup.c"
export GLOBOBJS+=" p_sight.c"
export GLOBOBJS+=" p_spec.c"
export GLOBOBJS+=" p_switch.c"
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
#export GLOBOBJS+=" tables.c"
export GLOBOBJS+=" tables.o"
export GLOBOBJS+=" v_video.c"
export GLOBOBJS+=" w_wad.c"
export GLOBOBJS+=" wi_stuff.c"
export GLOBOBJS+=" z_bmallo.c"
export GLOBOBJS+=" z_zone.c"

ia16-elf-gcc $GLOBOBJS $CFLAGS $RENDER_OPTIONS -o GCCIA16/DOOM8088.EXE

rm i_vmodya.o
rm m_fixed.o
rm i_vmodey.o
rm p_maputl.o
rm r_draw.o
rm tables.o

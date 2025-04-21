mkdir GCCIA16

unset CFLAGS


export RENDER_OPTIONS="-DFLAT_SPAN -DFLAT_NUKAGE1_COLOR=65 -DFLAT_SKY_COLOR=3 -DWAD_FILE=\"DOOM4.WAD\" -DMAPWIDTH=480 -DNR_OF_COLORS=2"

export CPU=$1
export OUTPUT=$2

if [ -z "$CPU" ]
then
  #export CPU=i8088
  export CPU=i286
fi

if [ -z "$OUTPUT" ]
then
  export OUTPUT=DOOM8088.EXE
fi

nasm i_vcgaa.asm -f elf -DCPU=$CPU
nasm m_fixed.asm -f elf -DCPU=$CPU
nasm z_xms.asm   -f elf -DCPU=$CPU

ia16-elf-gcc -c i_vcgabw.c $RENDER_OPTIONS -march=$CPU -mcmodel=medium -mnewlib-nano-stdio -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -fno-tree-pre
ia16-elf-gcc -c p_enemy2.c $RENDER_OPTIONS -march=$CPU -mcmodel=medium -mnewlib-nano-stdio -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -fno-tree-pre
ia16-elf-gcc -c p_map.c    $RENDER_OPTIONS -march=$CPU -mcmodel=medium -mnewlib-nano-stdio -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -fno-tree-pre
ia16-elf-gcc -c p_maputl.c $RENDER_OPTIONS -march=$CPU -mcmodel=medium -mnewlib-nano-stdio -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -fno-tree-pre
ia16-elf-gcc -c p_mobj.c   $RENDER_OPTIONS -march=$CPU -mcmodel=medium -mnewlib-nano-stdio -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -fno-tree-pre
ia16-elf-gcc -c p_sight.c  $RENDER_OPTIONS -march=$CPU -mcmodel=medium -mnewlib-nano-stdio -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -fno-tree-pre
ia16-elf-gcc -c r_data.c   $RENDER_OPTIONS -march=$CPU -mcmodel=medium -mnewlib-nano-stdio -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -fno-tree-pre
ia16-elf-gcc -c r_draw.c   $RENDER_OPTIONS -march=$CPU -mcmodel=medium -mnewlib-nano-stdio -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -fno-tree-pre
ia16-elf-gcc -c r_plane.c  $RENDER_OPTIONS -march=$CPU -mcmodel=medium -mnewlib-nano-stdio -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -fno-tree-pre
ia16-elf-gcc -c tables.c   $RENDER_OPTIONS -march=$CPU -mcmodel=medium -mnewlib-nano-stdio -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -fno-tree-pre
ia16-elf-gcc -c w_wad.c    $RENDER_OPTIONS -march=$CPU -mcmodel=medium -mnewlib-nano-stdio -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -fno-tree-pre
ia16-elf-gcc -c z_zone.c   $RENDER_OPTIONS -march=$CPU -mcmodel=medium -mnewlib-nano-stdio -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -fno-tree-pre

export CFLAGS="-march=$CPU -mcmodel=medium -li86 -mnewlib-nano-stdio -Os -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparmcall -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -funsafe-loop-optimizations -freorder-blocks-algorithm=stc -fno-tree-pre -fira-region=mixed"
#export CFLAGS="$CFLAGS -Ofast -flto -fwhole-program -fomit-frame-pointer -funroll-loops -fgcse-sm -fgcse-las -fipa-pta -Wno-attributes -Wpedantic"
#export CFLAGS="$CFLAGS -Wall -Wextra"

export GLOBOBJS="  a_pcfx.c"
export GLOBOBJS+=" a_taskmn.c"
export GLOBOBJS+=" am_map.c"
export GLOBOBJS+=" d_items.c"
export GLOBOBJS+=" d_main.c"
export GLOBOBJS+=" f_finale.c"
export GLOBOBJS+=" f_lib.c"
export GLOBOBJS+=" g_game.c"
export GLOBOBJS+=" hu_stuff.c"
export GLOBOBJS+=" i_audio.c"
export GLOBOBJS+=" i_main.c"
export GLOBOBJS+=" i_system.c"
#export GLOBOBJS+=" i_vcgabw.c"
export GLOBOBJS+=" i_vcgabw.o"
export GLOBOBJS+=" i_vcgaa.o"
export GLOBOBJS+=" info.c"
export GLOBOBJS+=" m_cheat.c"
export GLOBOBJS+=" m_fixed.o"
export GLOBOBJS+=" m_menu.c"
export GLOBOBJS+=" m_random.c"
export GLOBOBJS+=" p_doors.c"
export GLOBOBJS+=" p_enemy.c"
#export GLOBOBJS+=" p_enemy2.c"
export GLOBOBJS+=" p_enemy2.o"
export GLOBOBJS+=" p_floor.c"
export GLOBOBJS+=" p_inter.c"
export GLOBOBJS+=" p_lights.c"
#export GLOBOBJS+=" p_map.c"
export GLOBOBJS+=" p_map.o"
#export GLOBOBJS+=" p_maputl.c"
export GLOBOBJS+=" p_maputl.o"
#export GLOBOBJS+=" p_mobj.c"
export GLOBOBJS+=" p_mobj.o"
export GLOBOBJS+=" p_plats.c"
export GLOBOBJS+=" p_pspr.c"
export GLOBOBJS+=" p_setup.c"
#export GLOBOBJS+=" p_sight.c"
export GLOBOBJS+=" p_sight.o"
export GLOBOBJS+=" p_spec.c"
export GLOBOBJS+=" p_switch.c"
export GLOBOBJS+=" p_telept.c"
export GLOBOBJS+=" p_tick.c"
export GLOBOBJS+=" p_user.c"
#export GLOBOBJS+=" r_data.c"
export GLOBOBJS+=" r_data.o"
#export GLOBOBJS+=" r_draw.c"
export GLOBOBJS+=" r_draw.o"
#export GLOBOBJS+=" r_plane.c"
export GLOBOBJS+=" r_plane.o"
export GLOBOBJS+=" r_sky.c"
export GLOBOBJS+=" r_things.c"
export GLOBOBJS+=" s_sound.c"
export GLOBOBJS+=" sounds.c"
export GLOBOBJS+=" st_pal.c"
export GLOBOBJS+=" st_stuff.c"
#export GLOBOBJS+=" tables.c"
export GLOBOBJS+=" tables.o"
export GLOBOBJS+=" v_video.c"
#export GLOBOBJS+=" w_wad.c"
export GLOBOBJS+=" w_wad.o"
export GLOBOBJS+=" wi_lib.c"
export GLOBOBJS+=" wi_stuff.c"
export GLOBOBJS+=" z_bmallo.c"
export GLOBOBJS+=" z_xms.o"
#export GLOBOBJS+=" z_zone.c"
export GLOBOBJS+=" z_zone.o"

ia16-elf-gcc $GLOBOBJS $CFLAGS $RENDER_OPTIONS -o GCCIA16/$OUTPUT

rm i_vcgaa.o
rm m_fixed.o
rm z_xms.o

rm i_vcgabw.o
rm p_enemy2.o
rm p_map.o
rm p_maputl.o
rm p_mobj.o
rm p_sight.o
rm r_data.o
rm r_draw.o
rm r_plane.o
rm tables.o
rm w_wad.o
rm z_zone.o

#!/bin/sh
NOTOUCH=1 make debug -j 18 && cp build/debug-mingw32-x86/ioquake3.x86.exe /e/games/UrbanTerror/ioq3-urt.exe && gdb --cd /e/games/UrbanTerror --args ioq3-urt.exe +set fs_basepath /e/games/UrbanTerror +set fs_homepath ./tmp +set s_volume 0.1 +set s_musicvolume 0 +set r_mode 6 +set r_fullscreen 0 +set com_safemode 1 +set name test +set cl_nologo 1 +set r_gamma 1
#sh /e/games/UrbanTerror/DEBUGGING/debugurt2.txt

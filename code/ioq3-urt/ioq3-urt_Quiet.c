// under GPL v2

#include "ioq3-urt.h"

//remove some game generated technical messages only when com_quiet and com_quietuglyhack are set
qboolean Com_quiet_ugly_hack(char* txt) {
        if (clu.com_quietuglyhack && com_quiet && clu.com_quietuglyhack->integer && com_quiet->integer) {
                if              (!strncmp(txt, "UI menu load time", strlen("UI menu load time"))) return qtrue;
                else if (!strncmp(txt + strlen("## "), "bots parsed", strlen("bots parsed"))) return qtrue;
                else if (!strncmp(txt + strlen("## "), "crosshairs parsed", strlen("crosshairs parsed"))) return qtrue;
                else if (!strncmp(txt, "------- Game Initialization -------", strlen("------- Game Initialization -------"))) return qtrue;
                else if (!strncmp(txt, "gamename: ", strlen("gamename: "))) return qtrue;
                else if (!strncmp(txt, "gamedate: ", strlen("gamedate: "))) return qtrue;
                else if (!strncmp(txt, "-----------------------------------", strlen("-----------------------------------"))) return qtrue;
                else if (!strncmp(txt + strlen("## "), "UT bots parsed", strlen("UT bots parsed"))) return qtrue;
                else if (!strncmp(txt + strlen("## "), "arenas parsed", strlen("arenas parsed"))) return qtrue;
                else if (!strncmp(txt, "2 teams with ", strlen("2 teams with ")) && (!strncmp(txt + strlen("2 teams with ###"), " entities", strlen(" entities")) || !strncmp(txt + strlen("2 teams with ##"), " entities", strlen(" entities")))) return qtrue;
                else if (!strncmp(txt, "==== ShutdownGame ====", strlen("==== ShutdownGame ===="))) return qtrue;
                else if (!strncmp(txt, "Welcome to Urban Terror 4.", strlen("Welcome to Urban Terror 4."))) strcpy(txt,"Welcome to Urban Terror 4.x"); //no return here.
        }
        return qfalse; //required for this
}


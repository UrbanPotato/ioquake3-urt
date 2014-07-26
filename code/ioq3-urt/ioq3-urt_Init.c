//under GPL v2

#ifndef DEDICATED
#include "SDL.h"
#endif
#include "ioq3-urt.h"
#include "ioq3-urt_Init.h"
#include "../client/snd_local.h" // for MAX_CHANNELS

/*      initialize ioq3-urt's stuff
        earliest point this can be called is right after Cvar_Init () (in Com_Init() of common.c)  */
void ioq3_urt_init (void) {

        // Both Dedicated server and Client-Server

        #ifdef WIN32
                /*      Up the limit of simultaneous files open (usually for .pk3s) to 2048.
                        Notice this does not mean 2048 .pk3s will usually be allowed, due to BIG_INFO_STRING limitations. */
                int current_limit = _getmaxstdio();
                if(current_limit < 2048) {
                        //Com_Printf(   "* Simultaneously open files permitted: %i; not enough. Setting it to 2048.", current_limit);
                        _setmaxstdio(2048);
                }
        #endif

        clu.sys_microGranularity                        = Cvar_Get ("sysMicroGranularity", "1", CVAR_ARCHIVE);
        clu.sys_priority = Cvar_Get ("sysPriority", "0", CVAR_ARCHIVE);
        clu.Sys_MicroGranularityCheck   = Sys_MicroGranularityCheck;

        set_envs();

        Cmd_AddCommand("sysexec", Com_Sysexec_f);

        clu.Com_quiet_ugly_hack=Com_quiet_ugly_hack;
        com_quiet = Cvar_Get ("cl_quiet", "1", CVAR_ARCHIVE); //legacy to cl_ ; maybe change it later since it's default to 1 anyway
        clu.com_quietuglyhack = Cvar_Get ("cl_quietUglyHack", "1", CVAR_ARCHIVE); //legacy to cl_ ; maybe change it later since it's default to 1 anyway

        clu.NET_DynamicPort = Cvar_Get ("net_dynamicPort", "1", CVAR_ARCHIVE);

        #ifndef DEDICATED // Client-server only

                clu.com_nosafemode = Cvar_Get ("com_nosafemode", "0", CVAR_ARCHIVE);

                clu.DNS_workaround=DNS_workaround;

                #ifdef BUILD_FREETYPE

                        clu.FTGL_Init=FTGL_Init;
                        clu.Unicode_FindSlot=Unicode_FindSlot;
                        clu.Unicode_ChatOutput_f=Unicode_ChatOutput_f;
                        clu.Unicode_Init=Unicode_Init;
                        clu.Unicode_Render=Unicode_Render;
                        clu.Unicode_Shutdown=Unicode_Shutdown;

                        clu.Unicode_Color = Cvar_Get("Unicode_Color", "95 197 255", CVAR_ARCHIVE);
                        clu.Unicode_Shadow = Cvar_Get("Unicode_Shadow", "1.004 1.01", CVAR_ARCHIVE);
                        clu.Unicode_Alpha = Cvar_Get("Unicode_Alpha", "0.85", CVAR_ARCHIVE);
                        clu.Unicode_Font = Cvar_Get("Unicode_Font", "LiberationSerif-Regular.ttf", CVAR_ARCHIVE);
                        clu.Unicode_ConsoleOutput = Cvar_Get("Unicode_ConsoleOutput", "0", CVAR_ARCHIVE);
                        clu.Unicode_ChatOutput = Cvar_Get("Unicode_ChatOutput", "0", CVAR_ARCHIVE);
                        clu.Unicode_Enable = Cvar_Get("Unicode_Enable", "1", CVAR_ARCHIVE);
                        clu.Unicode_Fontsize = Cvar_Get("Unicode_Fontsize", "0.022", CVAR_ARCHIVE);
                        clu.Unicode_LineSpace = Cvar_Get("Unicode_LineSpace", "-1.3", CVAR_ARCHIVE);
                        clu.Unicode_LineSize = Cvar_Get("Unicode_LineSize", "64", CVAR_ARCHIVE);
                        clu.Unicode_TranslationOutput = Cvar_Get("Unicode_TranslationOutput", "1", CVAR_ARCHIVE);
                        clu.Unicode_Lines = Cvar_Get("Unicode_Lines", "10", CVAR_ARCHIVE);
                        clu.Unicode_PositionX = Cvar_Get("Unicode_PositionX", "11", CVAR_ARCHIVE);
                        clu.Unicode_PositionY = Cvar_Get("Unicode_PositionY", "4", CVAR_ARCHIVE);
                        clu.Unicode_MessageTime = Cvar_Get("Unicode_MessageTime", "4500", CVAR_ARCHIVE);
                        clu.Unicode_MessageFadeTime = Cvar_Get("Unicode_MessageFadeTime", "1350", CVAR_ARCHIVE);
                        clu.Unicode_Greeting = Cvar_Get("Unicode_Greeting", "Unicode support Initialized", CVAR_ARCHIVE);
                        clu.Unicode_Library = Cvar_Get("Unicode_Library", DEFAULT_FTGL_LIB, CVAR_ARCHIVE);
                #endif

                #ifdef USE_CURL

                        clu.Com_Translate_Init=Com_Translate_Init;
                        clu.Com_Translate_DispatchMessages=Com_Translate_DispatchMessages;
                        clu.Com_Translate_Auto=Com_Translate_Auto;
                        clu.translateIn = Cvar_Get ("translateIn", "auto", CVAR_ARCHIVE);
                        clu.translateOut = Cvar_Get ("translateOut", "en", CVAR_ARCHIVE);
                        clu.translateAuto = Cvar_Get ("translateAuto", "0", CVAR_ARCHIVE);
                        clu.cl_translation = Cvar_Get ("cl_translation", "1", CVAR_ARCHIVE);
                        clu.cl_translationSleep = Cvar_Get ("cl_translationSleep", "40", CVAR_ARCHIVE);
                        clu.translateAutoFilterServer = Cvar_Get ("translateAutoFilterServer", "1", CVAR_ARCHIVE);
                        clu.translateAutoFilterShorterThan = Cvar_Get ("translateAutoFilterShorterThan", "3", CVAR_ARCHIVE);

                        Cmd_AddCommand ("translate", Com_Translate_f);
                #endif

                clu.com_Sleep = Cvar_Get ("com_Sleep", "0", CVAR_ARCHIVE);
                clu.com_SleepWhenUnfocused = Cvar_Get ("com_SleepWhenUnfocused", "12", CVAR_ARCHIVE);
                clu.com_SleepWhenMinimized = Cvar_Get ("com_SleepWhenMinimized", "12", CVAR_ARCHIVE);
                clu.Com_SleepCheck = Com_SleepCheck;
                Cvar_CheckRange(clu.com_Sleep, 0, 100, qtrue); // restrict them since they are quite aggressive
                Cvar_CheckRange(clu.com_SleepWhenUnfocused, 0, 100, qtrue);
                Cvar_CheckRange(clu.com_SleepWhenMinimized, 0, 100, qtrue);


                clu.r_motionBlur = Cvar_Get ("r_motionBlur", "0", CVAR_ARCHIVE);
                clu.r_motionBlurStrength = Cvar_Get ("r_motionBlurStrength", "0.9", CVAR_ARCHIVE);
                Cvar_CheckRange(clu.r_motionBlurStrength, 0,    0.99, qfalse);
                clu.R_MotionBlur_f = R_MotionBlur_f;

                Cmd_AddCommand("bindTeleport", CL_BindTeleport_f);

                clu.mouse.cl_drawmouse = Cvar_Get ("cl_drawMouse", "0", CVAR_ARCHIVE);
                clu.mouse.cl_drawmouseinterval = Cvar_Get ("cl_drawMouseInterval", "0.08", CVAR_ARCHIVE);
                Cvar_CheckRange(clu.mouse.cl_drawmouseinterval, 0,      10, qfalse);
                clu.mouse.cl_drawmouseposx = Cvar_Get ("cl_drawMousePosX", "0", CVAR_ARCHIVE);
                clu.mouse.cl_drawmouseposy = Cvar_Get ("cl_drawMousePosY", "3", CVAR_ARCHIVE);
                clu.mouse.cl_drawmousesize = Cvar_Get ("cl_drawMouseSize", "7", CVAR_ARCHIVE);

                clu.r_windowPosition = Cvar_Get ("r_windowPosition",    "0,0", CVAR_ARCHIVE);

                clu.s_smpRepeat = Cvar_Get ("s_smpRepeat",      "16", CVAR_ARCHIVE);
                Cvar_CheckRange(clu.s_smpRepeat, 8,     MAX_CHANNELS, qtrue);

                clu.cl_aviMotionJpegQuality = Cvar_Get ("cl_aviMotionJpegQuality", "90", CVAR_ARCHIVE);
                clu.r_screenshotJPEGQuality = Cvar_Get ("r_screenshotJPEGQuality", "90", CVAR_ARCHIVE);

                Cmd_AddCommand ("rebind",Key_Rebind_f);

                clu.r_blank = Cvar_Get ("r_blank", "0", CVAR_ARCHIVE);
                clu.R_Blank = R_Blank;
                Cmd_AddCommand ("+blank",R_Blank_f);
                Cmd_AddCommand ("-blank",R_Blank_f);
                clu.r_blankrgb = Cvar_Get ("r_blankRGB", "0 0 0 0", CVAR_ARCHIVE);

                Cmd_AddCommand( "totalTimeRun", Com_Totaltimerun_f );
                clu.com_totaltimerunsave = Cvar_Get ("cl_totalTimeRunSave", "0", CVAR_ARCHIVE); //cl_ is legacy, it should be com_
                clu.Com_Totaltimerun_save=Com_Totaltimerun_save;
                clu.Com_Totaltimerun_f=Com_Totaltimerun_f;

#ifndef DEDICATED
                //minimize window via SDL
                Cmd_AddCommand( "minimize", (xcommand_t)SDL_WM_IconifyWindow );
                clu.cl_alttabminimize = Cvar_Get ("cl_altTabMinimize", "1", CVAR_ARCHIVE);
#endif
                clu.one_pk3=one_pk3;

                clu.PreCache_UT=PreCache_UT;

                clu.cl_autotimenudge = Cvar_Get ("cl_autoTimenudge", "1", CVAR_ARCHIVE); //could be off for performance, though very-very minimal
                clu.cl_autotimenudgeinterval = Cvar_Get ("cl_autoTimenudgeInterval", "1", CVAR_ARCHIVE);
                clu.cl_autotimenudgefloodprotsec = Cvar_Get ("cl_autoTimenudgeFloodProtSec", "5", CVAR_ARCHIVE);
                clu.cl_autotimenudgecommand = Cvar_Get ("cl_autoTimenudgeCommand", "ut_timenudge", CVAR_ARCHIVE);
                clu.cl_autotimenudgelive =  Cvar_Get ("cl_autoTimenudgeLive", "0", CVAR_ARCHIVE);
                clu.CL_Autotimenudge=CL_Autotimenudge;
                Cmd_AddCommand( "autoTimenudge", CL_Autotimenudge_f);

                clu.cl_recordfontsize = Cvar_Get ("cl_recordFontSize", "8", CVAR_ARCHIVE);

                clu.cl_nologo = Cvar_Get ("cl_noLogo", "0", CVAR_ARCHIVE);

                Cmd_AddCommand( "cl_utRadio", cl_utradio );

                clu.cl_drawclock         = Cvar_Get ("cl_drawClock", "0", CVAR_ARCHIVE);
                clu.cl_drawclock12 = Cvar_Get ("cl_drawClock12", "1", CVAR_ARCHIVE);
                clu.cl_drawclockalarm24 = Cvar_Get ("cl_drawClockAlarm24", "0", CVAR_ARCHIVE);
                clu.cl_drawclockshowseconds = Cvar_Get ("cl_drawClockShowSeconds", "0", CVAR_ARCHIVE);
                clu.cl_drawclockalarmcmd = Cvar_Get ("cl_drawClockAlarmCmd", "play music/mainmenu", CVAR_ARCHIVE);
                clu.cl_drawclockposx = Cvar_Get ("cl_drawClockPosX", "2", CVAR_ARCHIVE); //9 to sim iourt
                clu.cl_drawclockposy = Cvar_Get ("cl_drawClockPosY", "42", CVAR_ARCHIVE); //1 to sim iourt
                clu.cl_drawclockfontsize         = Cvar_Get ("cl_drawClockFontSize", "6", CVAR_ARCHIVE);
                clu.cl_drawclockcolor = Cvar_Get ("cl_drawClockColor", "8", CVAR_ARCHIVE);

                clu.cl_drawping = Cvar_Get ("cl_drawPing", "0", CVAR_ARCHIVE);
                clu.cl_drawpingfontsize = Cvar_Get ("cl_drawPingFontSize", "7", CVAR_ARCHIVE);
                clu.cl_drawpingposx = Cvar_Get ("cl_drawPingPosX", "0", CVAR_ARCHIVE);
                clu.cl_drawpingposy = Cvar_Get ("cl_drawPingPosY", "14", CVAR_ARCHIVE);
                clu.cl_drawpingfirstinterval = Cvar_Get ("cl_drawPingFirstInterval", "2", CVAR_ARCHIVE);
                clu.cl_drawpingsecondinterval = Cvar_Get ("cl_drawPingSecondInterval", "10", CVAR_ARCHIVE);

                clu.cl_drawfps = Cvar_Get ("cl_drawFPS", "0", CVAR_ARCHIVE);
                clu.cl_drawfpsfontsize = Cvar_Get ("cl_drawFPSfontSize", "7", CVAR_ARCHIVE);
                clu.cl_drawfpsposx = Cvar_Get ("cl_drawFPSposx", "0", CVAR_ARCHIVE);
                clu.cl_drawfpsposy = Cvar_Get ("cl_drawFPSposy", "11", CVAR_ARCHIVE);
                clu.cl_drawfpsinterval = Cvar_Get ("cl_drawFPSinterval", "0.9", CVAR_ARCHIVE);

                clu.cl_drawsnaps = Cvar_Get ("cl_drawSnaps", "0", CVAR_ARCHIVE);
                clu.cl_drawsnapsfontsize = Cvar_Get ("cl_drawSnapsFontSize", "7", CVAR_ARCHIVE);
                clu.cl_drawsnapsposx = Cvar_Get ("cl_drawSnapsPosX", "0", CVAR_ARCHIVE);
                clu.cl_drawsnapsposy = Cvar_Get ("cl_drawSnapsPosY", "7", CVAR_ARCHIVE);
                clu.cl_drawsnapsfirstinterval = Cvar_Get ("cl_drawSnapsFirstInterval", "2", CVAR_ARCHIVE);
                clu.cl_drawsnapssecondinterval = Cvar_Get ("cl_drawSnapsSecondInterval", "10", CVAR_ARCHIVE);

                clu.cl_stopwatch = Cvar_Get ("cl_stopwatch", "0", CVAR_ARCHIVE);
                clu.cl_stopwatchposx = Cvar_Get ("cl_stopwatchPosX", "8", CVAR_ARCHIVE);
                clu.cl_stopwatchposy = Cvar_Get ("cl_stopwatchPosY", "23", CVAR_ARCHIVE);
                clu.cl_stopwatchsize = Cvar_Get ("cl_stopwatchSize", "12", CVAR_ARCHIVE);
                Cmd_AddCommand( "cl_stopwatchReset", cl_stopwatchreset );
                Cmd_AddCommand( "cl_stopwatchStartPause", cl_stopwatchstartpause );

                clu.cl_drawpackets = Cvar_Get ("cl_drawPackets", "0", CVAR_ARCHIVE);
                clu.cl_drawpacketsposx = Cvar_Get ("cl_drawpacketsposx", "16", CVAR_ARCHIVE);
                clu.cl_drawpacketsposy = Cvar_Get ("cl_drawpacketsposy", "2", CVAR_ARCHIVE);
                clu.cl_drawpacketsfontsize = Cvar_Get ("cl_drawpacketsfontsize", "7", CVAR_ARCHIVE);
                clu.cl_drawpacketsfirstinterval = Cvar_Get ("cl_drawpacketsfirstinterval", "2", CVAR_ARCHIVE);
                clu.cl_drawpacketssecondinterval = Cvar_Get ("cl_drawpacketssecondtinterval", "10", CVAR_ARCHIVE);

                clu.SCR_ioq3_urt=SCR_ioq3_urt;

                clu.cl_zoomsensitivity = Cvar_Get ("zoomSensitivity", "1", CVAR_ARCHIVE);
                clu.cl_zoomsensitivityfovthreshold =  Cvar_Get ("cl_zoomSensitivityFOVthreshold", "90", CVAR_ARCHIVE); //zoomed dest->fovX: 24, normal: 90-110
                clu.CL_Zoom_Sens=CL_Zoom_Sens;
                clu.R_Zoom_Sens=R_Zoom_Sens;

                clu.reconnect_workaround=reconnect_workaround;
                clu.CL_Reconnect_f_clu=CL_Reconnect_f_clu;

                #ifdef _WIN32 // Windows only Client-Server
                        clu.raw.IN_RawInit                      = IN_RawInit;
                        clu.raw.IN_RawDeregister                = IN_RawDeregister;
                        clu.raw.init_WndProc                    = init_WndProc;
                        clu.raw.IN_RawProcess           = IN_RawProcess;
                        clu.raw.in_rawmouse                     = Cvar_Get ("in_rawMouse", "1", CVAR_ARCHIVE);

                        clu.SDL_EnableKeyRepeatWin = SDL_EnableKeyRepeatWin;
                        clu.keyboard_done = qfalse;
                #endif

        #endif // Client-Server only

        com_newdemoformat = Cvar_Get("com_newdemoformat", "1", CVAR_LATCH | CVAR_ARCHIVE); //r00t

        clu.initialized = qtrue;
}


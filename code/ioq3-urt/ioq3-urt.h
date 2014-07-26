//under GPL v2

#include "../renderer/tr_local.h"

/*
        ioq3-urt's struct
        Most new console variables and functions are called via this.
        Hence type 'clu.' in an IDE and get most additions of ioq3-urt.
        Some variables only used locally in ioq3-urt.c, only appear there.
*/

#ifdef WIN32
        typedef struct {
                int                     x, y;
                short int       butsnum;
                USHORT          buts[64];
                SHORT           wheeldat[64]; //it is signed even if it reads from unsigned since we check for negatives
                qboolean                registered;

                void                    (*IN_RawInit)(void);
                void                    (*IN_RawDeregister)(void);
                void                    (*init_WndProc)(void);
                void                    (*IN_RawProcess)(void);
                cvar_t          *in_rawmouse;
        } raw_t;
#endif

typedef struct {
        cvar_t                          *cl_drawmouse;
        cvar_t                          *cl_drawmouseinterval;
        cvar_t                          *cl_drawmouseposx;
        cvar_t                          *cl_drawmouseposy;
        cvar_t                          *cl_drawmousesize;

        int                                     maxx, maxy;
        unsigned long long  sumx, sumy;
        unsigned int                    count;
} drawmouse_t;

typedef struct {
        cvar_t                  *com_nosafemode;

        void                            (*DNS_workaround)(void);

        qboolean                        (*FTGL_Init)(void);
        void                            (*Unicode_Init)(void);
        void                            (*Unicode_Shutdown)(void);
        void                            (*Unicode_Render)(void);
        void                            (*Unicode_FindSlot)(char* Message, int FadeTimeout);
        void                            (*Unicode_ChatOutput_f)(char* Message);
        cvar_t                  *Unicode_Enable;
        cvar_t                  *Unicode_TranslationOutput;
        cvar_t                  *Unicode_ConsoleOutput;
        cvar_t                  *Unicode_ChatOutput;
        cvar_t                  *Unicode_Font;
        cvar_t                  *Unicode_Color;
        cvar_t                  *Unicode_Shadow;
        cvar_t                  *Unicode_Alpha;
        cvar_t                  *Unicode_PositionX;
        cvar_t                  *Unicode_PositionY;
        cvar_t                  *Unicode_Fontsize;
        cvar_t                  *Unicode_LineSpace;
        cvar_t                  *Unicode_Lines;
        cvar_t                  *Unicode_LineSize;
        cvar_t                  *Unicode_MessageTime;
        cvar_t                  *Unicode_MessageFadeTime;
        cvar_t                  *Unicode_Greeting;
        cvar_t                  *Unicode_Library;
        qboolean                        FTGLEnabled;

        void                            (*Com_Translate_Init)(void);
        void                            (*Com_Translate_Auto)(char*);
        void                            (*Com_Translate_DispatchMessages)(void);
        cvar_t                  *translateIn;
        cvar_t                  *translateOut;
        cvar_t                  *translateAuto;
        cvar_t                  *translateAutoFilterServer;
        cvar_t                  *translateAutoFilterShorterThan;
        qboolean                        TranslationInitialized;
        cvar_t                  *cl_translation;
        cvar_t                  *cl_translationSleep;

        cvar_t                  *com_Sleep;                                     // Explicit sleeping
        cvar_t                  *com_SleepWhenUnfocused;
        cvar_t                  *com_SleepWhenMinimized;
        void                            (*Com_SleepCheck)(void);                // warn harshly initially if com_Sleep is used

        void                            (*R_MotionBlur_f)(void);
        cvar_t                  *r_motionBlur;
        cvar_t                  *r_motionBlurStrength;

        cvar_t                  *sys_priority;
        cvar_t                  *sys_microGranularity;
        void                            (*Sys_MicroGranularityCheck)(void);
                                        //Sys_Microseconds() not in the struct to facilitate better initialization

        drawmouse_t     mouse;

        cvar_t          *r_windowPosition; //window position when r_noborder 1 and r_centerWindow 0.

        cvar_t          *s_smpRepeat; //be more forgiving of simultaneous playback of samples; default 8 of Listener was breaking automatic firing

        cvar_t          *NET_DynamicPort; //for dynamic port for NAT clients workaround

        cvar_t          *cl_aviMotionJpegQuality; //avi jpeg quality
        cvar_t          *r_screenshotJPEGQuality; //screenshot jpeg quality

        cvar_t          *r_blank; //blank the screen
        void                    (*R_Blank)(void);
        cvar_t          *r_blankrgb;

        cvar_t          *cl_alttabminimize;

        void                    (*SCR_ioq3_urt)(void); //all meters

        short int       done_reconnect; //for workaround for 'low pings/reconnect', to reset on connection

        void                    (*one_pk3)(void); //for auto-downloading

        cvar_t          *com_totaltimerunsave;
        void            (*Com_Totaltimerun_save)(void);
        void                    (*Com_Totaltimerun_f)(void);

        void                    (*PreCache_UT) (void);

        int                     atstd; //autotimenudge recorded standard deviation
        int                     atchoice; //autotimenudge recorded choice of value
        cvar_t          *cl_autotimenudge;
        cvar_t          *cl_autotimenudgeinterval;
        cvar_t          *cl_autotimenudgefloodprotsec;
        cvar_t          *cl_autotimenudgecommand;
        cvar_t          *cl_autotimenudgelive; //VERY experimental, especially on round-based games
        void                    (*CL_Autotimenudge)(void);


        void            (*R_Zoom_Sens)(viewParms_t*);
        float           (*CL_Zoom_Sens)(void);
        cvar_t          *cl_zoomsensitivity;
        cvar_t          *cl_zoomsensitivityfovthreshold; //to make it less hacky
        qboolean                do_zoomsens;

        cvar_t          *cl_recordfontsize;

        cvar_t          *cl_stopwatch;
        cvar_t          *cl_stopwatchposx;
        cvar_t          *cl_stopwatchposy;
        cvar_t          *cl_stopwatchsize;

        cvar_t          *cl_drawclock;
        cvar_t          *cl_drawclockshowseconds;
        cvar_t          *cl_drawclock12;
        cvar_t          *cl_drawclockalarm24;
        cvar_t          *cl_drawclockalarmcmd;
        cvar_t          *cl_drawclockposx;
        cvar_t          *cl_drawclockposy;
        cvar_t          *cl_drawclockfontsize;
        cvar_t          *cl_drawclockcolor;

        cvar_t          *cl_drawping;
        cvar_t          *cl_drawpingfontsize;
        cvar_t          *cl_drawpingposx;
        cvar_t          *cl_drawpingposy;
        cvar_t          *cl_drawpingfirstinterval;
        cvar_t          *cl_drawpingsecondinterval;

        cvar_t          *cl_drawfps;
        cvar_t          *cl_drawfpsfontsize;
        cvar_t          *cl_drawfpsposx;
        cvar_t          *cl_drawfpsposy;
        cvar_t          *cl_drawfpsinterval;

        cvar_t          *cl_drawsnaps;
        cvar_t          *cl_drawsnapsfontsize;
        cvar_t          *cl_drawsnapsposx;
        cvar_t          *cl_drawsnapsposy;
        cvar_t          *cl_drawsnapsfirstinterval;
        cvar_t          *cl_drawsnapssecondinterval;

        int                     sent; //used by drawpackets meter
        cvar_t          *cl_drawpackets;
        cvar_t          *cl_drawpacketsposx;
        cvar_t          *cl_drawpacketsposy;
        cvar_t          *cl_drawpacketsfontsize;
        cvar_t          *cl_drawpacketsfirstinterval;
        cvar_t          *cl_drawpacketssecondinterval;

        cvar_t          *cl_nologo; //'cause +nosplash was a workaround, it doesn't really exist, it only hijacks the command line; any +asdfasdf would work.

                                //com_quiet is excluded from the struct since it makes rebuilding hell.
        cvar_t          *com_quietuglyhack;
        qboolean                (*Com_quiet_ugly_hack)(char*);

        void                    (*reconnect_workaround)(char*);
        void                    (*CL_Reconnect_f_clu)(void);

        qboolean                initialized;
        #ifdef _WIN32
                raw_t           raw;

                void            (*SDL_EnableKeyRepeatWin)(void);
                int                     keyboard_done;
        #endif

} ioq3_urt_t;

extern ioq3_urt_t  clu; //informing the program about it; declared at the top of ioq3-urt.c
extern cvar_t *com_quiet;
extern cvar_t *com_newdemoformat;

void ioq3_urt_init (void); //this isn't in the struct since it assigns its functions

//globalized:
extern cvar_t *Cvar_FindVar( const char *var_name );
extern qboolean mouseActive;

//Definitions:
#ifdef WIN32
        #define DEFAULT_FTGL_LIB "libftgl-2.dll" // MinGW-w64
        #define ALTERNATIVE_FTGL_LIB "ftgl.dll" // MSVC
#elif defined(MACOS_X)
        #define DEFAULT_FTGL_LIB "libftgl.dylib" //?? random
        #define ALTERNATIVE_FTGL_LIB "libftgl-2.dylib" //? random
#else
        #define DEFAULT_FTGL_LIB "libftgl.so.2" // Debian
        #define ALTERNATIVE_FTGL_LIB "libftgl.so.0" // RPM based?
#endif

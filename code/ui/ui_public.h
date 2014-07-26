/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#ifndef __UI_PUBLIC_H__
#define __UI_PUBLIC_H__

#define UI_API_VERSION  6

typedef struct {
        connstate_t             connState;
        int                             connectPacketCount;
        int                             clientNum;
        char                    servername[MAX_STRING_CHARS];
        char                    updateInfoString[MAX_STRING_CHARS];
        char                    messageString[MAX_STRING_CHARS];
        #ifdef USE_AUTH
        char            serverAddress[MAX_STRING_CHARS];
        #endif
} uiClientState_t;

typedef enum {
        UI_ERROR,                             //00
        UI_PRINT,                             //01
        UI_MILLISECONDS,                      //02
        UI_CVAR_SET,                          //03
        UI_CVAR_VARIABLEVALUE,                //04
        UI_CVAR_VARIABLESTRINGBUFFER,         //05
        UI_CVAR_SETVALUE,                     //06
        UI_CVAR_RESET,                        //07
        UI_CVAR_CREATE,                       //08
        UI_CVAR_INFOSTRINGBUFFER,             //09
        UI_ARGC,                              //10
        UI_ARGV,                              //11
        UI_CMD_EXECUTETEXT,                   //12
        UI_FS_FOPENFILE,                      //13
        UI_FS_READ,                           //14
        UI_FS_WRITE,                          //15
        UI_FS_FCLOSEFILE,                     //16
        UI_FS_GETFILELIST,                    //17
        UI_R_REGISTERMODEL,                   //18
        UI_R_REGISTERSKIN,                    //19
        UI_R_REGISTERSHADERNOMIP,             //20
        UI_R_CLEARSCENE,                      //21
        UI_R_ADDREFENTITYTOSCENE,             //22
        UI_R_ADDPOLYTOSCENE,                  //23
        UI_R_ADDLIGHTTOSCENE,                 //24
        UI_R_RENDERSCENE,                     //25
        UI_R_SETCOLOR,                        //26
        UI_R_DRAWSTRETCHPIC,                  //27
        UI_UPDATESCREEN,                      //28
        UI_CM_LERPTAG,                        //29
        UI_CM_LOADMODEL,                      //30
        UI_S_REGISTERSOUND,                   //31
        UI_S_STARTLOCALSOUND,                 //32
        UI_KEY_KEYNUMTOSTRINGBUF,             //33
        UI_KEY_GETBINDINGBUF,                 //34
        UI_KEY_SETBINDING,                    //35
        UI_KEY_ISDOWN,                        //36
        UI_KEY_GETOVERSTRIKEMODE,             //37
        UI_KEY_SETOVERSTRIKEMODE,             //38
        UI_KEY_CLEARSTATES,                   //39
        UI_KEY_GETCATCHER,                    //40
        UI_KEY_SETCATCHER,                    //41
        UI_GETCLIPBOARDDATA,                  //42
        UI_GETGLCONFIG,                       //43
        UI_GETCLIENTSTATE,                    //44
        UI_GETCONFIGSTRING,                   //45
        UI_LAN_GETPINGQUEUECOUNT,             //46
        UI_LAN_CLEARPING,                     //47
        UI_LAN_GETPING,                       //48
        UI_LAN_GETPINGINFO,                   //49
        UI_CVAR_REGISTER,                     //50
        UI_CVAR_UPDATE,                       //51
        UI_MEMORY_REMAINING,                  //52
        UI_GET_CDKEY,                         //53
        UI_SET_CDKEY,                         //54
        UI_R_REGISTERFONT,                    //55
        UI_R_MODELBOUNDS,                     //56
        UI_PC_ADD_GLOBAL_DEFINE,              //57
        UI_PC_LOAD_SOURCE,                    //58
        UI_PC_FREE_SOURCE,                    //59
        UI_PC_READ_TOKEN,                     //60
        UI_PC_SOURCE_FILE_AND_LINE,           //61
        UI_S_STOPBACKGROUNDTRACK,             //62
        UI_S_STARTBACKGROUNDTRACK,            //63
        UI_REAL_TIME,                         //64
        UI_LAN_GETSERVERCOUNT,                //65
        UI_LAN_GETSERVERADDRESSSTRING,        //66
        UI_LAN_GETSERVERINFO,                 //67
        UI_LAN_MARKSERVERVISIBLE,             //68
        UI_LAN_UPDATEVISIBLEPINGS,            //69
        UI_LAN_RESETPINGS,                    //70
        UI_LAN_LOADCACHEDSERVERS,             //71
        UI_LAN_SAVECACHEDSERVERS,             //72
        UI_LAN_ADDSERVER,                     //73
        UI_LAN_REMOVESERVER,                  //74
        UI_CIN_PLAYCINEMATIC,                 //75  OK
        UI_CIN_STOPCINEMATIC,                 //76
        UI_CIN_RUNCINEMATIC,                  //77
        UI_CIN_DRAWCINEMATIC,                 //78
        UI_CIN_SETEXTENTS,                    //79
        UI_R_REMAP_SHADER,                    //80
        UI_VERIFY_CDKEY,                      //81 OK
        UI_LAN_SERVERSTATUS,                  //82
        UI_LAN_GETSERVERPING,                 //83
        UI_LAN_SERVERISVISIBLE,               //84
        UI_LAN_COMPARESERVERS,                //85

        // 1.32
        UI_FS_SEEK,                           //86
        UI_SET_PBCLSTATUS,                    //87

#ifdef USE_AUTH
        UI_NET_STRINGTOADR,                   //88
        UI_Q_VSNPRINTF,                       //89
        UI_NET_SENDPACKET,                    //90
        UI_COPYSTRING,                        //91
        UI_SYS_STARTPROCESS,                  //92
#endif

        UI_MEMSET = 100,
        UI_MEMCPY,
        UI_STRNCPY,
        UI_SIN,
        UI_COS,
        UI_ATAN2,
        UI_SQRT,
        UI_FLOOR,
        UI_CEIL
} uiImport_t;

typedef enum {
        UIMENU_NONE,
        UIMENU_MAIN,
        UIMENU_INGAME,
        UIMENU_NEED_CD,
        UIMENU_BAD_CD_KEY,
        UIMENU_TEAM,
        UIMENU_POSTGAME
} uiMenuCommand_t;

#define SORT_HOST                       0
#define SORT_MAP                        1
#define SORT_CLIENTS            2
#define SORT_GAME                       3
#define SORT_PING                       4
#define SORT_PUNKBUSTER         5

typedef enum {
        UI_GETAPIVERSION = 0,   // system reserved

        UI_INIT,
//      void    UI_Init( void );

        UI_SHUTDOWN,
//      void    UI_Shutdown( void );

        UI_KEY_EVENT,
//      void    UI_KeyEvent( int key );

        UI_MOUSE_EVENT,
//      void    UI_MouseEvent( int dx, int dy );

        UI_REFRESH,
//      void    UI_Refresh( int time );

        UI_IS_FULLSCREEN,
//      qboolean UI_IsFullscreen( void );

        UI_SET_ACTIVE_MENU,
//      void    UI_SetActiveMenu( uiMenuCommand_t menu );

        UI_CONSOLE_COMMAND,
//      qboolean UI_ConsoleCommand( int realTime );

        UI_DRAW_CONNECT_SCREEN,
//      void    UI_DrawConnectScreen( qboolean overlay );
        UI_HASUNIQUECDKEY,

        #ifdef USE_AUTH
        //@Barbatos @Kalish
        UI_AUTHSERVER_PACKET
        #endif

// if !overlay, the background will be drawn, otherwise it will be
// overlayed over whatever the cgame has drawn.
// a GetClientState syscall will be made to get the current strings
} uiExport_t;

#endif

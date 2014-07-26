/*
    ####################################################################

    This file is part of "iKALiZER Manager".

    "iKALiZER Manager" is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    "iKALiZER Manager" is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with "iKALiZER Manager".
    If not, see <http://www.gnu.org/licenses/>.

    ####################################################################

    About "iKALiZER"

    The audio engine iKALiZER is the property of Christophe Gossa.
    The Windows Library (DLL, AEI...) of iKALiZER could only be freely
    distributed without any restriction with ioUrbanTerror and ioQuake.

    Copyright (c) 2008 Christophe Gossa
    All rights reserved.

    ####################################################################
*/


//#define DEBUG_COMMAND_STARTSOUND
//#define DEBUG_COMMAND

//Write a PCM 32 bits stereo...
//#define DEBUG2WAVE

//#define DEBUGTESTWHITENOISE

    #include "snd_local.h"
    #include "snd_codec.h"
    #include "client.h"
    #include "sam.h"
    #include "sam_lib.h"


#ifdef _WIN32
//    #include <windows.h>
//    #include <windowsx.h>
//    #include <winuser.h>
//    #include <commdlg.h>
//    #include <winbase.h>
#endif

    #include <windows.h>
    #include <tchar.h>
    #include <stdio.h>
    #include <string.h>
    #include <conio.h>
    #include <direct.h>
    #include <stdlib.h>
    //#include <commctrl.h>
    #include <fcntl.h>
    #include <io.h>
    #include <time.h>
    #include <malloc.h>


/* It's all long int -> int for status displays */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"

/* Ugly def */
cvar_t *Cvar_Set2( const char *var_name, const char *value, qboolean force);

    /*
        Var list
    */
    /*
    cvar_t  *ikalizerSamplingRate;
    cvar_t  *s_samSamplingRate;
    cvar_t  *s_samChannelMode;
    cvar_t  *s_samLatencyDuration;
    cvar_t  *s_samBufferDuration;
    cvar_t  *s_samVoiceCount;
    cvar_t  *s_samSFXCount;
    cvar_t  *s_samMemoryAlloc;
    cvar_t  *s_samLimiterLevel;
    cvar_t  *s_samEnableSMP;
    cvar_t  *s_samDeviceSelect;
    cvar_t  *s_samDynamicDelayLines;
    cvar_t  *s_samMaxUsage;
    */


    #define GetEntityCode(a,b)      ((((DWORD)a)<<16)|((DWORD)b))
    #define SAM_DEFAULTMASTERLEVEL  (-8.0F)

    /*
        SAM Library
    */


    typedef struct {
        void ** pProcAdress;
        char szProcName[64];
        long lOptionnal;
    } SAM_PROC;

    long    (*pSAM_Open) ( void * pDeviceParam, SAM_CONFIG * psamConfig );
    long    (*pSAM_Close) ( void );
    long    (*pSAM_LimiterSet) ( long lMode );
    long    (*pSAM_GetInfo) ( DWORD dwInfoID, DWORD * pdwOutData, char * pszOutData );

    long    (*pSAM_Message) ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB );

    long    (*pSAM_GainSet) ( BYTE bSetGainMaskVMS, float fGainVoice, float fGainMusic, float fGainStreaming );

    long    (*pSAM_SFX_Load) ( DWORD dwHandle, const char * pszSFXName, DWORD dwSampleRate, DWORD dwSamplesCount, BYTE bStereo, BYTE b16bits, void * pAudioData, DWORD * pdwAllocatedHandle );
    long    (*pSAM_SFX_Unload) ( DWORD dwHandle );
    long    (*pSAM_SFX_Free) ( DWORD dwHandle );
    long    (*pSAM_SFX_IsLoaded) ( const char * pszSFXName, DWORD * pdwAllocatedHandle );
    long    (*pSAM_SFX_GetLoadedMemoryState) ( DWORD dwHandle, DWORD * pdwLoadedMemoryState );
    long    (*pSAM_SFX_GetOldest) ( DWORD * pdwOldestHandle, DWORD * pdwReservedHandle, DWORD dwReservedHandleCount );
    long    (*pSAM_SFX_GetName) ( DWORD dwHandle, const char * pszSFXName );
    long    (*pSAM_VOICE_Alloc) ( DWORD * pdwVoiceHandle, long lForceAlloc, DWORD * pdwKilledVoiceHandle );
    long    (*pSAM_VOICE_AllocByVoiceHandle) ( DWORD dwVoiceHandle, long lForceAlloc );
    long    (*pSAM_VOICE_AllocByUserID) ( DWORD dwUserID, long lForceAlloc );
    long    (*pSAM_VOICE_Free) ( DWORD dwVoiceHandle );
    long    (*pSAM_VOICE_FreeByUserID) ( DWORD dwUserID );
    long    (*pSAM_VOICE_GetHandleByUserID) ( DWORD * pdwVoiceHandle, DWORD dwStartVoiceHandle, DWORD dwUserID );

    long    (*pSAM_VOICE_SetSFX) ( DWORD dwVoiceHandle, DWORD dwHandleSFX );
    long    (*pSAM_VOICE_SetSampleRate) ( DWORD dwVoiceHandle, DWORD dwSampleRate_Hz );
    long    (*pSAM_VOICE_SetLoop) ( DWORD dwVoiceHandle, long bLoop );
    long    (*pSAM_VOICE_SetMasterLevel) ( DWORD dwVoiceHandle, float fMasterLevel_dB );
    long    (*pSAM_VOICE_SetDistanceLevel) ( DWORD dwVoiceHandle, float fDistanceLevel_dB );
    long    (*pSAM_VOICE_SetRatioIIR) ( DWORD dwVoiceHandle, float fRatioIIR );
    long    (*pSAM_VOICE_SetPlay) ( DWORD dwVoiceHandle, long bPlay );
    long    (*pSAM_VOICE_SetOrigin) ( DWORD dwVoiceHandle, long lAngleDegrees, float fDistanceMeters );

    long    (*pSAM_VOICE_GetUserData) ( DWORD dwVoiceHandle, BYTE * pbReceiveBuffer, DWORD dwByteCount );
    long    (*pSAM_VOICE_SetUserData) ( DWORD dwVoiceHandle, BYTE * pbSendBuffer, DWORD dwByteCount );
    long    (*pSAM_VOICE_GetUserID) ( DWORD dwVoiceHandle, DWORD * pdwUserID );
    long    (*pSAM_VOICE_SetUserID) ( DWORD dwVoiceHandle, DWORD dwUserID );
    long    (*pSAM_VOICE_GetVoiceUsedCount) ( DWORD * pdwVoiceTotalUsedCount, DWORD * pdwVoiceUsedCountUnlooped, DWORD * pdwVoiceUsedCountLooped );

    long    (*pSAM_STREAM_AddData) ( DWORD dwSampleRate, DWORD dwSamplesCount, BYTE bStereo, BYTE bQuantification, const void * pAudioData, float fVolume );
    long    (*pSAM_STREAM_SetState) ( BYTE bPlayState );

    long    (*pSAM_MUSIC_AddSFX) ( DWORD dwHandleSFX, DWORD * pdwAllocatedEntry );
    long    (*pSAM_MUSIC_Delete) ( DWORD dwEntry );
    long    (*pSAM_MUSIC_SetLoop) ( DWORD dwEntry, BYTE bLoop );
    long    (*pSAM_MUSIC_SetJump) ( DWORD dwEntry, DWORD dwJumpEntry );
    long    (*pSAM_MUSIC_Play) ( DWORD dwEntry );



    SAM_PROC samProcList[] = {  { (void **)&pSAM_Open,                      "SAM_Open", 0 },
                                { (void **)&pSAM_Close,                     "SAM_Close", 0 },
                                { (void **)&pSAM_LimiterSet,                "SAM_LimiterSet", 0 },
                                { (void **)&pSAM_GetInfo,                   "SAM_GetInfo", 0 },
                                { (void **)&pSAM_GainSet,                   "SAM_GainSet", 0 },
                                { (void **)&pSAM_Message,                   "SAM_Message", 0 },
                                { (void **)&pSAM_SFX_Load,                  "SAM_SFX_Load", 0 },
                                { (void **)&pSAM_SFX_Unload,                "SAM_SFX_Unload", 0 },
                                { (void **)&pSAM_SFX_Free,                  "SAM_SFX_Free", 0 },
                                { (void **)&pSAM_SFX_IsLoaded,              "SAM_SFX_IsLoaded", 0 },
                                { (void **)&pSAM_SFX_GetLoadedMemoryState,  "SAM_SFX_GetLoadedMemoryState", 0 },
                                { (void **)&pSAM_SFX_GetOldest,             "SAM_SFX_GetOldest", 0 },
                                { (void **)&pSAM_SFX_GetName,               "SAM_SFX_GetName", 0 },
                                { (void **)&pSAM_VOICE_Alloc,               "SAM_VOICE_Alloc", 0 },
                                { (void **)&pSAM_VOICE_AllocByVoiceHandle,  "SAM_VOICE_AllocByVoiceHandle", 0 },
                                { (void **)&pSAM_VOICE_AllocByUserID,       "SAM_VOICE_AllocByUserID", 0 },
                                { (void **)&pSAM_VOICE_Free,                "SAM_VOICE_Free", 0 },
                                { (void **)&pSAM_VOICE_FreeByUserID,        "SAM_VOICE_FreeByUserID", 0 },
                                { (void **)&pSAM_VOICE_GetHandleByUserID,   "SAM_VOICE_GetHandleByUserID", 0 },
                                { (void **)&pSAM_VOICE_SetSFX,              "SAM_VOICE_SetSFX", 0 },
                                { (void **)&pSAM_VOICE_SetSampleRate,       "SAM_VOICE_SetSampleRate", 0 },
                                { (void **)&pSAM_VOICE_SetLoop,             "SAM_VOICE_SetLoop", 0 },
                                { (void **)&pSAM_VOICE_SetMasterLevel,      "SAM_VOICE_SetMasterLevel", 0 },
                                { (void **)&pSAM_VOICE_SetDistanceLevel,    "SAM_VOICE_SetDistanceLevel", 0 },
                                { (void **)&pSAM_VOICE_SetRatioIIR,         "SAM_VOICE_SetRatioIIR", 0 },
                                { (void **)&pSAM_VOICE_SetPlay,             "SAM_VOICE_SetPlay", 0 },
                                { (void **)&pSAM_VOICE_SetOrigin,           "SAM_VOICE_SetOrigin", 0 },
                                { (void **)&pSAM_VOICE_GetUserData,         "SAM_VOICE_GetUserData", 0 },
                                { (void **)&pSAM_VOICE_SetUserData,         "SAM_VOICE_SetUserData", 0 },
                                { (void **)&pSAM_VOICE_GetUserID,           "SAM_VOICE_GetUserID", 0 },
                                { (void **)&pSAM_VOICE_SetUserID,           "SAM_VOICE_SetUserID", 0 },
                                { (void **)&pSAM_VOICE_GetVoiceUsedCount,   "SAM_VOICE_GetVoiceUsedCount", 0 },
                                { (void **)&pSAM_STREAM_AddData,            "SAM_STREAM_AddData", 0 },
                                { (void **)&pSAM_STREAM_SetState,           "SAM_STREAM_SetState", 0 },
                                { (void **)&pSAM_MUSIC_AddSFX,              "SAM_MUSIC_AddSFX", 0 },
                                { (void **)&pSAM_MUSIC_Delete,              "SAM_MUSIC_Delete", 0 },
                                { (void **)&pSAM_MUSIC_SetLoop,             "SAM_MUSIC_SetLoop", 0 },
                                { (void **)&pSAM_MUSIC_SetJump,             "SAM_MUSIC_SetJump", 0 },
                                { (void **)&pSAM_MUSIC_Play,                "SAM_MUSIC_Play", 0 },
                                { NULL, "" } };

//######################################################################################################################################################
// Les CVAR de IKALIZER

    typedef struct {
        char    szCvarNameOld[64];
        char    szCvarNameNew[64];
        char    szCvarNameNewBisA[64];
        char    szCvarNameNewBisB[64];
        long    lValueString;           //0 = value, 1 = string
        char    szDefaultValue[16];
        void    (*pProc) ( void );
        long    lNeedRestart;
        DWORD   dwMessageToSend;
        char    szUsageHelp[256];
        cvar_t  *cvarVariable;
    } IKA_CVAR_ENTRY;


    void CMD_ikalizerGetValue ( void );
    void CMD_ikalizerGetValueDR ( void );

    #define STR_ikalizer_SamplingRate           "ikalizer_SamplingRate"
    #define STR_ikalizer_ChannelMode            "ikalizer_ChannelMode"
    #define STR_ikalizer_LatencyDuration        "ikalizer_LatencyDuration"
    #define STR_ikalizer_BufferDuration         "ikalizer_BufferDuration"
    #define STR_ikalizer_LimiterLevel           "ikalizer_LimiterLevel"
    #define STR_ikalizer_DeviceSelect           "ikalizer_DeviceSelect"
    #define STR_ikalizer_DynamicDelayLines      "ikalizer_DynamicDelayLines"
    #define STR_ikalizer_MaxUsage               "ikalizer_MaxUsage"
    #define STR_ikalizer_VoiceCount             "ikalizer_VoiceCount"
    #define STR_ikalizer_SFXCount               "ikalizer_SFXCount"
    #define STR_ikalizer_MemoryAlloc            "ikalizer_MemoryAlloc"
    #define STR_ikalizer_EnableSMP              "ikalizer_EnableSMP"
    #define STR_ikalizer_Help                   "ikalizer_Help"
    #define STR_ikalizer_Restart                "ikalizer_Restart"
    #define STR_ikalizer_DeviceEnum             "ikalizer_DeviceEnum"
    #define STR_ikalizer_DistanceRendering      "ikalizer_DistanceRendering"
    //#define STR_ikalizer_ReplaceSamples         "ikalizer_ReplaceSamples"

    IKA_CVAR_ENTRY ikaCvarEntry[] = {
        { "s_samSamplingRate",      STR_ikalizer_SamplingRate,        "ika_SamplingRate",     "ika_SR",   0, "48000", CMD_ikalizerGetValue, 0, SAM_MESSAGE_SAMPLINGRATE_SET,    "%s <value>\nValue = 22050, 24000, 32000, 44100 or 48000(default)\n" },
        { "s_samChannelMode",       STR_ikalizer_ChannelMode,         "ika_ChannelMode",      "ika_CM",   1, "0x21",  CMD_ikalizerGetValue, 0, SAM_MESSAGE_RENDERMODE_SET,      "%s <value>\nValue = 0x20(Stereo), 0x21(Headphones Hybrid-HRTF),\n0x22(360VS), 0x23(DPL-4.0), 0x24(DPLII-5.0),\n0x25(Headphones Holographic), 0x26(Headphones virtual holographic),\n0x40(4.0), 0x60(5.0), 0x61(5.1)\n" },
        { "s_samLatencyDuration",   STR_ikalizer_LatencyDuration,     "ika_LatencyDuration",  "ika_LD",   0, "20",    CMD_ikalizerGetValue, 0, SAM_MESSAGE_LATENCYDURATION_SET, "%s <value>\nValue in milliseconds. Min=5ms, Max=100ms, Default=20ms\n" },
        { "s_samBufferDuration",    STR_ikalizer_BufferDuration,      "ika_BufferDuration",   "ika_BD",   0, "500",   CMD_ikalizerGetValue, 0, SAM_MESSAGE_BUFFERDURATION_SET,  "%s <value>\nValue in milliseconds. Min=100ms Max=500ms, Default=500ms\n" },
        { "s_samLimiterLevel",      STR_ikalizer_LimiterLevel,        "ika_LimiterLevel",     "ika_LL",   0, "0",     CMD_ikalizerGetValue, 0, SAM_MESSAGE_LIMITERLEVEL_SET,    "%s <value>\nValue 0=+0dB, 1=+6dB, 2=+12dB, 3=+20dB\n" },
        { "s_samDeviceSelect",      STR_ikalizer_DeviceSelect,        "ika_DeviceSelect",     "ika_DS",   0, "0",     CMD_ikalizerGetValue, 0, SAM_MESSAGE_DEVICESELECT_SET,    "%s <value>\nValue = value provided by DeviceEnum\n" },
        { "s_samDynamicDelayLines", STR_ikalizer_DynamicDelayLines,   "ika_DynamicDelayLines","ika_DDL",  0, "-1",    CMD_ikalizerGetValue, 0, SAM_MESSAGE_DYNAMICDELAYLINES,   "%s <value>\nValue = -1(best compromise), 0(off), 1...100(quality level)" },
        { "s_samMaxUsage",          STR_ikalizer_MaxUsage,            "ika_MaxUsage",         "ika_MU",   0, "7",     CMD_ikalizerGetValue, 0, SAM_MESSAGE_MAXUSAGE_SET,        "%s <value>\nTotal allowed processor usage, 0=off, 1...30(1%% to 30%%)" },
        { "s_samVoiceCount",        STR_ikalizer_VoiceCount,          "ika_VoiceCount",       "ika_VC",   0, "24",    CMD_ikalizerGetValue, 1, SAM_MESSAGE_UNDEFINED },
        { "s_samSFXCount",          STR_ikalizer_SFXCount,            "ika_SFXCount",         "ika_SC",   0, "4096",  CMD_ikalizerGetValue, 1, SAM_MESSAGE_UNDEFINED },
        { "s_samMemoryAlloc",       STR_ikalizer_MemoryAlloc,         "ika_MemoryAlloc",      "ika_MA",   0, "64",    CMD_ikalizerGetValue, 1, SAM_MESSAGE_UNDEFINED },
        { "s_samEnableSMP",         STR_ikalizer_EnableSMP,           "ika_EnableSMP",        "ika_ES",   0, "0",     CMD_ikalizerGetValue, 1, SAM_MESSAGE_UNDEFINED },
        { "s_samHelp",              STR_ikalizer_Help,                "ika_Help",             "ika_H",    0, "",      S_SAM_Help,           0 },
        { "s_samRestart",           STR_ikalizer_Restart,             "ika_Restart",          "ika_R",    0, "",      S_SAM_Restart,        0 },
        { "s_samDeviceEnum",        STR_ikalizer_DeviceEnum,          "ika_DeviceEnum",       "ika_DE",   0, "",      S_SAM_DeviceEnum,     0 },
        { "s_samDistanceRendering", STR_ikalizer_DistanceRendering,   "ika_DistanceRendering","ika_DR",   0, "2",     CMD_ikalizerGetValueDR, 0, SAM_MESSAGE_UNDEFINED,         "%s <value>\nValue = Distance processing rendering, 0 = initial iKALiZER, 1 = original ioQuake, 2 = more theatre\n" },
        //{ "s_samReplaceSamples",    STR_ikalizer_ReplaceSamples,      "ika_ReplaceSamples",   "ika_RS",   0, "1",     CMD_ikalizerGetValue, 1, SAM_MESSAGE_UNDEFINED,           "%s <value>\nSamples replacement, 0=off, 1=on(default)" },
        { "", "", "", "" },
    };

//cvar_t *Cvar_FindVar( const char *var_name );

SAM_CONFIG  samConfig;
HMODULE     hModuleSamLib = NULL;
DWORD       dwHandleVoiceMusicIntro;
DWORD       dwHandleVoiceMusicLoop;
DWORD       dwHandleSfxMusicIntro;
DWORD       dwHandleSfxMusicLoop;
DWORD       dwHandleSfxDefaultSound;

char        szMusicIntro[512];
char        szMusicLoop[512];

int         SAM_iListenerInWater;
int         SAM_iListenerEntityNumber;
vec3_t          SAM_vec3ListenerOrigin;
vec3_t          SAM_vec3ListenerAxis[3];
float       f32MasterLevelSFX;
float       f32MasterLevelMusic;
DWORD       SAM_dwSupportDeviceSelect;
DWORD       SAM_dwSupportVoiceSyncFreeze;
DWORD       SAM_dwSupportVoiceIsPlayed;
DWORD       SAM_dwSupportVirtualVoicesCount;

char        szSamLibraryNeeds[16];

int         IKALIZER_iDistanceRendering;


void CMD_ikalizerGetValue ( void )
{
        cvar_t  *v;
        long    i;
        long    lFoundEntry;
        char    *pszName;
        DWORD   dwBackValue;
        DWORD   dwSendValue;
        char    szTmp[32];

    //Récupération du nom de la fonction
        pszName = Cmd_Argv ( 0 );

        //Recherche la bonne fonction appelée
        lFoundEntry = -1;
    for (i=0;strlen(ikaCvarEntry[i].szCvarNameNew);i++)
    {
        if ( (Q_stricmp(pszName,ikaCvarEntry[i].szCvarNameOld)==0) ||
             (Q_stricmp(pszName,ikaCvarEntry[i].szCvarNameNew)==0) ||
             (Q_stricmp(pszName,ikaCvarEntry[i].szCvarNameNewBisA)==0) ||
             (Q_stricmp(pszName,ikaCvarEntry[i].szCvarNameNewBisB)==0) )
        {
            lFoundEntry = i;
            break;
        }
    }

    if (lFoundEntry==-1)
    {
        Com_Printf ( "IKALIZER: 'CMD_ikalizerGetValue' failure !!!\n" );
        return;
    }

    //Affichage de l'aide
        if ( Cmd_Argc() != 2 )
        {
                Com_Printf ( ikaCvarEntry[lFoundEntry].szUsageHelp, pszName );
                Com_Printf ( "\n Actual value : %s\n", Cvar_VariableString ( ikaCvarEntry[lFoundEntry].szCvarNameNew ) );
                return;
        }


        if (ikaCvarEntry[lFoundEntry].dwMessageToSend!=SAM_MESSAGE_UNDEFINED)
        {
            if (!ikaCvarEntry[lFoundEntry].lValueString)
                dwSendValue = atoi ( Cmd_Argv(1) );
            else
                dwSendValue = strtol ( Cmd_Argv(1), NULL, 16 );

        if ( (CL_VideoRecording()) &&
             ( (ikaCvarEntry[lFoundEntry].dwMessageToSend == SAM_MESSAGE_SAMPLINGRATE_SET) ||
               (ikaCvarEntry[lFoundEntry].dwMessageToSend == SAM_MESSAGE_RENDERMODE_SET) ) )
        {
            Com_Printf ( S_COLOR_YELLOW"Can't change sampling rate or render mode during Video Recording\n" );
        }
        else
        {
                //Message à envoyer
            pSAM_Message (
                ikaCvarEntry[lFoundEntry].dwMessageToSend,
                dwSendValue,
                (DWORD)&dwBackValue );

            if (!ikaCvarEntry[lFoundEntry].lValueString)
                sprintf ( szTmp, "%d", dwBackValue );
            else
                sprintf ( szTmp, "0x%x", dwBackValue );

            if (ikaCvarEntry[lFoundEntry].dwMessageToSend == SAM_MESSAGE_SAMPLINGRATE_SET)
            {
                samConfig.dwHardwareSamplingRate    = dwBackValue;
                dma.speed                           = samConfig.dwHardwareSamplingRate;
            }
            else if (ikaCvarEntry[lFoundEntry].dwMessageToSend == SAM_MESSAGE_RENDERMODE_SET)
            {
                    samConfig.dwHardwareChannelsMode    = dwBackValue;
                    dma.channels                        = samConfig.dwHardwareChannelsMode&0x00F0;
                }

            Cvar_Set2 ( ikaCvarEntry[lFoundEntry].szCvarNameNew, szTmp, qfalse);
        }
    }
    else strcpy ( szTmp, Cmd_Argv(1) );

    Cvar_Set2 ( ikaCvarEntry[lFoundEntry].szCvarNameNew, szTmp, qfalse);


    Com_Printf ( "IKALIZER: Set %s = %s\n", pszName, szTmp );

    if (ikaCvarEntry[lFoundEntry].lNeedRestart)
        Com_Printf ( S_COLOR_YELLOW"IKALIZER: A restart is needed to apply the new value\n" );
}



void SAM_Intro ( void )
{
    /*
    float   *pSound;
    float   f1;
    float   fAngle;
    long    lAngle;
    float   fDistance;
    float   fLevel;
    float   fFilter;
    long    lIndex;
    UINT32  ui32Random;
    long    lReturn;
    DWORD   dwVoiceHandle;
    DWORD   dwSFXHandle;
    FILE    *pFile;
    fpos_t  fpos;
    float   fPause;

    return;
    ui32Random = 0;

    pFile = fopen ( "C:\\TestPorteMusique.pcm", "rb" );
    fseek ( pFile, 0, SEEK_END );
    fgetpos ( pFile, &fpos );

    pSound = (BYTE *)Z_Malloc ( fpos );

    fseek ( pFile, 0, SEEK_SET );
    fread ( pSound, fpos, 1, pFile );
    fclose ( pFile );

    fPause = (((float)fpos)/48000)*1000;
    fPause += 200;



    dwSFXHandle = 0;

    lReturn = pSAM_SFX_Load (
        dwSFXHandle,
        "Intro",
        48000,
        fpos,
        0,
        8,
        pSound,
        NULL );

    Z_Free ( pSound );

    if (lReturn)
        return;



                    //At last... Play the sound !!!
                    //pSAM_VOICE_SetLoop          ( dwVoiceHandle|SAM_VOICE_FASTACCESS, 1 );
                    //pSAM_VOICE_SetPlay          ( dwVoiceHandle|SAM_VOICE_FASTACCESS, 1 );

    lReturn = pSAM_VOICE_Alloc  ( &dwVoiceHandle, 0, NULL );
    if (!lReturn)
    {
        pSAM_VOICE_SetSFX           ( dwVoiceHandle, dwSFXHandle );
        pSAM_VOICE_SetMasterLevel   ( dwVoiceHandle, -100 );
        //pSAM_VOICE_SetLoop          ( dwVoiceHandle, 1 );

        pSAM_VOICE_SetMasterLevel   ( dwVoiceHandle, -2 );
        pSAM_VOICE_SetDistanceLevel ( dwVoiceHandle, -8 );

        //Level
        S_SAM_SetVolume ( );

        //Apply new Limiter value ?
        pSAM_LimiterSet ( s_samLimiterLevel->integer );

        pSAM_Message ( SAM_MESSAGE_DYNAMICDELAYLINES, s_samDynamicDelayLines->integer, 0 );


        for (lIndex=0;lIndex<=180;lIndex+=90)
        {
            fAngle      = lIndex%360;
            fDistance   = (float)(lIndex+360)/360.0F;
            fDistance   = (float)pow ( (1+fDistance), 3.0F );
            fDistance   = (fDistance*100);

            //L'angle
            lAngle = lIndex%360;
            if (lAngle>=180) lAngle = lAngle-360;


                //Traitement distance => attenuation
                f1 = fDistance + 80.0F;
                f1 *= 0.01F;                //Convertion en metres
                if (f1<1.0F) f1 = 1.0F;     //On considère qu'à moins de 1m, le son reste le même
                f1 = 1.0F / f1;
                fLevel = log10 ( f1 ) * 20;

            f1 = log10 ( fDistance + 1.0F ) * 20.0F;
            fFilter = pow ( 1.0F / ( f1 + 1.0F ), 0.9F ) * 20.0F;
            fFilter = pow ( fFilter, 2.0F ) * 10.0F;
            if (fFilter>1.0F) fFilter = 1.0F;
            if (fFilter<0.1F) fFilter = 0.1F;

            pSAM_Message ( SAM_MESSAGE_VOICE_SYNCFREEZE, 1, 0 );



            //pSAM_VOICE_SetRatioIIR      ( dwVoiceHandle, fFilter );
            pSAM_VOICE_SetPlay          ( dwVoiceHandle|SAM_VOICE_FASTACCESS, 1 );
            pSAM_VOICE_SetOrigin         ( dwVoiceHandle|SAM_VOICE_FASTACCESS, lAngle, 0 );

            pSAM_Message ( SAM_MESSAGE_VOICE_SYNCFREEZE, 0, 0 );

            //pSAM_STREAM_SetState ( 1 );

            Sleep ( fPause );

        }

        pSAM_VOICE_SetPlay ( dwVoiceHandle, 0 );
        pSAM_VOICE_Free    ( dwVoiceHandle );
    }

    pSAM_SFX_Free ( dwSFXHandle );
    */
}






#define LOCKENTITY_CHANNEL      (0xFF00)
#define SINGLEENTITY_CHANNEL    (0xFFFD)
#define LOOPENTITY_CHANNEL      (0xFFFE)

//static SAM_LOOP_ENTITY_INFO samLoopEntityInfo[MAX_GENTITIES];

typedef struct
{
    BYTE        bLoopMode;      //0 = off, 1 = on
    vec3_t      vec3Origin;
    vec3_t      vec3Velocity;
    BYTE        bIsOrigin;
    BYTE        bIsVelocity;
} SAM_VOICE_PARAMS;

typedef struct
{
    BYTE                bIsActive;
    long                bUpdateMode;        //0 = do nothing, 1 = new play, 2 = modify params, 3 = play after respatialize, 4 = new play next frame...

    int                 iEntityNumber;
    int                 iEntityChannel;

    sfxHandle_t         sfxHandle;

    DWORD               dwTimeStarted;

    DWORD               dwVoiceHandle;

    float               fMasterLevel;
    float               fDistanceLevel;
    float               fFilter;
    long                lAngle;
    float               fDistanceMeters;
    float               fSourceSize;

    SAM_VOICE_PARAMS    samVoiceParams;

} SAM_VOICE_SINGLEPLAY_STACK;

typedef struct {
    BYTE                bIsActive;
    BYTE                bIsFrameKill;       //Si 1 ou 2, ce son sera détruit à la prochaine frame
    //BYTE                bIsSphere;
    long                bUpdateMode;        //0 = do nothing, 1=new play, 2=modify params


    int                 iEntityNumber;

    sfxHandle_t         sfxHandle;

    DWORD               dwTimeStarted;
    DWORD               dwTimeLastAccess;
    DWORD               dwVoiceHandle;

    float               fMasterLevel;
    float               fDistanceLevel;
    float               fFilter;
    long                lAngle;
    float               fDistanceMeters;
    float               fSourceSize;

    SAM_VOICE_PARAMS    samVoiceParams;

} SAM_VOICE_LOOP_PLAYSTACK;

#define MAX_SINGLEPLAYSTACK_COUNT   256
SAM_VOICE_SINGLEPLAY_STACK          samVoiceSinglePlayStack[MAX_SINGLEPLAYSTACK_COUNT];
long                                lVoiceSinglePlayStackCount;

#define MAX_LOOP_PLAYSTACK_COUNT    (MAX_GENTITIES)
SAM_VOICE_LOOP_PLAYSTACK            samVoiceLoopPlayStack[MAX_LOOP_PLAYSTACK_COUNT];
long                                lVoiceLoopPlayStackCount;

//Les voix single shoot !
long    SAM_VoiceSPS_Flush ( void );
long    SAM_VoiceSPS_Add ( int iEntityNumber, int iEntityChannel, sfxHandle_t sfxHandle );

//Les voix loop
long    SAM_VoiceLPS_Flush ( void );
long    SAM_VoiceLPS_Add ( int iEntityNumber, sfxHandle_t sfxHandle );
long    SAM_VoiceLPS_AddUpdate ( int iEntityNumber, sfxHandle_t sfxHandle );



/*

    VOICE Loop !

*/
long    SAM_VoiceLPS_Flush ( void )
{
    lVoiceLoopPlayStackCount = MAX_LOOP_PLAYSTACK_COUNT;

    memset (
        samVoiceLoopPlayStack,
        0,
        sizeof(SAM_VOICE_LOOP_PLAYSTACK)*lVoiceLoopPlayStackCount );

    return 0;
}


long    SAM_VoiceLPS_Add ( int iEntityNumber, sfxHandle_t sfxHandle )
{
    SAM_VOICE_LOOP_PLAYSTACK *psamVoiceLPS;
    DWORD dwTime;
    long i;
    long lFoundEntry;
    long lFoundEntryFree;

    dwTime = (DWORD)Com_Milliseconds();

    if (iEntityNumber>=MAX_LOOP_PLAYSTACK_COUNT)
        return -1;

    //Le nombre de voix...
    lVoiceLoopPlayStackCount = MAX_LOOP_PLAYSTACK_COUNT;

    //On détermine l'entrée...
    lFoundEntry = iEntityNumber;

    //Ajoute le son
    psamVoiceLPS                    = &(samVoiceLoopPlayStack[lFoundEntry]);
    memset ( psamVoiceLPS, 0, sizeof(SAM_VOICE_LOOP_PLAYSTACK) );
    psamVoiceLPS->bIsActive             = 1;
    psamVoiceLPS->dwTimeStarted         = dwTime;
    psamVoiceLPS->dwTimeLastAccess      = dwTime;
    psamVoiceLPS->iEntityNumber         = iEntityNumber;
    psamVoiceLPS->sfxHandle             = sfxHandle;
    psamVoiceLPS->dwVoiceHandle         = 0xFFFFFFFF;
    //psamVoiceLPS->bUpdatePositionDup    = 1;

    //Renvoie le numéro de l'entrée du son
    return lFoundEntry;
}

long    SAM_VoiceLPS_AddUpdate ( int iEntityNumber, sfxHandle_t sfxHandle )
{
    SAM_VOICE_LOOP_PLAYSTACK *psamVoiceLPS;
    DWORD dwTime;
    long i;
    long lFoundEntry;
    long lFoundEntryFree;
    long lUpdateOnly;

    dwTime = (DWORD)Com_Milliseconds();

    if (iEntityNumber>=MAX_LOOP_PLAYSTACK_COUNT)
        return -1;

    //Le nombre de voix...
    lVoiceLoopPlayStackCount = MAX_LOOP_PLAYSTACK_COUNT;

    //On détermine l'entrée...
    lFoundEntry = iEntityNumber;

    //Ajoute le son
    psamVoiceLPS = &(samVoiceLoopPlayStack[lFoundEntry]);

    if ( (psamVoiceLPS->bIsActive) && (iEntityNumber==iEntityNumber) ) lUpdateOnly = 1;
    else                                                               lUpdateOnly = 0;
    if (!lUpdateOnly) memset ( psamVoiceLPS, 0, sizeof(SAM_VOICE_LOOP_PLAYSTACK) );
    psamVoiceLPS->bIsActive             = 1;
    psamVoiceLPS->dwTimeStarted         = (!lUpdateOnly)?(dwTime):(psamVoiceLPS->dwTimeStarted);
    psamVoiceLPS->dwTimeLastAccess      = dwTime;
    psamVoiceLPS->iEntityNumber         = iEntityNumber;
    psamVoiceLPS->sfxHandle             = sfxHandle;
    psamVoiceLPS->bUpdateMode           = (lUpdateOnly)?(2):(1);
    psamVoiceLPS->dwVoiceHandle         = (!lUpdateOnly)?(0xFFFFFFFF):(psamVoiceLPS->dwVoiceHandle);

    //Renvoie le numéro de l'entrée du son
    return lFoundEntry;
}


/*

    VOICE Single !

*/
long    SAM_VoiceSPS_Flush ( void )
{
    lVoiceSinglePlayStackCount = 0;
    return 0;
}

long    SAM_VoiceSPS_Add ( int iEntityNumber, int iEntityChannel, sfxHandle_t sfxHandle )
{
    SAM_VOICE_SINGLEPLAY_STACK *psamVoiceSPS;
    DWORD dwTime;
    long i;
    long lFoundEntry;

        dwTime = (DWORD)Com_Milliseconds();

        //On recherche un son déjà identique...
        lFoundEntry = -1;
        psamVoiceSPS = samVoiceSinglePlayStack;
        for (i=0;i<lVoiceSinglePlayStackCount;i++,psamVoiceSPS++)
        {
            if ( (psamVoiceSPS->bIsActive) &&
                 (psamVoiceSPS->iEntityNumber==iEntityNumber) &&
                 (psamVoiceSPS->sfxHandle==sfxHandle) &&
                 ((dwTime - psamVoiceSPS->dwTimeStarted)<50) )
                return -1; //Son déjà démarré il y a moins de 50ms...

            //On recherche en même temps le premier emplacement libre
            if (!psamVoiceSPS->bIsActive)
                lFoundEntry = i;
    }

    //On n'a pas trouver d'emplacement vide, reste-t-il de la place à la fin ?
    if (lFoundEntry==-1)
    {
        if (lVoiceSinglePlayStackCount>=MAX_SINGLEPLAYSTACK_COUNT)
            return -2;

        lFoundEntry = lVoiceSinglePlayStackCount;
        lVoiceSinglePlayStackCount += 1;
    }

    //Ajoute le son
    psamVoiceSPS                    = &(samVoiceSinglePlayStack[lFoundEntry]);
    memset ( psamVoiceSPS, 0, sizeof(SAM_VOICE_SINGLEPLAY_STACK) );
    psamVoiceSPS->bIsActive         = 1;
    psamVoiceSPS->dwTimeStarted     = dwTime;
    psamVoiceSPS->iEntityNumber     = iEntityNumber;
    psamVoiceSPS->iEntityChannel    = iEntityChannel;
    psamVoiceSPS->sfxHandle         = sfxHandle;

    //Com_Printf( "  VoiceSPS => Add = %d\n", lFoundEntry );

    //Renvoie le numéro de l'entrée du son
    return lFoundEntry;
}


void CMD_ikalizerGetValueDR ( void )
{
    CMD_ikalizerGetValue ( );

    IKALIZER_iDistanceRendering = Cvar_VariableIntegerValue ( STR_ikalizer_DistanceRendering );
}



/*
=================
S_SAM_ErrorMessage
=================
*/
const char *S_SAM_ErrorMessage ( long lError )
{
        switch(lError)
        {
/*              case SAM_NO_ERROR:
                        return "No error";
                case SAM_INVALID_NAME:
                        return "Invalid name";
                case SAM_INVALID_ENUM:
                        return "Invalid enumerator";
                case SAM_INVALID_VALUE:
                        return "Invalid value";
                case SAM_INVALID_OPERATION:
                        return "Invalid operation";
                case SAM_OUT_OF_MEMORY:
                        return "Out of memory";*/
                default:
                        return "Unknown error";
        }
}

void S_SAM_Restart ( void )
{

/*
    qboolean    bInit;
    char        *pszSFXName;
    DWORD       dwIndex;
    char        *pszMusicIntro;
    char        *pszMusicLoop;
    char        szMusicIntroFileName[512];
    char        szMusicLoopFileName[512];

    Com_Printf ( "IKALIZER_Restart...\n" );
    Com_Printf ( S_COLOR_YELLOW" WARNING : This command line has been disabled.\n" );
    return;

    //Save all sounds
    Com_Printf ( "  Save all sfx..." );
    pszSFXName = (char *)Z_Malloc ( 256 * samConfig.dwSoftwareSFXCount );
    for (dwIndex=1;dwIndex<samConfig.dwSoftwareSFXCount;dwIndex++)
    {
        pszSFXName[dwIndex<<8] = 0;
        pSAM_SFX_GetName ( dwIndex, &pszSFXName[dwIndex<<8] );
    }
    Com_Printf ( "Ok\n" );

    //Close SAM
    Com_Printf ( "  Close IKALIZER..." );
    pSAM_Close ( );
    Com_Printf ( "Ok\n" );

    //Set new vars
    samConfig.dwHardwareSamplingRate            = s_samSamplingRate->integer;
    samConfig.dwHardwareChannelsMode            = strtol ( s_samChannelMode->string, NULL, 16 );
    samConfig.dwHardwareBufferLatencyDuration   = s_samLatencyDuration->integer;
    samConfig.dwSoftwareBufferDuration          = s_samBufferDuration->integer;
    samConfig.dwSoftwareVoicesCount             = s_samVoiceCount->integer;
    samConfig.dwTotalMemorySoundAllocCount      = s_samMemoryAlloc->integer;
    samConfig.dwProcessFlag                     = 0;
    samConfig.dwHardwareDeviceSelected          = s_samDeviceSelect->integer;

    //Pas de support de sélection du périphérique
    if (!SAM_dwSupportDeviceSelect)
        samConfig.dwHardwareDeviceSelected = 0;

    //Gestion du multiCPU avec ioUrbanTerror sur le CPU #0 et SAM sur les autres...
    if (s_samEnableSMP->integer)
        samConfig.dwProcessFlag |= SAM_PROCESS_ENABLESMP;

    Com_Printf ( "  Open IKALIZER..." );
    bInit = pSAM_Open ( NULL, &samConfig );
        if (bInit)
        {
            Z_Free ( pszSFXName );
        Com_Error( ERR_FATAL, "Error\n" );
        return;
    }
    Com_Printf ( "Ok\n" );

    //Refixe le volume
    S_SAM_SetVolume ( );

    //Vide les différents canaux
    //lChannelSinglePlayCount     = 0;
    lVoiceSinglePlayStackCount  = 0;
    lVoiceLoopPlayStackCount    = 0;
    SAM_VoiceLPS_Flush ( );
    //memset ( samLoopEntityInfo, 0, sizeof(SAM_LOOP_ENTITY_INFO)*MAX_GENTITIES );

    //Charge le son 0 par défaut
    S_SAM_BeginRegistration ( );

    //Recharge tous les sons
    Com_Printf ( "  Load all sfx..." );
    for (dwIndex=1;dwIndex<samConfig.dwSoftwareSFXCount;dwIndex++)
    {
        if (pszSFXName[dwIndex<<8])
            S_SAM_LoadSound ( &pszSFXName[dwIndex<<8], dwIndex );
    }
    Z_Free ( pszSFXName );
    Com_Printf ( "Ok\n" );

    //Recharge la musique
    Com_Printf ( "  Load music..." );
    dwHandleVoiceMusicIntro     = 0xFFFFFFFF;
    dwHandleVoiceMusicLoop      = 0xFFFFFFFF;
    dwHandleSfxMusicIntro       = 0xFFFFFFFF;
    dwHandleSfxMusicLoop        = 0xFFFFFFFF;
    pszMusicIntro = NULL;
    pszMusicLoop  = NULL;
    strcpy ( szMusicIntroFileName, szMusicIntro );
    strcpy ( szMusicLoopFileName, szMusicLoop );
    if (strlen(szMusicIntroFileName)) pszMusicIntro = szMusicIntroFileName;
    if (strlen(szMusicLoopFileName))  pszMusicLoop  = szMusicLoopFileName;
    S_SAM_StartBackgroundTrack ( pszMusicIntro, pszMusicLoop );
    Com_Printf ( "Ok\n" );

    Com_Printf ( "IKALIZER_Restart complete.\n" );
    */
}

/*
#######################################
S_SAM_Init
#######################################
*/





qboolean S_SAM_Init ( soundInterface_t * psi )
{
    qboolean    bInit;
    long        i;
    char        szLibrary[256];
    char        szLibraryA[256];
    char        szLibraryB[256];
    char        szTmp1[256], szTmp2[256], * psz;
    DWORD       dwVersion, dwVersionMini;
    long        lVersionL, lVersionM, lVersionH;
    cvar_t      *s_samVariable;
    long        lReturn, lValue;

        if (!psi)
        return qfalse;

    strcpy ( szLibraryA, "ikalizer.aei" );
    strcpy ( szLibraryB, "samlib.sae" );

    for (i=0;strlen(ikaCvarEntry[i].szCvarNameNew);i++)
    {
        //Initialisation de la variable
        if (strlen(ikaCvarEntry[i].szDefaultValue))
            ikaCvarEntry[i].cvarVariable = Cvar_Get ( ikaCvarEntry[i].szCvarNameNew, ikaCvarEntry[i].szDefaultValue, CVAR_ARCHIVE );

        //La procédure d'appel
        if (ikaCvarEntry[i].pProc)
        {
            //La variable de base
            Cmd_AddCommand ( ikaCvarEntry[i].szCvarNameNew, ikaCvarEntry[i].pProc );

            //La variable bis A
            Cmd_AddCommand ( ikaCvarEntry[i].szCvarNameNewBisA, ikaCvarEntry[i].pProc );

            //La variable bis B
            Cmd_AddCommand ( ikaCvarEntry[i].szCvarNameNewBisB, ikaCvarEntry[i].pProc );

            //L'ancienne variable
            if (strlen(ikaCvarEntry[i].szCvarNameOld))
                Cmd_AddCommand ( ikaCvarEntry[i].szCvarNameOld, ikaCvarEntry[i].pProc );
        }

        //Existe-t-il une ancienne variable à porter ?
        if (strlen(ikaCvarEntry[i].szCvarNameOld))
        {
            lReturn = Cvar_Flags ( ikaCvarEntry[i].szCvarNameOld );
            if (lReturn!=CVAR_NONEXISTENT)
            {
                if (ikaCvarEntry[i].lValueString==0) //0 = value
                {
                    lValue = Cvar_VariableIntegerValue ( ikaCvarEntry[i].szCvarNameOld );
                    Cvar_SetValue ( ikaCvarEntry[i].szCvarNameNew, lValue );
                }
                else                                 //1 = string
                {
                    psz = Cvar_VariableString ( ikaCvarEntry[i].szCvarNameOld );
                    Cvar_Set ( ikaCvarEntry[i].szCvarNameNew, psz );
                }
                //Cvar_ForceReset ( ikaCvarEntry[i].szCvarNameOld );
                //Cvar_Set2
                s_samVariable = Cvar_Set2 ( ikaCvarEntry[i].szCvarNameOld, NULL, qtrue);
                s_samVariable->flags = CVAR_TEMP;
            }
        }
    }

    strcpy ( szLibrary, szLibraryA );
    hModuleSamLib = LoadLibrary ( szLibraryA );
    if (!hModuleSamLib)
    {
        strcpy ( szLibrary, szLibraryB );
        hModuleSamLib = LoadLibrary ( szLibraryB );
        if (!hModuleSamLib)
        {
            Com_Printf( "IKALIZER : Cannot load '%s' or '%s'.\n", szLibraryA, szLibraryB );
            return qfalse;
        }
    }

    for (i=0;samProcList[i].pProcAdress;i++)
    {
        (*samProcList[i].pProcAdress) = NULL;
        (*samProcList[i].pProcAdress) = (void *)GetProcAddress ( hModuleSamLib, samProcList[i].szProcName );
        if (!(*samProcList[i].pProcAdress))
        {
            Com_Printf( "IKALIZER : Cannot find '%s' from '%s'.\n", samProcList[i].szProcName, szLibrary );
            if (!samProcList[i].lOptionnal)
                return qfalse;
            else
            {
                Com_Printf( "IKALIZER : '%s' function has been bypassed.\n", samProcList[i].szProcName );
                (*samProcList[i].pProcAdress) = NULL;
            }
        }
    }

    //Récupération des capacités de SAM
    SAM_dwSupportDeviceSelect       = 0;
    pSAM_GetInfo (
        SAM_INFO_SUPPORT_SELECT_DEVICE,
        &SAM_dwSupportDeviceSelect,
        NULL );
    //Com_Printf( "  SAM : SupportDeviceSelect : %d\n", SAM_dwSupportDeviceSelect );

    SAM_dwSupportVoiceSyncFreeze    = 0;
    pSAM_GetInfo (
        SAM_INFO_SUPPORT_MESSAGE_VOICE_SYNCFREEZE,
        &SAM_dwSupportVoiceSyncFreeze,
        NULL );
    //Com_Printf( "  SAM : SupportVoiceSyncFreeze : %d\n", SAM_dwSupportVoiceSyncFreeze );

    SAM_dwSupportVoiceIsPlayed      = 0;
    pSAM_GetInfo (
        SAM_INFO_SUPPORT_MESSAGE_VOICE_ISPLAYED,
        &SAM_dwSupportVoiceIsPlayed,
        NULL );
    //Com_Printf( "  SAM : SupportVoiceIsPlayed : %d\n", SAM_dwSupportVoiceIsPlayed );

    //Récupération de la version de SAM
    pSAM_GetInfo (
        SAM_INFO_VERSION_DATA,
        &dwVersion,
        NULL );

    lVersionL = dwVersion&0xFFFF;
    lVersionM = (dwVersion>>16)&0xFF;
    lVersionH = (dwVersion>>24)&0xFF;
    sprintf ( szTmp1, "%d.%d.%d", lVersionH, lVersionM, lVersionL );

    //                HHMMLLLL
    //dwVersionMini = 0x0015000A;
    dwVersionMini = 0x00160000;
    lVersionL = dwVersionMini&0xFFFF;
    lVersionM = (dwVersionMini>>16)&0xFF;
    lVersionH = (dwVersionMini>>24)&0xFF;
    sprintf ( szTmp2, "%d.%d.%d", lVersionH, lVersionM, lVersionL );
    strcpy ( szSamLibraryNeeds, szTmp2 );

    if (dwVersion<dwVersionMini)
    {
        Com_Printf( S_COLOR_YELLOW"IKALIZER : The library is too older (%s). Install a new version (>= %s)\n", szTmp1, szTmp2 );
        return qfalse;
    }



    samConfig.dwHardwareSamplingRate            = Cvar_VariableIntegerValue ( STR_ikalizer_SamplingRate );
    samConfig.dwHardwareChannelsMode            = strtol ( Cvar_VariableString ( STR_ikalizer_ChannelMode ), NULL, 16 );
    samConfig.dwHardwareBufferLatencyDuration   = Cvar_VariableIntegerValue ( STR_ikalizer_LatencyDuration );
    samConfig.dwSoftwareBufferDuration          = Cvar_VariableIntegerValue ( STR_ikalizer_BufferDuration );
    samConfig.dwSoftwareVoicesCount             = Cvar_VariableIntegerValue ( STR_ikalizer_VoiceCount );
    samConfig.dwSoftwareSFXCount                = 4096; //s_samSFXCount->integer;
    samConfig.dwTotalMemorySoundAllocCount      = Cvar_VariableIntegerValue ( STR_ikalizer_MemoryAlloc );
    samConfig.dwStreamingBufferDuration         = 1000;
    samConfig.dwProcessFlag                     = 0;
    samConfig.dwHardwareDeviceSelected          = Cvar_VariableIntegerValue ( STR_ikalizer_DeviceSelect );
    IKALIZER_iDistanceRendering                 = Cvar_VariableIntegerValue ( STR_ikalizer_DistanceRendering );
    //if (samConfig.dwSoftwareSFXCount>1024) samConfig.dwSoftwareSFXCount = 1024;

    //Pas de support de sélection du périphérique
    if (!SAM_dwSupportDeviceSelect)
        samConfig.dwHardwareDeviceSelected = 0;

    //Gestion du multiCPU avec ioUrbanTerror sur le CPU #0 et SAM sur les autres...
    switch (Cvar_VariableIntegerValue ( STR_ikalizer_EnableSMP ) )
    {
        case 0:
            //Laisse le système gérer les tâches
            break;

        case 1:
            //Demande à SAM de passer l'appelant sur le CPU0 et SAM sur le CPU1
            samConfig.dwProcessFlag |= 1;
            break;

        case 2:
            //Demande à SAM la fixation de l'affinité sur le CPU0
            samConfig.dwProcessFlag |= 2;
            break;
    }


    bInit = pSAM_Open ( NULL/*(void *)&g_wv.hWnd*/, &samConfig );
        if (bInit)
    {
        Com_Printf( "IKALIZER : Failed to start.\n" );
        Com_Printf( "  Sampling rate : %d Hz\n", samConfig.dwHardwareSamplingRate );
        Com_Printf( "  Buffer : %d ms\n", samConfig.dwSoftwareBufferDuration );
        Com_Printf( "  Latency : %d ms\n", samConfig.dwHardwareBufferLatencyDuration );
        Com_Printf( "  Channel mode : 0x%02x (0x%02x)\n", samConfig.dwHardwareChannelsMode, Cvar_VariableIntegerValue ( STR_ikalizer_ChannelMode ) );
        Com_Printf( "  SFX count : %d\n", samConfig.dwSoftwareSFXCount );
        Com_Printf( "  Voice count : %d\n", samConfig.dwSoftwareVoicesCount );
        Com_Printf( "  Memory : %d MiB\n", samConfig.dwTotalMemorySoundAllocCount );
        Com_Printf( "  Device : %d\n", samConfig.dwHardwareDeviceSelected );
        return qfalse;
    }

    pSAM_LimiterSet ( Cvar_VariableIntegerValue ( STR_ikalizer_LimiterLevel ) );
    if (pSAM_Message)
        pSAM_Message ( SAM_MESSAGE_DYNAMICDELAYLINES, Cvar_VariableIntegerValue ( STR_ikalizer_DynamicDelayLines ), 0 );


    SAM_dwSupportVirtualVoicesCount = samConfig.dwSoftwareVoicesCount;
    pSAM_Message ( SAM_MESSAGE_VOICE_VIRTUALCOUNT, (DWORD)&SAM_dwSupportVirtualVoicesCount, 0 );

    pSAM_Message ( SAM_MESSAGE_MAXUSAGE_SET, Cvar_VariableIntegerValue ( STR_ikalizer_MaxUsage ), 0 );


    //N = not implemented
    //W = write needed
    //P = partial
    //F = finished
    //U = untested

        psi->Shutdown                   = S_SAM_Shutdown;                   //F
        psi->RegisterSound              = S_SAM_RegisterSound;              //F

        psi->SoundInfo                  = S_SAM_SoundInfo;                  //F
        psi->SoundList                  = S_SAM_SoundList;                  //F

        psi->StartSound                 = S_SAM_StartSound;                 //F
        psi->StartLocalSound            = S_SAM_StartLocalSound;            //F
        psi->StartBackgroundTrack       = S_SAM_StartBackgroundTrack;       //F
        psi->StopBackgroundTrack        = S_SAM_StopBackgroundTrack;        //F
        psi->RawSamples                 = S_SAM_RawSamples;                 //F
        psi->StopAllSounds              = S_SAM_StopAllSounds;              //F
        psi->ClearLoopingSounds         = S_SAM_ClearLoopingSounds;         //F
        psi->AddLoopingSound            = S_SAM_AddLoopingSound;            //F
        psi->AddRealLoopingSound        = S_SAM_AddRealLoopingSound;        //F
        psi->StopLoopingSound           = S_SAM_StopLoopingSound;           //F
        psi->Respatialize               = S_SAM_Respatialize;               //F
        psi->UpdateEntityPosition       = S_SAM_UpdateEntityPosition;       //F
        psi->Update                     = S_SAM_Update;                     //F
        psi->DisableSounds              = S_SAM_DisableSounds;              //F
        psi->BeginRegistration          = S_SAM_BeginRegistration;          //F

        psi->ClearSoundBuffer           = S_SAM_ClearSoundBuffer;           //F

        //ioq3-urt, voip placeholders:
        #ifdef USE_VOIP
                psi->StartCapture                               = S_SAM_StartCapture;
                psi->AvailableCaptureSamples            = S_SAM_AvailableCaptureSamples;
                psi->Capture                                            = S_SAM_Capture;
                psi->StopCapture                                        = S_SAM_StopCapture;
                psi->MasterGain                                 = S_SAM_MasterGain;
        #endif

    dwHandleVoiceMusicIntro         = 0xFFFFFFFF;
    dwHandleVoiceMusicLoop          = 0xFFFFFFFF;
    dwHandleSfxMusicIntro           = 0xFFFFFFFF;
    dwHandleSfxMusicLoop            = 0xFFFFFFFF;

    f32MasterLevelSFX               = -120;
    f32MasterLevelMusic             = -120;

    dma.speed                       = samConfig.dwHardwareSamplingRate;
    dma.channels                    = (samConfig.dwHardwareChannelsMode&0x00F0)>>4;
    dma.samplebits                  = 16;


    //Le gain global
    pSAM_GainSet ( 7, 0.0F, 0.0F, 0.0F );

    lVoiceSinglePlayStackCount  = 0;
    lVoiceLoopPlayStackCount    = 0;
    SAM_VoiceLPS_Flush ( );
    SAM_iListenerInWater        = 0;
    SAM_iListenerEntityNumber   = 0;


    //Charge le son 0 par défaut
    S_SAM_BeginRegistration ( );

    //Online command
    Cmd_RemoveCommand( "snd_restart" );

    S_SAM_SetVolume ( );

    SAM_Intro ( );

        return qtrue;
}

/*

    Fixe le volume

*/
void S_SAM_SetVolume ( void )
{
    float fLevelSFX;
    float fLevelMusic;

    fLevelSFX   = s_volume->value;
    fLevelMusic = s_musicVolume->value;

    fLevelSFX   *= 0.5F;
    fLevelMusic *= 0.5F;

    //Com_Printf( "SFX=%f:\n", fLevelSFX );

    if (fLevelSFX  >1.0F) fLevelSFX   = 1.0F;
    if (fLevelMusic>1.0F) fLevelMusic = 1.0F;
    if (fLevelSFX  <0.0F) fLevelSFX   = 0.0F;
    if (fLevelMusic<0.0F) fLevelMusic = 0.0F;

    //Com_Printf( "SFX=%f:\n", fLevelSFX );

    f32MasterLevelSFX     = (float)log10 ( fLevelSFX  +0.000001F ) * 40;
    f32MasterLevelMusic   = (float)log10 ( fLevelMusic+0.000001F ) * 40;

    f32MasterLevelSFX       +=  SAM_DEFAULTMASTERLEVEL;
    f32MasterLevelMusic     +=  SAM_DEFAULTMASTERLEVEL;

    pSAM_GainSet (
        7,
        pow ( 10, (f32MasterLevelSFX*0.05) ),
        pow ( 10, (f32MasterLevelMusic*0.05) ),
        pow ( 10, (f32MasterLevelMusic*0.05) ) );

}




/*
#######################################
S_SAM_Shutdown

End of SAM. Bye. See you next time !
#######################################
*/
void S_SAM_Shutdown ( void )
{
        long i;
        if (!hModuleSamLib)
                return;

        pSAM_Close ( );

        FreeLibrary ( hModuleSamLib );
        hModuleSamLib = NULL;


        //Suppression des commandes
    for (i=0;strlen(ikaCvarEntry[i].szCvarNameNew);i++)
    {
        //La procédure d'appel
        if (ikaCvarEntry[i].pProc)
        {
            //La variable de base
            Cmd_RemoveCommand ( ikaCvarEntry[i].szCvarNameNew );

            //La variable bis A
            Cmd_RemoveCommand ( ikaCvarEntry[i].szCvarNameNewBisA );

            //La variable bis B
            Cmd_RemoveCommand ( ikaCvarEntry[i].szCvarNameNewBisB );

            //L'ancienne variable
            if (strlen(ikaCvarEntry[i].szCvarNameOld))
                Cmd_RemoveCommand ( ikaCvarEntry[i].szCvarNameOld );

            }
        }
}

/*
#######################################
S_SAM_SoundList

Display current played samples
#######################################
*/

void S_SAM_SoundList ( void )
{
    Com_Printf( "IKALIZER - Surround Audio Mixer list:\n" );
}

/*
#######################################
S_SAM_SoundInfo
#######################################
*/
void S_SAM_SoundInfo ( void )
{
    char szTmp[256];
    DWORD dwData1;
    DWORD dwData2;
    DWORD dwData3;

    szTmp[0] = 0;

    Com_Printf( "ioq3-urt with IKALIZER audio engine : \n" );
    Com_Printf( "  Engine version : "Q3_VERSION"\n" );
    Com_Printf( "  ioq3-urt IKALIZER mod : "__TIMESTAMP__"\n" );
    Com_Printf( "  IKALIZER lib needs : v%s\n\n", szSamLibraryNeeds );

    pSAM_GetInfo ( SAM_INFO_TITLE, NULL, szTmp );
        Com_Printf( "Audio engine information : %s\n", szTmp );

        pSAM_GetInfo ( SAM_INFO_VERSION, NULL, szTmp );
        Com_Printf( "  Driver version : %s\n", szTmp );

    pSAM_GetInfo ( SAM_INFO_VENDOR, NULL, szTmp );
    Com_Printf( "  Vendor : %s\n", szTmp );

    pSAM_GetInfo ( SAM_INFO_COPYRIGHT, NULL, szTmp );
    Com_Printf( "  Copyright : %s\n", szTmp );

    pSAM_GetInfo ( SAM_INFO_SYSTEM_SIMD, NULL, szTmp );
    Com_Printf( "  %s\n", szTmp );

    pSAM_GetInfo ( SAM_INFO_SYSTEM_INSTANCES, &dwData1, NULL );
    Com_Printf( "  SYSTEM processor count : %d\n", dwData1 );

    switch (samConfig.dwProcessFlag&SAM_PROCESS_ENABLESMP)
    {
        case 0:
            strcpy ( szTmp, "Keep system" );
            break;

        case 1:
            strcpy ( szTmp, "Force Dual-core" );
            break;

        case 2:
            strcpy ( szTmp, "Force Single-core" );
            break;
    }
    Com_Printf( "  SYSTEM SMP Usage (multi-core/multi-cpu) : %s\n", szTmp );

    pSAM_GetInfo ( SAM_INFO_MEMORY_TOTAL, &dwData1, NULL );
    Com_Printf( "  MEMORY total allocated : %d KiB\n", dwData1/1024 );

    pSAM_GetInfo ( SAM_INFO_MEMORY_SFXTOTAL, &dwData1, NULL );
    pSAM_GetInfo ( SAM_INFO_MEMORY_SFXFREE,  &dwData2, NULL );
    pSAM_GetInfo ( SAM_INFO_MEMORY_SFXUSED,  &dwData3, NULL );
    Com_Printf( "  SFX total / free / used memory : %d KiB / %d KiB / %d KiB\n", dwData1/1024, dwData2/1024, dwData3/1024 );


    Com_Printf( "  MIXER sampling rate : %d Hz\n", samConfig.dwHardwareSamplingRate );
        Com_Printf( "  MIXER buffer / latency : %d ms / %d ms \n", samConfig.dwSoftwareBufferDuration, samConfig.dwHardwareBufferLatencyDuration );

    pSAM_Message ( SAM_MESSAGE_LATENCYDURATION_BU_GET, (DWORD)&dwData1, (DWORD)&dwData2 );
    Com_Printf( "  MIXER current latency : %4.2f ms (B/U:%d) \n", (((float)dwData1)*1000)/(float)samConfig.dwHardwareSamplingRate, dwData2 );

        pSAM_GetInfo ( SAM_INFO_RENDER_MODE, &dwData1, szTmp );
        Com_Printf( "  MIXER render mode : 0x%02x (%s)\n", dwData1, szTmp );
        //Com_Printf( "  MIXER render name : %s\n", szTmp );

        Com_Printf( "  VOICE total : %d\n", samConfig.dwSoftwareVoicesCount );



    dma.samplebits                  = 16;

        if (SAM_dwSupportDeviceSelect)
        {
            pSAM_GetInfo ( SAM_INFO_DEVICE_GETCURRENT, &dwData1, szTmp );
            Com_Printf( "  Device info : %d - %s\n", dwData1, szTmp );
        }
        else
        {
            Com_Printf( "  Device info : " );
            Com_Printf( S_COLOR_YELLOW "This function is not supported with this older SAM library !\n" );
        }

    pSAM_GetInfo ( SAM_INFO_DEVICE_OUTPUT_MODEL, NULL, szTmp );
    Com_Printf( "  Device output model : %s\n", szTmp );

    pSAM_GetInfo ( SAM_INFO_DEVICE_OUTPUT_FLAGS, NULL, szTmp );
    Com_Printf( "  Device output flags : %s\n", szTmp );


}

/*
#######################################
S_SAM_RawSamples

Add Raw samples to current audio
Sample count reach max MAX_RAW_SAMPLES
#######################################
*/
void S_SAM_RawSamples ( int stream, int iSamples, int iSampleRate, int iWidth, int iChannelCount, const byte * pbData, float f32VolumeMul )
{
    pSAM_STREAM_AddData (
        iSampleRate,
        iSamples,
        (iChannelCount==2)?(1):(0),
        iWidth*8,
        pbData,
        f32VolumeMul );

    //Com_Printf ( "RawSamples. %dHz - %dbit - %dCh - %f Vol - %d samples\n", iSampleRate, iWidth*8, iChannelCount, f32VolumeMul, iSamples );
}

/*
#######################################
S_SAM_BeginRegistration

Unknow usage ???
#######################################
*/
void S_SAM_BeginRegistration( void )
{
        snd_info_t      info;
        long        lReturn;
        DWORD       dwFoundHandle;
        byte        *pbData;
        char        szFileName[128];
        float       fValue;
        float       fLevel, fReEqLevel;
        long        i;
        long        lev;

        strcpy ( szFileName, "sound/feedback/hit.wav" );

    //Searching for the sample filename
    if (!pSAM_SFX_IsLoaded ( szFileName, &dwFoundHandle ))
    {
        dwHandleSfxDefaultSound = dwFoundHandle;
        return;
    }

    // Load sound
    fReEqLevel = 0;
    pbData = IKA_QualitySound_Load ( szFileName, &info, &fReEqLevel );
    if (!pbData) pbData = S_CodecLoad ( szFileName, &info);
        if (!pbData)
        {
            //Génération d'un son par défaut si on ne trouve pas le bon !
            info.rate       = 11025;
            info.samples    = 2000;
            info.width      = 1;
            info.channels   = 1;
            pbData          = Z_Malloc ( info.samples );
            fLevel          = 128.0F;
            for (i=0;i<info.samples;i++)
            {
                fValue = (float)((i&63)-32);
                fValue = fValue * 0.03125F;
                fValue *= fLevel;
                fValue += 128;
                if (fValue>255) fValue = 255;
                if (fValue<  0) fValue = 0;
                pbData[i] = fValue;
                fLevel *= 0.997F;
            }
        }

        // Copy sound in the memory
    pSAM_SFX_Load (
        0,     //Get a new handle
        szFileName,
        info.rate,
        info.samples,
        (info.channels==1)?(0):(1),
        IKA_SampleFormatWidthToBits(info.width),
        pbData,
        &dwFoundHandle );

    pSAM_Message ( SAM_MESSAGE_SFX_DEFAULTLEVEL_SET, dwFoundHandle, (DWORD)&fReEqLevel );

    Z_Free(pbData);

    dwHandleSfxDefaultSound = dwFoundHandle;
}

long IKA_SampleFormatWidthToBits ( long lWidth )
{
    switch (lWidth)
    {
        case 1:     return 8;
        case 2:     return 16;
        case 4:     return 32;
    }
    return 8;
}

/*
#######################################
S_SAM_RegisterSound
#######################################
*/
void S_SAM_LoadSound ( char * pszFileName, DWORD dwHandle )
{
        snd_info_t      info;
        byte        *pbData;
        float       fReEqLevel;

        if (!pszFileName)
            return;

    // Load sound
    fReEqLevel = 0;
    pbData = IKA_QualitySound_Load ( pszFileName, &info, &fReEqLevel );
    if (!pbData) pbData = S_CodecLoad ( pszFileName, &info );
        if (!pbData)
        return;





    pSAM_SFX_Load (
        dwHandle,
        pszFileName,
        info.rate,
        info.samples,
        (info.channels==1)?(0):(1),
        IKA_SampleFormatWidthToBits(info.width),
        pbData,
        &dwHandle );

    pSAM_Message ( SAM_MESSAGE_SFX_DEFAULTLEVEL_SET, dwHandle, (DWORD)&fReEqLevel );

    Z_Free(pbData);
}


//#define REGISTERSOUND_INFO
sfxHandle_t S_SAM_RegisterSound ( const char * pszFileName, qboolean bCompressed )
{
        snd_info_t      info;
        long        lReturn, lFin;
        DWORD       dwFoundKillHandle;
        DWORD       dwReservedListHandle[16];
        byte                    *pbData;

        DWORD       dwHandle_FoundLoaded;
        DWORD       dwLoadedMemoryState;
        DWORD       dwLoadMode;
        char        szTmp[256];
        float       fReEqLevel;

        //return dwHandleSfxDefaultSound;
#ifdef REGISTERSOUND_INFO
        //Com_Printf ( "RegisterSound (In) : %s\n", pszFileName );
#endif
        if (!pszFileName)
            return 0;

    //Searching for the sample filename
    dwHandle_FoundLoaded = 0xFFFFFFFF;
    dwLoadedMemoryState  = 0;

    //Le SFX est ou a été chargé ?
    lReturn = pSAM_SFX_IsLoaded ( pszFileName, &dwHandle_FoundLoaded );
#ifdef REGISTERSOUND_INFO
    Com_Printf (S_COLOR_YELLOW "%d %s\n", lReturn, pszFileName);
#endif
    if (!lReturn)
    {
        //Le SFX utilise de la mémoire ?
        pSAM_SFX_GetLoadedMemoryState ( dwHandle_FoundLoaded, &dwLoadedMemoryState );

        //Le SFX est présent, mais il n'y a plus rien en mémoire
        if (dwLoadedMemoryState==0)
            dwLoadMode = 1;                 //Chargement seul
        else
            return dwHandle_FoundLoaded;    //Le SFX est déjà en mémoire
    }
    else
        dwLoadMode = 2; //Chargement complet
#ifdef REGISTERSOUND_INFO
 //   Com_Printf ( "RegisterSound : %s (mode=%d)\n", pszFileName, dwLoadMode );
#endif
    //A-bas les jokers !
    if (pszFileName[0]=='*')
        return dwHandleSfxDefaultSound;

    //Load sound
    fReEqLevel = 0;
    pbData = IKA_QualitySound_Load ( pszFileName, &info, &fReEqLevel );
    if (!pbData) pbData = S_CodecLoad ( pszFileName, &info );
        if (!pbData)
        return dwHandleSfxDefaultSound;

    //IKA_QualitySound_Exchange ( pszFileName, &pbData, &info );

    //return dwHandleSfxDefaultSound;

    //Copie des données vers le SFX
    lFin = 0;
    do {
#ifdef REGISTERSOUND_INFO
 //       Com_Printf ( " + Pass %d\n", lFin );
#endif
        if (dwLoadMode==1)
        {
            lReturn = pSAM_SFX_Load (
                dwHandle_FoundLoaded,   //SFX already exists
                pszFileName,
                info.rate,
                info.samples,
                (info.channels==1)?(0):(1),
                IKA_SampleFormatWidthToBits(info.width),
                pbData,
                NULL );
        }
        else if (dwLoadMode==2)
        {
            lReturn = pSAM_SFX_Load (
                0xFFFFFFFF,     //Get a new sfx handle
                pszFileName,
                info.rate,
                info.samples,
                (info.channels==1)?(0):(1),
                IKA_SampleFormatWidthToBits(info.width),
                pbData,
                &dwHandle_FoundLoaded );
        }
        else
        {
            lFin = 32;
        }

        /*if (lReturn)
        {
            lReturn = lReturn;
        }
        */

        switch (lReturn)
        {
            case -3: //Plus assez de mémoire pour stocker les données du son (il faut vider d'autres anciens sons)
                dwReservedListHandle[0] = dwHandleSfxDefaultSound;
                dwReservedListHandle[1] = dwHandle_FoundLoaded;
#ifdef REGISTERSOUND_INFO
                Com_Printf ( " + Error (-3) unload\n" );
#endif
                if (!pSAM_SFX_GetOldest ( &dwFoundKillHandle, dwReservedListHandle, 3-dwLoadMode ))
                {
                    pSAM_SFX_GetName ( dwFoundKillHandle, szTmp );
                    pSAM_SFX_Unload ( dwFoundKillHandle );
#ifdef REGISTERSOUND_INFO
                    Com_Printf ( " + Unloaded (%d) : %s\n", dwFoundKillHandle, szTmp );
#endif
                }
                else
                {
                    lFin = 32;
#ifdef REGISTERSOUND_INFO
                    Com_Printf ( " !! Can't unload !\n" );
#endif
                }

                break;

            case -2: //Plus assez de mémoire pour allouer un nouvel SFX !
#ifdef REGISTERSOUND_INFO
                Com_Printf ( "RegisterSound. Not enough SFX place ! Increase 's_samSFXCount' value to more than %d", samConfig.dwSoftwareSFXCount+32 );
#endif
                dwHandle_FoundLoaded = dwHandleSfxDefaultSound;
                lFin = 32;
                break;

            case -1:
#ifdef REGISTERSOUND_INFO
                Com_Printf ( "RegisterSound. Error during allocation..." );
#endif
                dwHandle_FoundLoaded = dwHandleSfxDefaultSound;
                lFin = 32;
                break;

            case 0: //OK !
                lFin = 32;
                break;
        }

        lFin++;
    } while (lFin<32);

    Z_Free(pbData);

    if (dwHandle_FoundLoaded==0xFFFFFFFF)
    {
#ifdef REGISTERSOUND_INFO
        Com_Printf ( "RegisterSound. Impossible to load %s...", pszFileName );
#endif
        dwHandle_FoundLoaded = dwHandleSfxDefaultSound;
    }
    else pSAM_Message ( SAM_MESSAGE_SFX_DEFAULTLEVEL_SET, dwHandle_FoundLoaded, (DWORD)&fReEqLevel );
#ifdef REGISTERSOUND_INFO
 //   Com_Printf ( " + Returned : %d\n", dwHandle_FoundLoaded );
#endif
    return (sfxHandle_t)dwHandle_FoundLoaded;

}



/*
#######################################
S_SAM_StartLocalSound

    iChannelNum - Channel number used to play the sfx


#######################################
*/
void S_SAM_StartLocalSound( sfxHandle_t sfxHandle, int iChannelNum )
{
        if ( (sfxHandle<0) || (sfxHandle>=samConfig.dwSoftwareSFXCount) ) {
                Com_Printf( S_COLOR_YELLOW"IKALIZER_StartLocalSound: handle %d out of range\n", sfxHandle );
                return;
        }

        S_SAM_StartSound (
            NULL,
            SAM_iListenerEntityNumber,
            iChannelNum,
            sfxHandle );
}



/*
#######################################
S_SAM_StartSound
#######################################
*/
void S_SAM_StartSound ( vec3_t vec3Origin, int iEntityNumber, int iEntChannel, sfxHandle_t sfxHandle )
{
    SAM_VOICE_SINGLEPLAY_STACK  *psamVoiceSPS;
    long                        lVoiceSPS;

#ifdef DEBUG_COMMAND_STARTSOUND
    Com_Printf ( "StartSound. EntNumb=%d, EnCh=%d, sfx=%d\n", iEntityNumber, iEntChannel, sfxHandle );
#endif

    //If Origin is null and entity number is invalid : do error !
    if ( (!vec3Origin) && ( (iEntityNumber<0) || (iEntityNumber>MAX_GENTITIES) ) )
    {
        Com_Error( ERR_DROP, "S_IKALIZER_StartSound: bad entitynum %i", iEntityNumber );
        return;
    }

        if ( (sfxHandle<0) || (sfxHandle>=samConfig.dwSoftwareSFXCount) ) {
                Com_Printf( S_COLOR_YELLOW"IKALIZER_StartLocalSound: sfxhandle %d out of range\n", sfxHandle );
                return;
        }

    //Ajoute un son     (si possible)
        lVoiceSPS = SAM_VoiceSPS_Add ( iEntityNumber, iEntChannel, sfxHandle );
        if (lVoiceSPS<0) return;
        psamVoiceSPS = &(samVoiceSinglePlayStack[lVoiceSPS]);

        psamVoiceSPS->bUpdateMode                   = 1;
        psamVoiceSPS->fDistanceLevel                = 0;
        psamVoiceSPS->fDistanceMeters               = 0;
        psamVoiceSPS->fSourceSize                   = 0.1F;
        psamVoiceSPS->fFilter                       = 1.0;
        psamVoiceSPS->fMasterLevel                  = 0; //-6.0F; // fMasterLevelSFX; //ioq3-urt, 0 attempting to simulate 0.22b
        psamVoiceSPS->lAngle                        = 0;
        psamVoiceSPS->samVoiceParams.bLoopMode      = 0;

    //3D position
    psamVoiceSPS->samVoiceParams.bIsOrigin      = (vec3Origin)?(1):(0);
    psamVoiceSPS->samVoiceParams.bIsVelocity    = 0;
    if (vec3Origin) VectorCopy ( vec3Origin, psamVoiceSPS->samVoiceParams.vec3Origin );

}


void S_SAM_StartBackgroundTrack ( const char * pszIntroFileName, const char * pszLoopFileName )
{
    long            lEnableIntro;
    long            lEnableLoop;

    szMusicIntro[0] = 0;
    szMusicLoop[0] = 0;

    if ((pszIntroFileName)&&(pszLoopFileName))
    {
        if (Q_stricmp(pszIntroFileName,pszLoopFileName)==0)
            pszIntroFileName = NULL;
    }

    //Gestion de l'intro
    if (pszIntroFileName)
    {
        strcpy ( szMusicIntro, pszIntroFileName );
        if (dwHandleSfxMusicIntro!=0xFFFFFFFF)
        {
            if (dwHandleVoiceMusicIntro!=0xFFFFFFFF)
            {
                pSAM_MUSIC_Delete ( dwHandleVoiceMusicIntro );
                dwHandleVoiceMusicIntro = 0xFFFFFFFF;
            }
            pSAM_SFX_Free ( dwHandleSfxMusicIntro );
            dwHandleSfxMusicIntro = 0xFFFFFFFF;
        }

        //Allocation d'une nouvelle intro
        dwHandleSfxMusicIntro = (DWORD)S_SAM_RegisterSound ( pszIntroFileName, 0 );

        //Lecture de l'intro...
        pSAM_MUSIC_AddSFX ( dwHandleSfxMusicIntro, &dwHandleVoiceMusicIntro );
    }

    //Gestion de la boucle
    if (pszLoopFileName)
    {
        strcpy ( szMusicLoop, pszLoopFileName );
        if (dwHandleSfxMusicLoop!=0xFFFFFFFF)
        {
            if (dwHandleVoiceMusicLoop!=0xFFFFFFFF)
            {
                pSAM_MUSIC_Delete ( dwHandleVoiceMusicLoop );
                dwHandleVoiceMusicIntro = 0xFFFFFFFF;
            }
            pSAM_SFX_Free ( dwHandleSfxMusicLoop );
            dwHandleSfxMusicIntro = 0xFFFFFFFF;
        }

        //Allocation d'une nouvelle intro
        dwHandleSfxMusicLoop = (DWORD)S_SAM_RegisterSound ( pszLoopFileName, 0 );

        //Lecture de l'intro...
        pSAM_MUSIC_AddSFX ( dwHandleSfxMusicLoop, &dwHandleVoiceMusicLoop );
        pSAM_MUSIC_SetLoop ( dwHandleVoiceMusicLoop, 1 );
    }

    //Gestion de la liaison boucle/intro
    if ((dwHandleVoiceMusicIntro!=0xFFFFFFFF)&&(dwHandleVoiceMusicLoop!=0xFFFFFFFF))
        pSAM_MUSIC_SetJump ( dwHandleVoiceMusicIntro, dwHandleVoiceMusicLoop );

    //Démarrage de la lecture...
    if (dwHandleVoiceMusicIntro!=0xFFFFFFFF)
        pSAM_MUSIC_Play ( dwHandleVoiceMusicIntro );
    else if (dwHandleVoiceMusicLoop!=0xFFFFFFFF)
        pSAM_MUSIC_Play ( dwHandleVoiceMusicLoop );
}


void S_SAM_StopBackgroundTrack ( void )
{

    pSAM_MUSIC_Delete ( 0xFFFFFFFF );
    dwHandleVoiceMusicIntro = 0xFFFFFFFF;
    dwHandleVoiceMusicLoop = 0xFFFFFFFF;

    if (dwHandleSfxMusicIntro!=0xFFFFFFFF)
    {
        pSAM_SFX_Free ( dwHandleSfxMusicIntro );
        dwHandleSfxMusicIntro = 0xFFFFFFFF;
    }

    if (dwHandleSfxMusicLoop!=0xFFFFFFFF)
    {
        pSAM_SFX_Free ( dwHandleSfxMusicLoop );
        dwHandleSfxMusicLoop = 0xFFFFFFFF;
    }

    //Close all stream
    szMusicIntro[0] = 0;
    szMusicLoop[0] = 0;

}

void S_SAM_StopAllSounds ( void )
{
        //Stop the background
        S_SAM_StopBackgroundTrack ( );

    //Stop other sounds
        S_SAM_ClearSoundBuffer ();
}

void S_SAM_ClearSoundBuffer ( void )
{
    DWORD   dwVoiceHandle;

        //Clear all channels
    for (dwVoiceHandle=0;dwVoiceHandle<SAM_dwSupportVirtualVoicesCount;dwVoiceHandle++)
    {
        pSAM_VOICE_Free ( dwVoiceHandle );
    }

    SAM_VoiceSPS_Flush ( );

    //Clear Audio out
    S_SAM_StopBackgroundTrack ( );
    //...

    pSAM_Message ( SAM_MESSAGE_OUTPUT_FLUSH, 0, 0 );
}



/*

    Appelé à chaque affichage :

        // any looped sounds will be respecified as entities
        // are added to the render list
        trap_S_ClearLoopingSounds(qfalse);


*/
void S_SAM_ClearLoopingSounds ( qboolean bKillAll )
{
    SAM_VOICE_LOOP_PLAYSTACK *psamVoiceLPS;
    long i;

    psamVoiceLPS = samVoiceLoopPlayStack;
    for (i=0;i<lVoiceLoopPlayStackCount;i++,psamVoiceLPS++)
    {
        if ( ( (psamVoiceLPS->bIsActive) && (psamVoiceLPS->bIsFrameKill) ) ||
                (bKillAll) )
        {
            if (psamVoiceLPS->dwVoiceHandle!=0xFFFFFFFF)
                pSAM_VOICE_Free ( psamVoiceLPS->dwVoiceHandle );

            psamVoiceLPS->bIsActive     = 0;
            psamVoiceLPS->dwVoiceHandle = 0xFFFFFFFF;
        }
    }
}

void S_SAM_StopLoopingSound ( int iEntityNumber )
{
    SAM_VOICE_LOOP_PLAYSTACK *psamVoiceLPS;

    psamVoiceLPS = &(samVoiceLoopPlayStack[iEntityNumber]);

    if (psamVoiceLPS->dwVoiceHandle!=0xFFFFFFFF)
        pSAM_VOICE_Free ( psamVoiceLPS->dwVoiceHandle );

    psamVoiceLPS->bIsActive     = 0;
    psamVoiceLPS->dwVoiceHandle = 0xFFFFFFFF;
}


void S_SAM_DisableSounds ( void )
{

}

/*
==================
S_AddLoopingSound

Called during entity generation for a frame
Include velocity in case I get around to doing doppler...
==================
*/
void S_SAM_AddLoopingSound ( int iEntityNumber, const vec3_t vec3Origin, const vec3_t vec3Velocity, sfxHandle_t sfxHandle )
{
    SAM_VOICE_LOOP_PLAYSTACK *psamVoiceLPS;
    long lEntry;

        if ( (sfxHandle<0) || (sfxHandle>=samConfig.dwSoftwareSFXCount) ) {
                Com_Printf( S_COLOR_YELLOW "IKALIZER_AddLoopingSound: handle %i out of range\n", sfxHandle );
                return;
        }

        if (sfxHandle==0)
            return;

    //Ajouter une LPS
        lEntry = SAM_VoiceLPS_AddUpdate ( iEntityNumber, sfxHandle );
        if (lEntry<0)
        {
            Com_Printf( "AddLoopingSound: Can't add ! (LPS is full!) entity=%d . sfx=%d\n", iEntityNumber, sfxHandle );
            return;
        }

        psamVoiceLPS = &(samVoiceLoopPlayStack[lEntry]);
        psamVoiceLPS->bIsFrameKill  = 1;
        //psamVoiceLPS->bIsSphere     = 0;
        psamVoiceLPS->fSourceSize   = 0.1F;
        psamVoiceLPS->samVoiceParams.bLoopMode   = 1;
        psamVoiceLPS->samVoiceParams.bIsOrigin   = 1;
        psamVoiceLPS->samVoiceParams.bIsVelocity = 1;
        VectorCopy ( vec3Origin, psamVoiceLPS->samVoiceParams.vec3Origin );
    VectorCopy ( vec3Velocity, psamVoiceLPS->samVoiceParams.vec3Velocity );
}

/*
==================
S_AddRealLoopingSound

Called during entity generation for a frame
Include velocity in case I get around to doing doppler...

This proc is used for EntityType = ET_SPEAKER (like in game radio-box, boom-box...)
These sounds don't be killed after a frame
==================
*/
void S_SAM_AddRealLoopingSound( int iEntityNumber, const vec3_t vec3Origin, const vec3_t vec3Velocity, sfxHandle_t sfxHandle )
{
    SAM_VOICE_LOOP_PLAYSTACK *psamVoiceLPS;
    long lEntry;

        if ( (sfxHandle<0) || (sfxHandle>=samConfig.dwSoftwareSFXCount) ) {
                Com_Printf( S_COLOR_YELLOW "IKALIZER_AddRealLoopingSound: handle %i out of range\n", sfxHandle );
                return;
        }

    //Ajouter une LPS
        lEntry = SAM_VoiceLPS_AddUpdate ( iEntityNumber, sfxHandle );
        if (lEntry<0)
        {
            Com_Printf( "AddLoopingSound: Can't add ! (LPS is full!) entity=%d . sfx=%d\n", iEntityNumber, sfxHandle );
            return;
        }

        psamVoiceLPS = &(samVoiceLoopPlayStack[lEntry]);
        psamVoiceLPS->bIsFrameKill  = 0;
        //psamVoiceLPS->bIsSphere     = 1;
        psamVoiceLPS->fSourceSize   = 1.0F;
        //psamVoiceLPS->bUpdateMode   = 1;
        psamVoiceLPS->samVoiceParams.bLoopMode   = 1;
        psamVoiceLPS->samVoiceParams.bIsOrigin   = 1;
        psamVoiceLPS->samVoiceParams.bIsVelocity = 1;
        VectorCopy ( vec3Origin, psamVoiceLPS->samVoiceParams.vec3Origin );
    VectorCopy ( vec3Velocity, psamVoiceLPS->samVoiceParams.vec3Velocity );
}

/*

    Underwater FX :
        Like Adobe Audition VST Plugin "Echo chamber" with "Underwater PA"
        Add low pass with cut at 3500Hz

*/

void SAM_SpatializeFromOrigin_CalcWaterFilter ( long iInWater, float * pfLevel, float * pfFilter )
{
    float fLevel;
    float fFilter;

    if (pfLevel)  fLevel  = *pfLevel;  else return;
    if (pfFilter) fFilter = *pfFilter; else return;

    if (iInWater)
    {
        fFilter *= 500.0F / (float)samConfig.dwHardwareSamplingRate;
        fLevel += 18.0F;
    }

    if (pfLevel)  *pfLevel  = fLevel;
    if (pfFilter) *pfFilter = fFilter;
}


void IKALIZER_SpatializeFromOrigin_ioQuake ( float fDistance, float fSourceSize, float * pfLevel, float * pfFilter )
{
    float f1, f2;
    float fDistanceMeters;

    float fLevel, fFilter;

    /*
        //Traîtement de ioQuake pour la distance
        f1 = fDistance;
        f1 *= 0.0008F;
        f1 = (1.0 - f1);

    //Traitement distance => attenuation
        if (fDistance>=1250)
        {
            //Petite modif pour se rapprocher de l'effet d'occultation par distance d'ioQuake
            f1 = fDistance-1250;
            f1 = pow ( f1, 1.5F );
            fDistanceMeters = (1250+f1)*0.1F;
        }
        else fDistanceMeters = fDistance * 0.1F;

        f2 = fDistanceMeters;
        if (f2<1.0F) f2 = 1.0F;     //On considère qu'à moins de 1m, le son reste le même
        f2 = log10 ( f2 ) - 0.1F;
        if (f2<0) f2 = 0.0F;
        fLevel = -(f2*f2)*5;
        */

        f1 = fDistance;
        f1 *= 0.0008F;
        f1 = (1.0 - f1);

        if (f1>0) fLevel = log10 ( f1 ) * 20;
        else      fLevel = -100;







    //Atténuation par occultation sur le niveau est inférieur à -100dB
        if (fLevel<-100) fLevel = -200;

    //Le passe-bas de distance
    fFilter = 1.0F;
    /*
    f2 = fDistance * 0.01F;
    f2 -= 8;
    if (f2<1.0F) f2 = 1.0F;
    fFilter = 1.0F / f2;
    if (fFilter>1.0F) fFilter = 1.0F;
    if (fFilter<0.01F) fFilter = 0.01F;
    */
    if (pfLevel)  *pfLevel  = fLevel;
    if (pfFilter) *pfFilter = fFilter;
}


void IKALIZER_SpatializeFromOrigin_v0_21_7 ( float fDistance, float fSourceSize, float * pfLevel, float * pfFilter )
{
    float f1;
    float fDistanceMeters;

    float fLevel, fFilter;

        /*
            Rappel : Atténuation du son en extérieur (Temp= 30°C Hygro=30% Alt=100m)

            Fréq.     k    (k @ XX m)
            500Hz  => 0.20 (  0.0 dB)
            1000Hz => 0.49 ( -7.8 dB)
            2000Hz => 1.21 (-15.6 dB)
            4000Hz => 3.06 (-23.7 dB)
            6000Hz => 5.60 (-28.9 dB)

            Cair= 344m/s

            Niveau de puissance acoustique selon la distance (m)

            Espace sphérique
                Lp = Lw - 10.log (4.PI.r.r) + G
                => Lp = Lw - 11 - 20.log(r)

            Espace hémisphérique
                Lp = Lw - 10.log (2.PI.r.r) + G
                => Lp = Lw - 8 - 20.log(r)
        */

    //Distance en mètres
        f1 = fDistance + 80.0F;
        f1 *= 0.01F;                //Convertion en metres
        fDistanceMeters = f1;

        //Traitement distance => attenuation
        if (f1<1.0F) f1 = 1.0F;     //On considère qu'à moins de 1m, le son reste le même
        f1 = 1.0F / f1;
        fLevel = log10 ( f1 ) * 20;



    //Le passe-bas de distance

    f1 = log10 ( fDistance + 1.0F ) * 20.0F;
    fFilter = pow ( 1.0F / ( f1 + 1.0F ), 0.9F ) * 20.0F;
    fFilter = pow ( fFilter, 2.0F ) * 10.0F;
    if (fFilter>1.0F) fFilter = 1.0F;
    if (fFilter<0.1F) fFilter = 0.1F;

    if (pfLevel)  *pfLevel  = fLevel;
    if (pfFilter) *pfFilter = fFilter;
}



void IKALIZER_SpatializeFromOrigin_v0_21_10 ( float fDistance, float fSourceSize, float * pfLevel, float * pfFilter )
{
    float f1, f2;
    float fLevel, fFilter;
    float fLevelHigh;
    float fLevelLow;
    float fRatioDistance;
    float fRatioComp;
    float fListenSize;

    //Les distances dans UrbanTerror sont en pouces...
    //Application d'une mise à l'echelle : Convertion des pouces en metres...
    fDistance *= 0.0254F;

    //Volume de l'auditeur = 0.80m
    fListenSize = 0.80F;

    //Le ratio de distance... 1 à +oo
    fRatioDistance = fDistance / ( fSourceSize + fListenSize );
    if (fRatioDistance<1.0F) fRatioDistance = 1.0F;

    //Application de l'atténuation...
    f1              = log10 ( fRatioDistance );
    f2              = pow ( fRatioDistance, 0.1F );
    fRatioComp      = pow ( f1, f2 + 1.0F );
    fLevelHigh      = -pow ( fRatioComp, 1.5F ) * 10.0F;

    f2 = f2 - 1.3F;
    if (f2<0.0F) f2 = 0.0F;

    fLevelLow       = -f1 * 5.0F;
    fLevelLow       += fLevelHigh * f2;

    //Le filtre
    f1 = fLevelHigh+45;
    if (f1>0) f1 = 0;
    f2 = fLevelHigh+40;
    if (f2>0) f2 = 0;


    fFilter = ( 1 / ( 1 - f1 ) );
    if (fFilter>1.0F) fFilter = 1.0F;
    if (fFilter<0.0F) fFilter = 0.0F;

    f1 = pow ( fFilter, 0.5F );
    fLevelHigh *= 0.7F;
    fLevel = (fLevelHigh * f1) + (fLevelLow * (1-f1));


    fFilter         = 0.001F + ( 1 / ( 1 - f2 ) );
    if (fFilter>1.0F   ) fFilter = 1.0F;
    if (fFilter<0.005F) fFilter = 0.005F;

    fFilter = pow ( fFilter, 2.0F );
    if (fFilter<0.005F) fFilter = 0.005F;
    /*

        fLevel = -10 . [ f1^(f2+1)             ]^1.5

    */

    //On bloque en dessous de -100dB
    if (fLevel<-120) fLevel = -200;

    //fLevel = -f1 * 10.0F;

    //Com_Printf( "SFO: LevelL=%4.1f, LevelH=%4.1f, fFilter=%2.5f\n", fLevelLow, fLevelHigh, fFilter );


    //Le filtre de distance...
    //fFilter = 1.0F / (1.0F+fRatioComp);
    //fFilter = pow ( fFilter, 1.5F );
    //fFilter = 0.01F;


    if (pfLevel)  *pfLevel  = fLevel;
    if (pfFilter) *pfFilter = fFilter;

}

void SAM_SpatializeFromOrigin ( vec3_t vec3Origin, float fSourceSize, long iInWater, float * pfLevel, float * pfFilter, long * plAngle, float * pfDistance )
{
    vec3_t                      vec3Source;
    vec3_t                      vec3Rotated;
    vec3_t                      vec3Angle;
    vec_t               vecDistance;

    float               fGain;
    float               fLevel, fQuakeLevel;
    float               fFilter;
    float               f1, f2;
    long                lAngle;
    float               fDistance;
    float               fDistanceBase;
    float               fDistanceMeters;

    float               fValToDeg;

        // calculate stereo separation and distance attenuation
        VectorSubtract ( vec3Origin, SAM_vec3ListenerOrigin, vec3Source );

    // Distance from source...
    vecDistance = VectorNormalize(vec3Source);
    fDistanceBase = vecDistance;

        vecDistance -= 80.0;
        if (vecDistance < 0)
                vecDistance = 0;                        // close enough to be at full volume
        fDistance = vecDistance;
        vecDistance *= 0.0008;          // different attenuation levels

        VectorRotate( vec3Source, SAM_vec3ListenerAxis, vec3Rotated );

        fLevel  = 0.0F;
        fFilter = 1.0F;
        switch (IKALIZER_iDistanceRendering)
        {
            case 0:
            default:
            IKALIZER_SpatializeFromOrigin_v0_21_7 ( fDistance, fSourceSize, &fLevel, &fFilter );
            break;

        case 1:
            IKALIZER_SpatializeFromOrigin_ioQuake ( fDistance, fSourceSize, &fLevel, &fFilter );
            break;

        case 2:
            fDistance = fDistanceBase;
                if (fDistance < 0)
                        fDistance = 0;                  // close enough to be at full volume

            IKALIZER_SpatializeFromOrigin_v0_21_10 ( fDistance, fSourceSize, &fLevel, &fFilter );
            break;
    }



    //Com_Printf( "SFO: Level=%4.1f, Dist=%f, DistM=%f\n", fLevel, fDistance, fDistanceMeters );


    //Le passe-bas sous l'eau
    SAM_SpatializeFromOrigin_CalcWaterFilter (
        iInWater,
        &fLevel,
        &fFilter );

    //L'angle
    vectoangles ( vec3Rotated, vec3Angle );
    lAngle = vec3Angle[1];
    if (lAngle>=180) lAngle = lAngle-360;

    if (pfFilter)   *pfFilter   = fFilter;
    if (pfLevel)    *pfLevel    = fLevel;
    if (plAngle)    *plAngle    = lAngle;
    if (pfDistance) *pfDistance = fDistance; //Meters; // wtf
}

void S_SAM_Respatialize ( int iEntityNumber, const vec3_t vec3Head, vec3_t vec3Axis[3], int iInWater )
{
    DWORD                   dwVoiceHandle;
    DWORD                   dwEntityCode;
    DWORD                   dwEntityNumber;
    DWORD                   dwEntityChannel;
    DWORD                   dwStartVoiceHandle;

    vec3_t                  vec3Origin;
    vec3_t                          vec3Source;
    vec3_t                          vec3Rotated;
    vec_t                   vecDistance;

    float                   fLevel, fPression;
    float                   fFilter;
    long                    lAngle;
    float                   fDistance;

    long                    lReturn;
    long                    lIndex;
    long                    lIndex_Dup;
    SAM_VOICE_PARAMS        samVoiceParams;
    SAM_VOICE_SINGLEPLAY_STACK  *psamVoiceSPS;
    SAM_VOICE_LOOP_PLAYSTACK    *psamVoiceLPS;


    //Pre-process
    SAM_iListenerInWater      = iInWater;
    SAM_iListenerEntityNumber = iEntityNumber;
    VectorCopy ( vec3Head, SAM_vec3ListenerOrigin );
    VectorCopy ( vec3Axis[0], SAM_vec3ListenerAxis[0] );
    VectorCopy ( vec3Axis[1], SAM_vec3ListenerAxis[1] );
    VectorCopy ( vec3Axis[2], SAM_vec3ListenerAxis[2] );

    //On parcours les canaux non-looped (et éventuellement, on met à jour les entités "loop")
    psamVoiceSPS = samVoiceSinglePlayStack;
    psamVoiceLPS = samVoiceLoopPlayStack;
    for (lIndex=0;lIndex<lVoiceSinglePlayStackCount;lIndex++,psamVoiceSPS++)
    {
        if (psamVoiceSPS->bIsActive)
        {
            if (psamVoiceSPS->bUpdateMode==0)
                psamVoiceSPS->bUpdateMode = 2;

            if (psamVoiceSPS->bUpdateMode)
            {
                if (psamVoiceSPS->iEntityNumber==iEntityNumber)
                {
                    //Entité de l'auditeur => Paramètres par défaut
                    psamVoiceSPS->fDistanceLevel    = 0.0F;
                    psamVoiceSPS->fFilter           = 1.0F;
                    psamVoiceSPS->lAngle            = 0;
                    psamVoiceSPS->fDistanceMeters   = 0;
                    psamVoiceSPS->fSourceSize       = 0.1F;

                    SAM_SpatializeFromOrigin_CalcWaterFilter (
                        iInWater,
                        &(psamVoiceSPS->fDistanceLevel),
                        &(psamVoiceSPS->fFilter) );
                }
                else
                {
                    //Entité autre => Détermine les paramètres
                    if (psamVoiceSPS->samVoiceParams.bIsOrigin)
                        VectorCopy ( psamVoiceSPS->samVoiceParams.vec3Origin, vec3Origin );
                    else
                        VectorCopy ( psamVoiceLPS[psamVoiceSPS->iEntityNumber].samVoiceParams.vec3Origin, vec3Origin );

                    //Com_Printf( "non-looped-spatialize=%d\n", psamVoiceSPS->sfxHandle );
                    SAM_SpatializeFromOrigin (
                        vec3Origin,
                        iInWater,
                        psamVoiceSPS->fSourceSize,
                        &(psamVoiceSPS->fDistanceLevel),
                        &(psamVoiceSPS->fFilter),
                        &(psamVoiceSPS->lAngle),
                        &(psamVoiceSPS->fDistanceMeters) );

                    //Com_Printf( "RespatializeLoop: %f = Dist=%d\n", psamVoiceLPS->iEntityNumber, fDistance );
                }
            }
        }
    }


    //On parcours les voix loop
    psamVoiceLPS = samVoiceLoopPlayStack;
    for (lIndex=0;lIndex<lVoiceLoopPlayStackCount;lIndex++,psamVoiceLPS++)
    {
        if (psamVoiceLPS->bIsActive)
        {
            if (psamVoiceLPS->bUpdateMode==0)
                psamVoiceLPS->bUpdateMode = 2;

            if (psamVoiceLPS->bUpdateMode)
            {
                //if ((psamVoiceLPS->sfxHandle==31))
                //    Com_Printf( "sfx = %d\n", psamVoiceLPS->sfxHandle );

                //Spatialisation
                SAM_SpatializeFromOrigin (
                    psamVoiceLPS->samVoiceParams.vec3Origin,
                    psamVoiceLPS->fSourceSize,
                    iInWater,
                    &fLevel,
                    &fFilter,
                    &lAngle,
                    &fDistance );

                //Type de LPS
                /*if (psamVoiceLPS->bIsSphere) fLevel -= 9;   // Sphere
                else                         fLevel -= 6;   // 3D point
                */
                if (IKALIZER_iDistanceRendering==2) fLevel -= 6;
                psamVoiceLPS->fDistanceLevel    = fLevel;
                psamVoiceLPS->fFilter           = fFilter;
                psamVoiceLPS->lAngle            = lAngle;
                psamVoiceLPS->fDistanceMeters   = fDistance;

                //Com_Printf( "RespatializeLoop: %d = Dist=%f\n", psamVoiceLPS->iEntityNumber, fDistance );
            }
        }
    }
}



void S_SAM_UpdateEntityPosition ( int iEntityNumber, const vec3_t vec3Origin )
{
    SAM_VOICE_LOOP_PLAYSTACK    *psamVoiceLPS;

    //If Origin is null or entity number is invalid : do nothing !
    if ( (iEntityNumber<0) || (iEntityNumber>MAX_GENTITIES) )
    {
        Com_Error( ERR_DROP, "SAM_UpdateEntityPosition: bad entitynum %i", iEntityNumber );
        return;
    }

    psamVoiceLPS = &(samVoiceLoopPlayStack[iEntityNumber]);
    psamVoiceLPS->samVoiceParams.bIsOrigin = 1;
    VectorCopy( vec3Origin, psamVoiceLPS->samVoiceParams.vec3Origin );
}

/*
"^4.####\\   .##.   ##. .##",
"^4##   `  ##  ##  ###.###",
"^4'####.  ######  ##'#'##",
"^4.   ##  ##  ##  ##   ##  Audio ",
"^4\\####'  ##  ##  ##   ##  Engine",

"    .####-   .##.   ##. .## ",
"    ##OOO`o ##oO##O ###.###O",
"    '####.  ######O ##'#'##O",
"    . OO##o ##O ##O ##OoO##O Audio ",
"    -####'O ##O ##O ##O  ##O Engine",
"      OOOOo  OO  OO  OO   OO",

*/
char *szSamHelp[] =
{
"",
/*"^7    .####\\   .##.   ##. .## ",
"^7    ##^4###^7`^4'^7 ##^4'#^7##^4.^7 ###.###^4#^7",
"^7    '####.  ######^4#^7 ##'#'##^4#",
"^7    . ^4##^7##^4'^7 ##^4#^7 ##^4#^7 ##^4#'#^7##^4#^7   Audio ",
"^7    \\####'^4#^7 ##^4#^7 ##^4#^7 ##^4#^7  ##^4#^7   Engine",
"^4     \\####'  ##  ##  ##   ##",
"",*/
"IKALIZER online help : Type 's_samXXXX' or 'ikalizer_XXXX' or 'ika_YY'",
"----------------------------",
" ^2Realtime command :",
"   ^5ika_^3L^5imiter^3L^5evel ^3X^7      Limiter gain (^30...3^7 => +0/+6/+12/+20dB)",
"   ^5ika_^3H^5elp^7                This help",
//"    ^5s_samRestart^7             Restart IKALIZER audio engine (^3disabled^7)",
"   ^5ika_^3D^5evice^3E^5num^7          List all DirectSound available device",
"   ^5ika_^3D^5ynamic^3D^5elay^3L^5ines ^3X^7 Quality/Speed of 0x21/0x22 mode",
"                            ^3-1^7=Auto ^30^7=Off ^31...100^7 Quality level",
"   ^5ika_^3M^5ax^3U^5sage^7            MAX CPU usage with single core CPU (def=^37^7)",
"                            ^30^7=Off ^31...30^7 Total allowed usage (in %%)",
"   ^5ika_^3S^5ampling^3R^5ate ^3XXXXX^7  Sound card sampling frequency (def=^348000^7)",
"   ^5ika_^3C^5hannel^3M^5ode ^3XXXX^7    0x^3A^2B^7 ^3A^7=Hardware channels ^2B^7=Mix mode",
"                            ^30x20^7=simple stereo ^30x22^7=360VS",
"                            ^30x23^7=DPL (3/1/0ch) ^30x24^7=DPLII (3/2/0ch)",
"                            ^30x21^7=headphones hybrid-HRTF",
"                            ^30x25^7=headphones holographic",
"                            ^30x26^7=headphones virtual holographic",
"                            ^30x40^7=Quad ^30x60^7=5.0 ^30x61^7=5.1",
"   ^5ika_^3D^5evice^3S^5elect ^3X^7      Select a DirectSound device (def=^30^7)",
"   ^5ika_^3D^5istance^3R^5endering ^3X^7 ^30^7=Initial iKALiZER ^31^7=ioQuake ^32^7=Theatre (def)",
"   ^5ika_^3R^5eplace^3S^5amples ^3X^7    ^30^7=Disable samples replace ^31^7=Enable s. r. (def)",
//"",
" ^2Off-line command, need a general restart :^7", // '^5s_samRestart^7'",

//"    ^5s_samLatencyDuration ^3XX^7  Sound reactivity in ms (def=^320^7)",
//"    ^5s_samBufferDuration ^3XXX^7  Global sound buffer in ms (def=^3250^7)",
"   ^5ika_^3V^5oice^3C^5ount ^3XX^7       Total voices available for mixer (def=^332^7)",
//"    ^5s_samMemoryAlloc ^3XX^7      Total memory in MiB allowed (def=^364^7)",
"   ^5ika_^3E^5nable^3S^5MP ^3X^7         0=keep system(def), 1=dual, 2=single",
"",
" Tips: You could type 's_samMaxUsage' or 'ikalizer_MaxUsage' or 'ika_MU'.",
//"",
"@" };




void S_SAM_Help ( void )
{
    long i;

    for (i=0;szSamHelp[i][0]!='@';i++)
    {
        Com_Printf ( "%s\n", szSamHelp[i] );
    }

    if (!SAM_dwSupportDeviceSelect)
        Com_Printf( S_COLOR_YELLOW "WARNING ! Device functions are not supported with this older IKALIZER library !\n" );
}

void S_SAM_DeviceEnum ( void )
{
    long i;
    char szTmp[512];
    DWORD dwData;

    if (!SAM_dwSupportDeviceSelect)
        Com_Printf( S_COLOR_YELLOW "WARNING ! This function is not supported with this older IKALIZER library !\n" );
    else
    {
        pSAM_Message ( SAM_MESSAGE_DEVICESETENUM, 0, 0 );

        for (i=0;i<64;i++)
        {
                pSAM_GetInfo ( SAM_INFO_DEVICE_ENUM_00+i, &dwData, szTmp );
                if (dwData==0)
                {
                    Com_Printf( "  Device %d : %s\n", i, szTmp );
                }


        }
    }
}

void S_SAM_Update ( void )
{
    DWORD                   dwTotalVoiceLoop;
    DWORD                   dwTotalVoiceUnLoop;
    DWORD                   dwState;
    static long             lCount;
    DWORD                   dwVoiceHandle;
    long                    lIndex, lReturn;
    SAM_VOICE_SINGLEPLAY_STACK  *psamVoiceSPS;
    SAM_VOICE_LOOP_PLAYSTACK    *psamVoiceLPS;
    DWORD                   dwEntityCode, dwEntityCodeCopy;
    DWORD                   dwEntityNumber, dwEntityChannel;
    //SAM_LOOP_ENTITY_INFO    *psamLoopEntityInfo;
    long                    lShow;
    float                   fLevel, fMinLevel, fMaxLevel;
    static char             szValueChannelMode[16];

    //Level
    S_SAM_SetVolume ( );


    //###########################################################################################
    //  Recherche du niveau le plus élevé dans les canaux "single" et "loop"
    fMaxLevel    = -100;

    psamVoiceSPS = samVoiceSinglePlayStack;
    for (lIndex=0;lIndex<lVoiceSinglePlayStackCount;lIndex++,psamVoiceSPS++)
    {
        if ( (psamVoiceSPS->bIsActive) && (psamVoiceSPS->bUpdateMode) )
        {
            fLevel = psamVoiceSPS->fDistanceLevel + psamVoiceSPS->fMasterLevel;
            if (fLevel>fMaxLevel) fMaxLevel = fLevel;
        }
    }

    psamVoiceLPS = samVoiceLoopPlayStack;
    for (lIndex=0;lIndex<lVoiceLoopPlayStackCount;lIndex++,psamVoiceLPS++)
    {
        if ( (psamVoiceLPS->bIsActive) && (psamVoiceLPS->bUpdateMode) )
        {
            fLevel = psamVoiceLPS->fDistanceLevel + psamVoiceLPS->fMasterLevel;
            if (fLevel>fMaxLevel) fMaxLevel = fLevel;
        }
    }


    //###########################################################################################
    //  Détermine le niveau minimal
    fMinLevel = fMaxLevel - 60;
    if (fMinLevel<-100) fMinLevel = -100;


    //###########################################################################################
    //Désactive temporairement la gestion des voix de SAM
    pSAM_Message ( SAM_MESSAGE_VOICE_SYNCFREEZE, 1, 0 );


    //###########################################################################################
    //  Gestion des canaux "single"
    psamVoiceSPS = samVoiceSinglePlayStack;
    for (lIndex=0;lIndex<lVoiceSinglePlayStackCount;lIndex++,psamVoiceSPS++)
    {
        if (psamVoiceSPS->bIsActive)
        {
            //La voix en cours en toujours en lecture ?
            if ( (psamVoiceSPS->bUpdateMode==0) || (psamVoiceSPS->bUpdateMode==2) )
            {
                dwState = 0;
                dwEntityCode    = GetEntityCode(psamVoiceSPS->iEntityNumber,psamVoiceSPS->iEntityChannel);

                //On vérifie que l'on travaille sur la bonne entitée... Elle a peut-être été supprimée ?
                lReturn = pSAM_VOICE_GetUserID ( psamVoiceSPS->dwVoiceHandle|SAM_VOICE_FASTACCESS, &dwEntityCodeCopy );

                if ((!lReturn)&&(dwEntityCodeCopy==dwEntityCode))
                {
                    pSAM_Message ( SAM_MESSAGE_VOICE_ISPLAYED, psamVoiceSPS->dwVoiceHandle|SAM_VOICE_FASTACCESS, (DWORD)&dwState );

                    if (!dwState)
                    {
                        //Com_Printf( "  VoiceSPS => Stop = %d\n", psamVoiceSPS->dwVoiceHandle );
                        pSAM_VOICE_Free ( psamVoiceSPS->dwVoiceHandle|SAM_VOICE_FASTACCESS );
                        psamVoiceSPS->bIsActive = 0;
                    }
                }
                else psamVoiceSPS->bIsActive = 0;
            }

            fLevel = psamVoiceSPS->fDistanceLevel + psamVoiceSPS->fMasterLevel;

            switch (psamVoiceSPS->bUpdateMode)
            {
                case 1: //Immediate voice creation !
                case 4: //Delayed voice creation...

                    //Le niveau est-il suffisant ?
                    if (fLevel>fMinLevel)
                    {
                        //Generate the entity code
                        dwEntityCode = GetEntityCode(psamVoiceSPS->iEntityNumber,psamVoiceSPS->iEntityChannel);

                        //Alloc a new voice !
                        lReturn = pSAM_VOICE_Alloc  ( &dwVoiceHandle, 0, NULL );

                        if (lReturn)
                        {
                            Com_Printf( "  VoiceSPS => Can't create voice ! = %d\n", psamVoiceSPS->sfxHandle );
                            if (psamVoiceSPS->bUpdateMode==1)
                            {
                                //Le niveau est insuffisant, on tente une deuxième tentative pour la frame suivante...
                                psamVoiceSPS->bUpdateMode=4;
                            }
                            else
                            {
                                //Le niveau est insuffisant, on détruit la voix
                                psamVoiceSPS->bIsActive = 0;
                            }

                            break;
                        }

                        //Assign sfx
                        pSAM_VOICE_SetSFX           ( dwVoiceHandle|SAM_VOICE_FASTACCESS, (DWORD)psamVoiceSPS->sfxHandle );

                        //Assign the entity code
                        pSAM_VOICE_SetUserID        ( dwVoiceHandle|SAM_VOICE_FASTACCESS, dwEntityCode );

                        //Assign voice params
                        pSAM_VOICE_SetUserData      ( dwVoiceHandle|SAM_VOICE_FASTACCESS, (unsigned char *)&psamVoiceSPS->samVoiceParams, sizeof(SAM_VOICE_PARAMS) );

                        //Assign all other data
                        pSAM_VOICE_SetMasterLevel   ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceSPS->fMasterLevel );
                        pSAM_VOICE_SetDistanceLevel ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceSPS->fDistanceLevel );
                        pSAM_VOICE_SetRatioIIR      ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceSPS->fFilter );
                        pSAM_VOICE_SetOrigin        ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceSPS->lAngle, psamVoiceSPS->fDistanceMeters );

                        //Com_Printf( "Update: %d = Dist=%f\n", psamVoiceSPS->iEntityNumber, psamVoiceSPS->fDistanceMeters );

                        //At last... Play the sound !!!
                        pSAM_VOICE_SetPlay          ( dwVoiceHandle|SAM_VOICE_FASTACCESS, 1 );

                        //Save the voice handle
                        psamVoiceSPS->dwVoiceHandle = dwVoiceHandle;

                        psamVoiceSPS->bUpdateMode = 0;

                        //Com_Printf( "PlaySound=> Voice %d - Sfx %d\n", dwVoiceHandle, psamVoiceSPS->sfxHandle );
                    }
                    else
                    {
                        if (psamVoiceSPS->bUpdateMode==1)
                        {
                            //Le niveau est insuffisant, on tente une deuxième tentative pour la frame suivante...
                            psamVoiceSPS->bUpdateMode=4;
                        }
                        else
                        {
                            //Le niveau est insuffisant, on détruit la voix
                            psamVoiceSPS->bIsActive = 0;
                        }
                    }
                    break;

                case 2: //Update
                    dwVoiceHandle   = psamVoiceSPS->dwVoiceHandle;
                    dwEntityCode    = GetEntityCode(psamVoiceSPS->iEntityNumber,psamVoiceSPS->iEntityChannel);

                    //On vérifie que l'on travaille sur la bonne entitée... Elle a peut-être été supprimée ?
                    lReturn = pSAM_VOICE_GetUserID ( dwVoiceHandle|SAM_VOICE_FASTACCESS, &dwEntityCodeCopy );

                    if ((!lReturn)&&(dwEntityCodeCopy==dwEntityCode))
                    {
                        if (fLevel>fMinLevel)
                        {
                            pSAM_VOICE_SetDistanceLevel ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceSPS->fDistanceLevel );
                            pSAM_VOICE_SetRatioIIR      ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceSPS->fFilter );
                            pSAM_VOICE_SetOrigin        ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceSPS->lAngle, psamVoiceSPS->fDistanceMeters );

                            //Com_Printf( "Update: %d = Dist=%f\n", psamVoiceSPS->iEntityNumber, psamVoiceSPS->fDistanceMeters );
                        }
                        else
                        {
                            //Le niveau est insuffisant, on détruit la voix
                            pSAM_VOICE_Free ( dwVoiceHandle|SAM_VOICE_FASTACCESS );
                            psamVoiceSPS->bIsActive = 0;
                        }
                    }
                    else
                    {
                        //La voix est introuvable...
                        psamVoiceSPS->bIsActive = 0;
                    }

                    psamVoiceSPS->bUpdateMode = 0;

                default:
                    break;
            }
        }
    }


    //###########################################################################################
    //  Gestion des canaux "loop"
    psamVoiceLPS = samVoiceLoopPlayStack;
    for (lIndex=0;lIndex<lVoiceLoopPlayStackCount;lIndex++,psamVoiceLPS++)
    {
        if (psamVoiceLPS->bIsActive)
        {
            switch (psamVoiceLPS->bUpdateMode)
            {
                case 1: //Immediate voice creation !

                    //Alloc a new voice !
                    lReturn = pSAM_VOICE_Alloc  ( &dwVoiceHandle, 0, NULL );

                    if (lReturn)
                    {
                        Com_Printf( "  VoiceLPS => Can't create voice (%d)! = %d\n", lReturn, psamVoiceLPS->sfxHandle );
                        psamVoiceLPS->bUpdateMode = 0;
                        psamVoiceLPS->dwVoiceHandle = 0xFFFFFFFF;

                        break;
                    }

                    //Assign sfx
                    pSAM_VOICE_SetSFX           ( dwVoiceHandle|SAM_VOICE_FASTACCESS, (DWORD)psamVoiceLPS->sfxHandle );

                    //Assign the entity code
                    //pSAM_VOICE_SetUserID        ( dwVoiceHandle|SAM_VOICE_FASTACCESS, dwEntityCode );

                    //Assign voice params
                    pSAM_VOICE_SetUserData      ( dwVoiceHandle|SAM_VOICE_FASTACCESS, (unsigned char*)&psamVoiceLPS->samVoiceParams, sizeof(SAM_VOICE_PARAMS) );

                    //Assign all other data
                    pSAM_VOICE_SetMasterLevel   ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceLPS->fMasterLevel );
                    pSAM_VOICE_SetDistanceLevel ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceLPS->fDistanceLevel );
                    pSAM_VOICE_SetRatioIIR      ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceLPS->fFilter );
                    pSAM_VOICE_SetOrigin        ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceLPS->lAngle, psamVoiceLPS->fDistanceMeters );

                    //At last... Play the sound !!!
                    pSAM_VOICE_SetLoop          ( dwVoiceHandle|SAM_VOICE_FASTACCESS, 1 );
                    pSAM_VOICE_SetPlay          ( dwVoiceHandle|SAM_VOICE_FASTACCESS, 1 );

                    //Save the voice handle
                    psamVoiceLPS->dwVoiceHandle = dwVoiceHandle;

                    psamVoiceLPS->bUpdateMode        = 0;

                    //Recherche la voix la plus anciennement créée
                    {
                        DWORD                       dwTime;
                        long                        lFoundVoice;
                        long                        lIndex2;
                        DWORD                       dwPosition;
                        DWORD                       dwPositionTick;
                        SAM_VOICE_LOOP_PLAYSTACK    *psamVoiceLPS2;

                        lFoundVoice     = -1;
                        dwTime          = psamVoiceLPS->dwTimeStarted;
                        psamVoiceLPS2   = samVoiceLoopPlayStack;
                        for (lIndex2=0;lIndex2<lVoiceLoopPlayStackCount;lIndex2++,psamVoiceLPS2++)
                        {
                            if ( (lIndex2!=lIndex) &&
                                (psamVoiceLPS2->bIsActive) &&
                                (psamVoiceLPS2->sfxHandle==psamVoiceLPS->sfxHandle) )
                            {
                                if (psamVoiceLPS2->dwTimeStarted<dwTime)
                                {
                                    dwTime = psamVoiceLPS2->dwTimeStarted;
                                    lFoundVoice = lIndex2;
                                }
                            }
                        }

                        //Si on a trouvé, on se synchronise !
                        if (lFoundVoice!=-1)
                        {
                            psamVoiceLPS2 = &samVoiceLoopPlayStack[lFoundVoice];

                            //Lecture de la position
                            dwPosition      = 0;
                            dwPositionTick  = 0;

                            pSAM_Message (
                                SAM_MESSAGE_VOICE_POSITION_GET,
                                psamVoiceLPS2->dwVoiceHandle|SAM_VOICE_FASTACCESS,
                                (DWORD)&dwPosition );

                            pSAM_Message (
                                SAM_MESSAGE_VOICE_POSITIONTICK_GET,
                                psamVoiceLPS2->dwVoiceHandle|SAM_VOICE_FASTACCESS,
                                (DWORD)&dwPositionTick );

                            //Ecriture de la position
                            pSAM_Message (
                                SAM_MESSAGE_VOICE_POSITION_SET,
                                psamVoiceLPS->dwVoiceHandle|SAM_VOICE_FASTACCESS,
                                dwPosition );

                            pSAM_Message (
                                SAM_MESSAGE_VOICE_POSITIONTICK_SET,
                                psamVoiceLPS->dwVoiceHandle|SAM_VOICE_FASTACCESS,
                                dwPositionTick );

                            /*Com_Printf (
                                "  VoiceLPS => Set sfx(%d) voice(%d) at pos %d\n",
                                psamVoiceLPS->sfxHandle, psamVoiceLPS->dwVoiceHandle, dwPosition );*/
                        }



                    }
                    break;

                case 2: //Update
                    dwVoiceHandle   = psamVoiceLPS->dwVoiceHandle;

                    if (dwVoiceHandle==0xFFFFFFFF)
                        break;

                    pSAM_VOICE_SetDistanceLevel ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceLPS->fDistanceLevel );
                    pSAM_VOICE_SetRatioIIR      ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceLPS->fFilter );
                    pSAM_VOICE_SetOrigin        ( dwVoiceHandle|SAM_VOICE_FASTACCESS, psamVoiceLPS->lAngle, psamVoiceLPS->fDistanceMeters );

                    //Com_Printf( "Update: %d = Dist=%f\n", psamVoiceSPS->iEntityNumber, psamVoiceSPS->fDistanceMeters );

                    psamVoiceLPS->bUpdateMode = 0;
                    break;

                default:
                    break;
            }
        }
    }

    //Réactive temporairement la gestion des voix de SAM
    pSAM_Message ( SAM_MESSAGE_VOICE_SYNCFREEZE, 0, 0 );


    //Streaming
    pSAM_STREAM_SetState ( 1 );


    //Si le SMP est Off, alors on mixe dans la thread de Quake !
    //if (s_samEnableSMP->integer==0)
    //    pSAM_Message ( SAM_MESSAGE_PROCESS_MIXER, 0, 0 );

    IKA_WriteAudioToVideoRecording ( );
}


char IKA_QualitySound_szNoPathCharText[]="/\\:,?;!#&%*";
char IKA_QualitySound_sNoPathCharList[256];
long IKA_QualitySound_lInit = 0;
extern int fs_numServerPaks;

void * IKA_QualitySound_Load ( const char * pszFileName, snd_info_t * pInfo, float * pfReEqLevel )
{
    char szTextName[1024];
    char szTmp[1024];
    char szFullPath[1024];
    char szFullPathOGG[1024];
    char *pszFullPathLoad;
    long i;
    void * pData;
    //FILE * pFile;
    fileHandle_t fFile;
    long lNotFound;
    float fReEqLevel;

    //if (Cvar_VariableIntegerValue ( STR_ikalizer_ReplaceSamples )==0)
    {
        *pfReEqLevel = 0;
        return NULL;
    }

    if (!IKA_QualitySound_lInit)
    {
        IKA_QualitySound_lInit = 1;
        IKA_QualitySound_sNoPathCharList[0] = 0;
        for (i=1;i<256;i++)
        {
            if (strchr(IKA_QualitySound_szNoPathCharText,i)) IKA_QualitySound_sNoPathCharList[i] = '_';
            else IKA_QualitySound_sNoPathCharList[i] = (char)i;
        }
    }

    sprintf ( szTmp, pszFileName );
    for (i=0;szTmp[i];i++)
    {
        szTmp[i] = IKA_QualitySound_sNoPathCharList[(unsigned)szTmp[i]];
    }

    //Le nouveau nom
    sprintf ( szFullPath, "%s%s", IKA_QualitySoundPath, szTmp );
    strcpy ( szFullPathOGG, szFullPath );
    strcpy ( szFullPathOGG+strlen(szFullPathOGG)-3, "ogg" );

    //pFile = fopen ( szFullPath, "rb" );
    //FS_FOpenFileWrite ( szFullPath );

    //Com_Printf ( "IQS-IsExists:%s\n", szFullPath );
    lNotFound = 1;
    fReEqLevel = 0;

    //Teste la présence du fichier OGG
    if (FS_FileExists(szFullPathOGG)) pszFullPathLoad = szFullPathOGG;
    else if (FS_FileExists(szFullPath)) pszFullPathLoad = szFullPath;
    else pszFullPathLoad = NULL;

    if (pszFullPathLoad)// FS_FileExists(szFullPath))
    {
        char            szOldValueA[32];
        int                     fs_numServerPaks_Old;
        int             iFileLenght;
        char            *pszText;

        //Le fichier a été trouvé
        //Com_Printf ( "IQS-Found=>Open:%s\n", szFullPath );

        Cvar_VariableStringBuffer ( "fs_restrict", szOldValueA, 31 );
        Cvar_Set ( "fs_restrict", "0" );
        fs_numServerPaks_Old = fs_numServerPaks;
        fs_numServerPaks = 0;

        sprintf ( szTextName, "%s.cfg", szFullPath );
        if (FS_FileExists(szTextName))
        {
            FS_FOpenFileRead ( szTextName, &fFile, qtrue );
            if (fFile)
            {
                iFileLenght = FS_filelength ( fFile );
                pszText = Z_Malloc ( iFileLenght + 1 );
                memset ( pszText, 0, iFileLenght + 1 );

                FS_Read ( pszText, iFileLenght, fFile );
                FS_FCloseFile ( fFile );

                for (i=0;pszText[i];i++)
                {
                    if ((unsigned)pszText[i] < 31 ) pszText[i] = 0;
                }

                fReEqLevel = atof ( pszText );

                Z_Free ( pszText );

            }
        }



        pData = S_CodecLoad ( pszFullPathLoad, pInfo );

        fs_numServerPaks = fs_numServerPaks_Old;
        Cvar_Set ( "fs_restrict", szOldValueA );

        if (pData) lNotFound = 0;
    }

    if (lNotFound)
    {
        //Le fichier est introuvable
        strcat ( szFullPath, ".notfound" );
        //Com_Printf ( "IQS-NoFound=>Write:%s\n", szFullPath );
        fFile = FS_FOpenFileWrite ( szFullPath );
        if (fFile) FS_FCloseFile ( fFile );


        //pFile = fopen ( szFullPath, "wt" );
        //if (pFile) fclose ( pFile );

        //Com_Printf ( "IQS-Out:%s", szFullPath );
        pData = NULL;
    }

    if (pfReEqLevel) *pfReEqLevel = fReEqLevel;

    return pData;
}

void IKA_WriteAudioToVideoRecording ( void )
{
    WORD wBufferIn[44100];
    DWORD dwMCSamplesLength;
    DWORD dwMCSamplesCopied;
    DWORD dwMCSamplesAVIFrameDuration;
        DWORD dwMCSamplesNeededSingleCopy;
        DWORD dwMCSamplesTotalCopied;

    static long bStateLastVR = 0;
    static long bStateCurrVR = 0;
    long lStateVR;
    extern cvar_t       *com_maxfps;

    bStateLastVR = bStateCurrVR;
    bStateCurrVR = (CL_VideoRecording())?(1):(0);

    if ((bStateCurrVR==1)&&(bStateLastVR==0))
        lStateVR = 1;
    else if ((bStateCurrVR==0)&&(bStateLastVR==1))
        lStateVR = -1;
    else
        lStateVR = 0;

    if (cl_aviFrameRate->integer>0)
        dwMCSamplesAVIFrameDuration = dma.speed / cl_aviFrameRate->integer;
    else
        dwMCSamplesAVIFrameDuration = dma.speed / com_maxfps->integer;

    switch (lStateVR)
    {
        case 1: //VR goes enabled

            //Set iKALiZER in Quake Record Mode (and clear mixer buffer)
            pSAM_Message ( SAM_MESSAGE_MIXER_SET_RECORD2QUAKE, 1, dwMCSamplesAVIFrameDuration );
            break;

        case -1: //VR goes disabled

            //Set iKALiZER in Normal Mode
            pSAM_Message ( SAM_MESSAGE_MIXER_SET_RECORD2QUAKE, 0, 0 );
            break;
    }

        if (CL_VideoRecording())
        {
            dwMCSamplesLength   = floor ( 44100 / ( dma.channels * (dma.samplebits>>3) ) );

            dwMCSamplesNeededSingleCopy = dwMCSamplesAVIFrameDuration;
            if (dwMCSamplesNeededSingleCopy>dwMCSamplesLength) dwMCSamplesNeededSingleCopy = dwMCSamplesLength;

            dwMCSamplesTotalCopied = 0;
            do {
            dwMCSamplesCopied   = pSAM_Message ( SAM_MESSAGE_MIXER_OUTPUT_GET, dwMCSamplesNeededSingleCopy, (DWORD)wBufferIn );

            if (dwMCSamplesCopied>0)
            {
                        CL_WriteAVIAudioFrame( (byte *)wBufferIn, dwMCSamplesCopied * ( dma.channels * (dma.samplebits>>3) ) );
                        dwMCSamplesTotalCopied += dwMCSamplesCopied;
                    }
                    else pSAM_Message ( SAM_MESSAGE_PROCESS_MIXER, 0, 0 );

                } while (dwMCSamplesTotalCopied<dwMCSamplesAVIFrameDuration);
        }
}

//ioq3-urt, voip placeholders:
void                            S_SAM_StartCapture (void) { /*write me*/}
int                             S_SAM_AvailableCaptureSamples (void) { /*write me*/return 0; }
void                            S_SAM_Capture (int samples, byte *data) { /*write me*/}
void                            S_SAM_StopCapture (void) { /*write me*/}
void                            S_SAM_MasterGain (float gain) { /*write me*/}

#pragma GCC diagnostic pop
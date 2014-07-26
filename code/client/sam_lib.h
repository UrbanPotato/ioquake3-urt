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

#include <malloc.h>
#include <string.h>
#include <math.h>

//typedef unsigned char       BYTE;
//typedef unsigned short      WORD;
//typedef unsigned long       DWORD;
typedef unsigned __int64    QWORD;
typedef float               FLOAT;
//typedef float               FLOAT32;
//typedef double              FLOAT64;

typedef signed char         INT8;
typedef signed short        INT16;
//typedef signed long         INT32;
typedef signed __int64      INT64;
typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
//typedef unsigned long       UINT32;
typedef unsigned __int64    UINT64;

//typedef unsigned long       BOOL;


typedef struct {
    DWORD       dwTotalMemorySoundAllocCount;               //Mémoire totale pour les sons de SAM (en mega octets)
    DWORD       dwHardwareSamplingRate;                     //Fréquence d'échantillonnage du matériel (en Hz) - défaut:48000Hz
    DWORD       dwHardwareChannelsMode;                     //Mode de sortie audio sur 2x4bits => 0xXY, X=ChannelsCount, Y=RenderMode
    DWORD       dwHardwareBufferLatencyDuration;            //Latence du buffer matériel (en ms) - défaut:20ms
    DWORD       dwSoftwareBufferDuration;                   //Taille du buffer logiciel (en ms) - défaut:250ms
    DWORD       dwSoftwareVoicesCount;                      //Nombre de voies logicielles pouvant être lues en simultanées - défaut:64
    DWORD       dwSoftwareSFXCount;                         //Nombre d'effets pouvant être chargés en mémoire - défaut:4096
    DWORD       dwSoftwareResamplingQuality;                //Qualité du ré-échantillonnage 0...3 - défaut:0
    DWORD       dwStreamingBufferDuration;
    DWORD       dwProcessFlag;                              //Flag de process... Threads...
    DWORD       dwHardwareDeviceSelected;
} SAM_CONFIG;

#define SAM_INFO_VERSION                    0x0000
#define SAM_INFO_VENDOR                     0x0001
#define SAM_INFO_COPYRIGHT                  0x0002
#define SAM_INFO_TITLE                      0x0003
#define SAM_INFO_VERSION_DATA               0x0004
#define SAM_INFO_VERSION_BUILD              0x0005
#define SAM_INFO_MEMORY_TOTAL               0x0100
#define SAM_INFO_MEMORY_SFXFREE             0x0101
#define SAM_INFO_MEMORY_SFXUSED             0x0102
#define SAM_INFO_MEMORY_SFXTOTAL            0x0103
#define SAM_INFO_RENDER_MODE                0x0200
#define SAM_INFO_SYSTEM_INSTANCES           0x0300
#define SAM_INFO_SYSTEM_SIMD                0x0301
#define SAM_INFO_DEVICE_ENUM_00             0x0400
#define SAM_INFO_DEVICE_ENUM_63             0x043F
#define SAM_INFO_DEVICE_OUTPUT_MODEL        0x04FD
#define SAM_INFO_DEVICE_OUTPUT_FLAGS        0x04FE
#define SAM_INFO_DEVICE_GETCURRENT          0x04FF

#define SAM_INFO_SUPPORT_SELECT_DEVICE              0x1000
#define SAM_INFO_SUPPORT_MESSAGE_UNDEFINED          0x1001
#define SAM_INFO_SUPPORT_MESSAGE_VOICE_SYNCFREEZE   0x1002
#define SAM_INFO_SUPPORT_MESSAGE_VOICE_ISPLAYED     0x1003
#define SAM_INFO_SUPPORT_MESSAGE_VOICE_POSITION     0x1004

#define SAM_PROCESS_ENABLESMP               0x0003


#define SAM_MESSAGE_DEVICESETENUM           0x0000
#define SAM_MESSAGE_DYNAMICDELAYLINES       0x0001
#define SAM_MESSAGE_VOICE_SYNCFREEZE        0x0002
#define SAM_MESSAGE_VOICE_ISPLAYED          0x0003
#define SAM_MESSAGE_VOICE_VIRTUALCOUNT      0x0004
#define SAM_MESSAGE_VOICE_POSITION_GET      0x0005
#define SAM_MESSAGE_VOICE_POSITION_SET      0x0006
#define SAM_MESSAGE_VOICE_POSITIONTICK_GET  0x0007
#define SAM_MESSAGE_VOICE_POSITIONTICK_SET  0x0008
#define SAM_MESSAGE_PROCESS_MIXER           0x0009
#define SAM_MESSAGE_OUTPUT_FLUSH            0x000A
#define SAM_MESSAGE_RENDERMODE_SET          0x000B
#define SAM_MESSAGE_MAXUSAGE_SET            0x000C
#define SAM_MESSAGE_SOFTRESTART             0x000D
#define SAM_MESSAGE_SAMPLINGRATE_SET        0x000E
#define SAM_MESSAGE_DEVICESELECT_SET        0x000F
#define SAM_MESSAGE_LATENCYDURATION_SET     0x0010
#define SAM_MESSAGE_BUFFERDURATION_SET      0x0011
#define SAM_MESSAGE_LIMITERLEVEL_SET        0x0012
#define SAM_MESSAGE_SFX_DEFAULTLEVEL_SET    0x0013
#define SAM_MESSAGE_LATENCYDURATION_BU_GET  0x0020
#define SAM_MESSAGE_MIXER_OUTPUT_GET        0x0030
#define SAM_MESSAGE_MIXER_OUTPUT_KEEP       0x0031
#define SAM_MESSAGE_MIXER_SET_RECORD2QUAKE  0x0032

#define SAM_MESSAGE_UNDEFINED               0xFFFFFFFF

#define SAM_VOICE_MASK                      0x7FFFFFFF
#define SAM_VOICE_FASTACCESS                0x80000000


long    SAM_Open ( void * pDeviceParam, SAM_CONFIG * psamConfig );
long    SAM_Close ( void );
long    SAM_LimiterSet ( long lMode );
long    SAM_GetInfo ( DWORD dwInfoID, DWORD * pdwOutData, char * pszOutData );

long    SAM_Message ( DWORD dwMessage, DWORD dwParamA, DWORD dwParamB );


long    SAM_SFX_Load ( DWORD dwHandle, char * pszSFXName, DWORD dwSampleRate, DWORD dwSamplesCount, BYTE bStereo, BYTE b16bits, void * pAudioData, DWORD * pdwAllocatedHandle );
long    SAM_SFX_Unload ( DWORD dwHandle );
long    SAM_SFX_Free ( DWORD dwHandle );
long    SAM_SFX_IsLoaded ( char * pszSFXName, DWORD * pdwAllocatedHandle );
long    SAM_SFX_GetLoadedMemoryState ( DWORD dwHandle, DWORD * pdwLoadedMemoryState );
long    SAM_SFX_GetOldest ( DWORD * pdwOldestHandle );

long    SAM_VOICE_Alloc ( DWORD * pdwVoiceHandle, long lForceAlloc, DWORD * pdwKilledVoiceHandle );
long    SAM_VOICE_AllocByVoiceHandle ( DWORD dwVoiceHandle, long lForceAlloc );
long    SAM_VOICE_AllocByUserID ( DWORD dwUserID, long lForceAlloc );
long    SAM_VOICE_Free ( DWORD dwVoiceHandle );
long    SAM_VOICE_FreeByUserID ( DWORD dwUserID );
long    SAM_VOICE_GetHandleByUserID ( DWORD * pdwVoiceHandle, DWORD dwStartVoiceHandle, DWORD dwUserID );

long    SAM_VOICE_SetSFX ( DWORD dwVoiceHandle, DWORD dwHandleSFX );
long    SAM_VOICE_SetSampleRate ( DWORD dwVoiceHandle, DWORD dwSampleRate_Hz );
long    SAM_VOICE_SetLoop ( DWORD dwVoiceHandle, long bLoop );
long    SAM_VOICE_SetMasterLevel ( DWORD dwVoiceHandle, float fMasterLevel_dB );
long    SAM_VOICE_SetDistanceLevel ( DWORD dwVoiceHandle, float fDistanceLevel_dB );
long    SAM_VOICE_SetRatioIIR ( DWORD dwVoiceHandle, float fRatioIIR );
long    SAM_VOICE_SetPlay ( DWORD dwVoiceHandle, long bPlay );
long    SAM_VOICE_SetAngle ( DWORD dwVoiceHandle, long lAngleDegrees );

long    SAM_VOICE_GetUserData ( DWORD dwVoiceHandle, BYTE * pbReceiveBuffer, DWORD dwByteCount );
long    SAM_VOICE_SetUserData ( DWORD dwVoiceHandle, BYTE * pbSendBuffer, DWORD dwByteCount );
long    SAM_VOICE_GetUserID ( DWORD dwVoiceHandle, DWORD * pdwUserID );
long    SAM_VOICE_SetUserID ( DWORD dwVoiceHandle, DWORD dwUserID );

long    SAM_VOICE_GetVoiceUsedCount ( DWORD * pdwVoiceTotalUsedCount, DWORD * pdwVoiceUsedCountUnlooped, DWORD * pdwVoiceUsedCountLooped );


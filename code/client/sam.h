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

#ifdef MSVC
#endif

//#define                     _SSE2_

//Define
#define                     sam_PI		                3.1415926535897932384626433832795
#define                     sam_ABS(a)                  ((a<0)?(-a):(a))
//#define                     sam_FIR_ORDER_MAX           128
//#define                     sam_FIR_ORDER_MASK          127

#define                     sam_CHBUFFER_SAVE_COUNT     1024
#define                     sam_CHBUFFER_SAVE_MASK      1023

#define                     sam_POLYCHANNEL_COUNT       32
#define                     sam_POLYCHANNEL_MASK        31


#define                     samKERNEL_ALLOC(n)      malloc(n)
#define                     samKERNEL_REALLOC(p,n)  realloc(p,n)
#define                     samKERNEL_FREE(p)       free(p)


typedef char		        int8;
typedef unsigned char       uint8;
typedef short               int16;
typedef unsigned short      uint16;
typedef long                int32;
typedef unsigned long       uint32;
typedef float               float32;
typedef double              float64;
typedef unsigned char       Byte;
typedef unsigned short      Word;
typedef unsigned long       DWord;
typedef float               FLOAT32;
typedef double              FLOAT64;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;

typedef struct
{
    //long                    lUsedAngle;             // Angle used in processing
    //float                   fDistance;              // Distance from object
    long                    lDelayCount;            // 0...32
    long                    lDelayIndex[32];        // 32 Delay index in samples
    float                   fDelayValue[32];        // 32 Delay value of sample
    long                    lDelayMixCh[32];        // 32 Delay mixing channel
} sam_render_t;

typedef struct
{
    long                    bIsFree;                // This sfx is free
    char                    szFileName[MAX_QPATH];
    void                    *pBuffer;
    long                    bIsLoaded;              // Sound is loaded in pBuffer (is pBuffer is allocated)
    long                    lSizeInBytes;           // Sound size in bytes
    long                    lSizeInSamples;         // Sound size in samples
    long                    lBitsPerSample;         // Bits per sample
    long                    lBytesPerSample;        // Bytes per sample
    float                   fBaseSampleRate;        // Base sample rate in Hertz
    //float                   fBaseAbsoluteLevel;     // Base absolute level to convert to real world (in dB)
    float                   fCompensationGain;     // Compensation gain
    long                    lStoreMode;             // Store mode 0 = 32 bits float, 1 = XPCM
    long                    lInUseCount;            // Usage counter 0 = if not in use, >=1 if used
    long                    lAllocTime;             // Time when this sfx was created in ms
} sam_sfx_t;
 
typedef struct
{
    char                    szIntroFileName[MAX_QPATH];
    char                    szLoopFileName[MAX_QPATH];
    char                    *pszCurrentFileName;
    void                    *pBuffer;

    long                    bEnableIntro;
    long                    bEnableLoop;

    long                    lStateReadIntroLoop;    // State of reading intro and loop (0 = off, 1 = intro, 2 = loop)

    snd_stream_t            *sndStream;

} sam_stream_t;

typedef struct
{
    long                    bIsFree;                // This channel is free
    long                    bIsPlayed;              // Currently played if true
    long                    bNeedToBeKilled;        // This channel need to be killed
    long                    bNeedToSlotUpdate;      // This channel will be played on the next update slot
    long                    lAllocTime;             // Time when this sfx was created in ms
    sam_sfx_t               *pSFX;                  // SFX data
    sfxHandle_t             sfxHandle;              // Handle of the sfx used

    long                    lLoopMode;              // 0 = noloop, -1 = infinite loop
    int                     iEntityNumber;
    int                     iEntityNumberStandAlone;
    //int                     iEntityChannel;

    long                    bIsOrigin;              // vec3Origin is used if true
    long                    bIsVelocity;            // vec3Velocity is used if true
    vec3_t		            vec3Origin;
	vec3_t		            vec3Velocity;

    float                   fLevelMasterGain;       // Master level gain in dB
    float                   fLevelDistanceGain;     // Distance level gain in dB (-oo to 0dB)
    long                    lAngleView;             // Angle of sfx

	long                    bIsDoppler;             // Doppler is used if true
	float		            fDopplerScale;
	float		            fOldDopplerScale;


    uint32                  ui32TickPositionDecimal;    // Position in decimal part (24 bits used)
    float                   fTickPositionIncrement;     // Increment of the position on each sample process
    long                    lTickPosition;              // Position in the sfx
    float                   fAdditionnalLPRatio;        // Additionnal low pass ratio for FIR
    float                   fDistanceLowPassRatio;      // Low pass ratio filter (for distance filtering)
    float                   fDistanceLowPassValue;      // Low pass value filter (for distance filtering)

    float                   fBufferSaveDataLast[sam_CHBUFFER_SAVE_COUNT];  // saved samples (sam_CHBUFFER_SAVE_COUNT points)
    long                    lBufferSaveCurrentPosition; // Current save position


    float                   fFIRCoef[256];              // Coef of FIR A filter (128coef)
    float                   fFIRLast[128];              // Last data for A filter
    long                    lFIRPosition;
    float                   fFIRRatio;
    long                    lFIRTableRatioIndex;        // Ratio index
/*
    float                   fLowPassValue;
    float                   fLastSample;
*/
    sam_render_t            *psamRender;

} sam_channel_t;




    #define         SAM_NO_ERROR                    0
    #define         SAM_INVALID_NAME                -2
    #define         SAM_INVALID_ENUM                -3
    #define         SAM_INVALID_VALUE               -4
    #define         SAM_INVALID_OPERATION           -5
    #define         SAM_OUT_OF_MEMORY               -6


    // 960 Kbytes of samples data in mixing buffer
    // 122880 samples of 2-channel buffer   FL/FR               2.0
    //  61440 samples of 4-channel buffer   FL/FR/RL/RR         4.0
    //  49152 samples of 5-channel buffer   FL/FC/FR/RL/RR      5.0
    //  40960 samples of 6-channel buffer   FL/FC/FR/RL/RR/BC   6.0
    #define         SAM_MIXING_BUFFERCOUNT          245760

typedef struct {
    float                   fLimiterGain;           //0..1
    float                   *pfMixingBuffer;
    long                    lMixingChannelCount;
    float                   fSampleRate;
    long                    lMixingSizeInMCSample;
    long                    lMixingSizeInBytes;
    long                    lMixingSentSamples;

    void                    *pRawBufferData;        // Output buffer
    long                    lRawSizeInBytes;        // Size of output buffer
    long                    lRawBytesPerSample;     // Bytes in each individual sample
    long                    lRawChannelCount;       // Number of channels
    long                    lRawSizeInMCSample;   // Number of multi-channel samples

    long                    lRawPositionCurrent;    // Position in MCSamples
    long                    lRawPositionLast;       // Position in MCSamples
    
    long                    lRawPositionCopyCurrent;

    long                    lTimeCurrent;           // Current time to mix
    long                    lTimeLast;              // Last time of mix process
    
} sam_mix_t;

typedef struct {

    long                    lSurround;              //s_samSurround
    float                   fDistSpk;               //s_samDistSpk
    float                   fDistEars;              //s_samDistEars
    long                    lFIRQuality;            //s_samFIR

} sam_usedsettings_t;

typedef struct {
    long                    bIsStarted;              // SAM is started

    //SFX mixer (only mono support like reality)
    sam_sfx_t               *psfxData;              // SFX data
    long                    lSFXCount;              // Number of SFX

    //Stream mixer (all multi-channels format supported)
    sam_stream_t            *pstreamData;
    long                    lStreamCount;

    
    
    //Mixing buffer
    sam_mix_t               mixData;                // Mixer data
    

    sam_channel_t           *pChannelData;          // Channel data
    long                    lChannelCount;

    //Misc
    int                     iListenerEntityNumber;
    vec3_t                  vec3ListenerOrigin;
    vec3_t                  vec3ListenerAxis[3];

    long                    lPolyChannelEntry[sam_POLYCHANNEL_COUNT];
    long                    lPolyChannelIndex;
    
    
    sam_usedsettings_t      UsedSettings;  
    sam_render_t            *pDefaultRender;
    
} sam_data_t;




/*
    Fast and short pack mode

        0 = Linear PCM in 8 or 16 bits
        1 = eXponential PCM (like a/muLaw)
        2 = Differential Multi Sample Pack

            16 bits = 4 samples

            1 x 4 bits = DC offset
            4 x 3 bits = Delta sample n[0...3]

            Coded DC offset = -8..-1..+1..+8
            DC_offset = 64<<abs(c_dc) * sgn(c_dc)

            Coded Delta Sample = -3..-1..+1..+3
            Sample[n] = DC_offset + 11 * c_delta[n]
            
                        

        

*/


    #define IKA_QualitySoundPath    "iqs/"
    


    //External usage
    void            S_SAM_Shutdown                          ( void );
    qboolean        S_SAM_Init                              ( soundInterface_t * psi );
    void            S_SAM_SoundList                         ( void );
    void            S_SAM_SoundInfo                         ( void );
    void            S_SAM_RawSamples                        ( int stream, int iSamples, int iSampleRate, int iWidth, int iChannelCount, const byte * pbData, float f32VolumeMul );
    void            S_SAM_BeginRegistration                 ( void );
    sfxHandle_t     S_SAM_RegisterSound                     ( const char * pszFileName, qboolean bCompressed );
    void            S_SAM_StartSound                        ( vec3_t vec3Origin, int iEntityNumber, int iEntChannel, sfxHandle_t sfxHandle );
    void            S_SAM_StartLocalSound                   ( sfxHandle_t sfxHandle, int iChannelNum );
    void            S_SAM_StartBackgroundTrack              ( const char * pszIntroFileName, const char * pszLoopFileName );
    void            S_SAM_StopBackgroundTrack               ( void );
    void            S_SAM_ClearSoundBuffer                  ( void );
    void            S_SAM_StopAllSounds                     ( void );
    void            S_SAM_ClearLoopingSounds                ( qboolean bKillAll );
    void            S_SAM_StopLoopingSound                  ( int iEntityNumber );
    void            S_SAM_DisableSounds                     ( void );
    void            S_SAM_AddLoopingSound                   ( int iEntityNumber, const vec3_t vec3Origin, const vec3_t vec3Velocity, sfxHandle_t sfxHandle );
    void            S_SAM_AddRealLoopingSound               ( int iEntityNumber, const vec3_t vec3Origin, const vec3_t vec3Velocity, sfxHandle_t sfxHandle );
    void            S_SAM_Respatialize                      ( int iEntityNumber, const vec3_t vec3Head, vec3_t vec3Axis[3], int iInWater );
    void            S_SAM_UpdateEntityPosition              ( int iEntityNumber, const vec3_t vec3Origin );
    void            S_SAM_Update                            ( void );
    //ioq3-urt, voip placeholders:
	void				S_SAM_StartCapture (void);
	int				S_SAM_AvailableCaptureSamples (void);
	void				S_SAM_Capture (int samples, byte *data);
	void				S_SAM_StopCapture (void);
	void				S_SAM_MasterGain (float gain);

    void            SAM_SpatializeFromOrigin                ( vec3_t vec3Origin, float fSourceSize, long iInWater, float * pfLevel, float * pfFilter, long * plAngle, float * pfDistance );


    const char     *S_SAM_ErrorMessage                      ( long lError );
    
    void            S_SAM_DeviceEnum                        ( void );

    
    void            S_SAM_Help ( void );
    void            S_SAM_Restart ( void );
    void            S_SAM_LoadSound ( char * pszFileName, DWORD dwHandle );
    void            S_SAM_SetVolume ( void );
    
    void *          IKA_QualitySound_Load ( const char * pszFileName, snd_info_t * pInfo, float * pfReEqLevel );
    long            IKA_SampleFormatWidthToBits ( long lWidth );
    
    void            IKA_WriteAudioToVideoRecording ( void );

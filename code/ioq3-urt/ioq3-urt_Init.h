//under GPL v2

void Sys_MicroGranularityCheck (void);

void set_envs(void);

void Com_Sysexec_f (void);

qboolean Com_quiet_ugly_hack(char* txt);

void DNS_workaround (void);

qboolean FTGL_Init (void);
void Unicode_FindSlot (char* Message, int FadeTimeout);
void Unicode_ChatOutput_f (char* Message);
void Unicode_Init (void);
void Unicode_Render (void);
void Unicode_Shutdown (void);

void Com_Translate_Init(void);
void Com_Translate_Auto (char *input);
void Com_Translate_DispatchMessages(void);

void Com_Translate_f (void);

void Com_SleepCheck (void);

void R_MotionBlur_f (void);

void CL_BindTeleport_f (void);

void Key_Rebind_f (void);

void R_Blank (void);
void R_Blank_f (void);

void Com_Totaltimerun_f (void);
void Com_Totaltimerun_save (void);

void one_pk3(void);

void PreCache_UT (void);

void CL_Autotimenudge (void);

void CL_Autotimenudge_f(void);

void cl_utradio (void);

void cl_stopwatchreset (void);
void cl_stopwatchstartpause (void);

void SCR_ioq3_urt (void);

float CL_Zoom_Sens (void);
void R_Zoom_Sens (viewParms_t *dest);

void reconnect_workaround (char *txt);
void CL_Reconnect_f_clu (void);

void IN_RawInit (void);
void IN_RawDeregister (void);
void init_WndProc (void);
void IN_RawProcess (void);

void SDL_EnableKeyRepeatWin (void);
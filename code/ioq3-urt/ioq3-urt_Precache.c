// under GPL v2

#if !defined(DEDICATED) 

	#include "ioq3-urt.h"
	#include "../client/snd_local.h"

	/*	Precache the radio sounds + some other sounds of urt 'cause it doesn't on its own and it
		may be freezing rendering in the process of fread()ing assets for the 1st time. */
	static void PreCache_UT_Audio (void) {

		//lister stolen from UI_LoadBots()
		char	*		dirptr;
		char			dirlist[8192],filename[1024];
		int			numdirs,dirlen, i;

		numdirs = FS_GetFileList("sound/radio/female", ".wav", dirlist, 8192);
		dirptr  = dirlist;
		for (i = 0; i < numdirs; i++, dirptr += dirlen+1) {
			dirlen = strlen(dirptr);
			strcpy(filename, "sound/radio/female/");
			strcat(filename, dirptr);
			S_RegisterSound(filename, qfalse);
		}

		numdirs = FS_GetFileList("sound/radio/male", ".wav", dirlist, 8192);
		dirptr  = dirlist;
		for (i = 0; i < numdirs; i++, dirptr += dirlen+1) {
			dirlen = strlen(dirptr);
			strcpy(filename, "sound/radio/male/");
			strcat(filename, dirptr);
			S_RegisterSound(filename, qfalse);
		}

		//some extra ones caught not being pre-registered:
		S_RegisterSound ("sound/misc/kcaction.wav", qfalse);
		S_RegisterSound ("sound/player/ledgegrab.wav", qfalse);
		S_RegisterSound ("sound/surfaces/bullets/water1.wav", qfalse);
		S_RegisterSound ("sound/items/nvgoff.wav", qfalse);
		S_RegisterSound ("sound/items/nvgon.wav", qfalse);
		S_RegisterSound ("sound/items/flashlight.wav", qfalse);
		S_RegisterSound ("sound/items/laseronoff.wav", qfalse);
		S_RegisterSound ("sound/weapons/beretta/92G_noammo.wav", qfalse);
		S_RegisterSound ("sound/bomb/Bomb_disarm.wav", qfalse);
		S_RegisterSound ("sound/bomb/Explode01.wav", qfalse);
		S_RegisterSound ("sound/misc/blast_wind.wav", qfalse);
		S_RegisterSound ("sound/misc/blast_fire.wav", qfalse);
	}

	//similarly, pre-cache funstuff
	static void PreCache_UT_Models (void) {
		char*		dirptr;
		char			dirlist[8192],filename[1024];
		int			numdirs,dirlen, i;

		numdirs = FS_GetFileList("models/players/funstuff/", ".md3", dirlist, 8192 );
		dirptr  = dirlist;
		for (i = 0; i < numdirs; i++, dirptr += dirlen+1) {
			dirlen = strlen(dirptr);
			strcpy(filename, "models/players/funstuff/");
			strcat(filename, dirptr);
			RE_RegisterModel (filename);
		}
	}

	// Do the precaching once per run in CL_InitCGame()
	void PreCache_UT (void) {
		static qboolean done = qfalse;
		if (!done) {
			PreCache_UT_Audio();
			PreCache_UT_Models();
			done = qtrue;
		}
	}	

#endif

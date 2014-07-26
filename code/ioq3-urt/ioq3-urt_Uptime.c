// under GPL v2

#if !defined(DEDICATED) 

	#include "ioq3-urt.h"

	//for uint32_t etc. (overkill compliance ftw):
	#if defined(_MSC_VER) && (_MSC_VER <= 1500) //MSVC <= 2008 doesn't have stdint.h
		#include "msvc08_stdint.h"	
	#else
		#include <stdint.h> 
	#endif

	/*
	report total time run
	Notice that it relies on renderer starts/stops, its printing command, or quit to run its saving function. 
	Periodically saving would be bad because of disk writes; a mod version could potentially do it prettier.

	Earliest point this can  be for an auto-run, is right after Com_ExecuteCfg() in Com_Init().

	uint32_t and strtoul() are used (working as intented in most systems) for 136.2 years of recording in seconds.
	64bit support would be possible but a mess of #ifdefs since the printing and saving methods to do it
	aren't standardized across systems and their 32 and 64 bit versions.

	TODO: Server support. Difficulty may arise for multiple server support. This is already a quirk on client.
	*/
	void Com_Totaltimerun_f(void) {
		uint32_t time,x,total,h,min,s,d,ht,mint,st,dt;
		static short int first_time = qfalse; //to print it thin first time on Com_init()

		//not strictly required, but for the extra efficiency; run normally on quit and renderer inits.
		//Do not run periodically since disk read/writes are very bad during gameplay.
		clu.Com_Totaltimerun_save();

		total = strtoul(clu.com_totaltimerunsave->string, NULL, 0); //it's total since we just run (the previously non-required) totaltimerunsave()
		time = Sys_Milliseconds() / 1000; //time since current session started in seconds
	 
		//left milliseconds out since it would need higher than 32bit unsigned and it's an overkill anyway.
		//thx to Greg Hewgill for teh moduli
		//mst	=	total % 1000;
  		x 	=	total; // / 1000;
		st 	=	x % 60;
		x 	/=	60;
		mint	=	x % 60;
		x 	/=	60;
		ht 	= 	x % 24;
		x 	/= 	24;
		dt 	= 	x;

		//ms	=	time % 1000;
  		x 	=	time; // / 1000;
		s 	=	x % 60;
		x 	/=	60;
		min 	=	x % 60;
		x 	/=	60;
		h 	= 	x % 24;
		x 	/= 	24;
		d 	= 	x;
	
		if(first_time)
			Com_Printf(	"running for %idays %ih %im and %is\n"
						"now running for %idays %ih %im and %is\n",dt,ht,mint,st,d,h,min,s);
		else
			Com_Printf(	"running for %idays %ih %im and %is\n",dt,ht,mint,st);

		first_time = qtrue;
	}

	/*totaltimerun's saving
	Do not run periodically since disk read/writes are very bad during gameplay.
	It has to run in some ugly parts, renderer start/stops and quit to avoid disk writing during gameplay (which is bad).
	Game code could potentially do it prettier.
	TODO/FIXME: multiple clients on same user aren't taken into account
	The cvar is CVAR_ARCHIVE and not CVAR_INIT (to potentially 'write protect' it) since the later produced quirky bahavior, 
	but even if the user changes it manually on console it has no effect on the config or even the functions; to actually
	change it one has to edit the config.*/
	void Com_Totaltimerun_save(void) {
		static unsigned int saved; //remember what is disk saved; unsigned is properly supported, don't remove.

		//ask for the var initially; that's the only time 'saved' is set, rest saving is done based on this and Sys_Milliseconds() only
		if (!saved)
			saved = strtoul(clu.com_totaltimerunsave->string, NULL, 0); 
	
		//TODO: live-changing of the var (but it could be considered a feature, we're keeping total time, it's not supposed to be often edited)
    
		//set the var and save it to disk
 		Cvar_Set("cl_totaltimerunsave", va("%u", (Sys_Milliseconds() / 1000) + saved)); //cl_ is legacy, it should be com_
	}


#endif

// under GPL v2

#if !defined(DEDICATED) && defined(WIN32)

	#include "ioq3-urt.h"
	#include "SDL.h"

	/*	Approximation of Delay/Speed(Interval) keyboard values on Windows.
		SDL apparently does not utilize them directly, only supporting the use of SDL_EnableKeyRepeat().  */

	//rint() for MSVC from Ethereal: 
	#if !defined(__MINGW32__)

		static int rint (double x) {
			char *buf;
			int i,dec,sig;
		
			buf = _fcvt(x, 0, &dec, &sig);
			i = atoi(buf);
			if(sig == 1) i = i * -1;

			return(i);
		}
	#endif

	void SDL_EnableKeyRepeatWin (void) {
		
		static UINT delay, speed; /*		There were overflows on short int (after the point of calling SystemParametersInfo)
										only on debug builds because SystemParametersInfo requires UINT */
		 
		//don't do it more than once during rendering, to save cycles
		//it is redone after Key_SetCatcher() calls (clu.keyboard_done is reset)
		if (!clu.keyboard_done) {
			SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &delay, 0);
			SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &speed, 0);
			clu.keyboard_done = qtrue;

			//actual values on windows vary depending on hardware
			
			//WINAPI: "0 (approximately 250 ms delay) through 3 (approximately 1 second delay)"
			delay = 250 + delay * 250;
			
			//WINAPI: "0 (approximately 2.5 repetitions per second)[400ms] through 31 (approximately 30 repetitions per second)[33ms]"
			speed = rint(400.00f - (float)speed * 11.84f);
		}
		
		SDL_EnableKeyRepeat(delay, speed);
	}	

#endif

// under GPL v2

#if !defined(DEDICATED) 

	#include "ioq3-urt.h"

	void SCR_DrawMouse (void);
	void SCR_DrawPing (void); 
	void SCR_DrawFPS (void);
	void SCR_DrawSnaps (void); 
	void SCR_DrawPackets (void);
	void SCR_DrawClock (void);
	void SCR_StopWatch (void);

	//collect ioq3-urt scr stuff
	void SCR_ioq3_urt (void) {
		if (clu.mouse.cl_drawmouse->integer)		SCR_DrawMouse();	
		if (clu.cl_drawping->integer)			SCR_DrawPing();
		if (clu.cl_drawfps->integer)				SCR_DrawFPS();
		if (clu.cl_drawsnaps->integer)			SCR_DrawSnaps();
		if (clu.cl_drawpackets->integer)			SCR_DrawPackets();
		if (clu.cl_drawclock->integer)			SCR_DrawClock();
		if (clu.cl_stopwatch->integer)			SCR_StopWatch();
		//***
		//not allowed
		//SCR_DrawStringExt( 317, 236, 6, ".", g_color_table[5], qtrue,qtrue);
		//***
	}	

#endif

// under GPL v2

#if !defined(DEDICATED) 

	#include "ioq3-urt.h"
	#include "../client/client.h"

	/*
	=================
	SCR_StopWatch - Draw a Stopwatch based on q3 milliseconds timer (affected by timescale)
	=================
	*/

	//to make Stopwatch code readable:
	#define ZERO		1
	#define PLAY		2
	#define PAUSE	3
	#define ZEROED 	1
	#define PLAYING	2
	#define PAUSED	3

	typedef struct { //could be in clu.
		int cmd;
		int state;
	} stopwatch_s;

	stopwatch_s sw;

	void cl_stopwatchreset (void) {
		sw.cmd = ZERO;
	}

	void cl_stopwatchstartpause (void) {
		if (sw.cmd == PLAY) 
			sw.cmd = PAUSE;
		else
			sw.cmd = PLAY;
	}

	void SCR_StopWatch (void) {
		static unsigned long long int 	time, last_recorded, time_paused;
		static int 						ms, sec, min, h;
		char								stopwatch[1024]; //fix its size, again
		int								posx, posy, posxx, posyy, size;

		posx	= clu.cl_stopwatchposx->integer;
		posy	= clu.cl_stopwatchposy->integer;
		size	= clu.cl_stopwatchsize->integer;

		//using cls.realtime - affected by /timescale - on purpose

		if 			(sw.cmd == ZERO || !sw.cmd) { // || initially
			time				=	0;
			last_recorded	=	cls.realtime; //to allow for zeroed realtime
			sw.state 		=	ZEROED;
			time_paused		=	0;
		} else if	(sw.cmd == PLAY) {
			time				=	time_paused + cls.realtime - last_recorded;
			sw.state			=	PLAYING;
		} else if	(sw.cmd == PAUSE && sw.state == PLAYING) {
			//time remains unchanged from here on
			time_paused		=	time;
			last_recorded	=	cls.realtime;
			sw.state			=	PAUSED;
		} else if 	(sw.cmd == PAUSE && sw.state == PAUSED) {
			last_recorded	=	cls.realtime; //keep track of time when paused for later use
		}

		//cheers to gigahertz205
		//don't make it unreadable with resultant numbers, that's what compilers are for
		h 	=   time / (1000*60*60);
		min	=  (time % (1000*60*60)) / (1000*60);
		sec	= ((time % (1000*60*60)) % (1000*60)) / 1000;
		ms 	= ((time % (1000*60*60)) % (1000*60)) % 1000;

		Com_sprintf( stopwatch, sizeof ( stopwatch ), "%0i:%02i:%02i:%03i",h,min,sec,ms);
	
		if (posx >= 0 && posx < 21)
			posxx = posx * 30;
		else
			posxx = 0;

		if (posy >= 0 && posy < 24) 
			posyy = posy * 20;
		else 
			posyy = 0;

		SCR_DrawStringExt(posxx, posyy, size, stopwatch, g_color_table[3], qtrue, qtrue);
	}

#endif

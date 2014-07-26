// under GPL v2

#if !defined(DEDICATED) 
	
	#include "../qcommon/q_shared.h"
	#include "ioq3-urt.h"
	#include "../client/client.h"

/*
	=================
	SCR_DrawClock - draw a clock of OS real time on screen; allow the user to choose whether to show seconds; alarm function.
	=================
	*/

	void SCR_DrawClock (void) { //some credit to iourt
		qtime_t		myTime;
		char			clock[32], alarm[32], alarmh[32], alarmm[32], source[32], colon_check[32]; 
		int			posx, posxx, posy, posyy, fontsize, color;
		static int	alarm_went_off, alarmhi, alarmmi; //last two here to avoid unitialized warning
		qboolean alarm_ok;

		color = clu.cl_drawclockcolor->integer;
		if (color > 8) color = 8;
		posx = clu.cl_drawclockposx->integer;
		posy = clu.cl_drawclockposy->integer;
		fontsize = clu.cl_drawclockfontsize->integer;
		Com_RealTime( &myTime );
		strcpy(source,clu.cl_drawclockalarm24->string);

		if (strlen(source) == 5) { //check if it's five chars XXXXX

			strncpy(alarmh,source, 2);
			alarmh[2] = '\0'; //technically required; probably faster than Q_strncpyz
			alarmhi = atoi(alarmh);
			strncpy(alarmm,source + 3, 2);
			alarmm[2] = '\0';
			alarmmi = atoi(alarmm);

			strncpy(colon_check,source + 2, 1); //for checking if a colon is used XX:XX
			colon_check[1] = '\0';

			if (alarmhi >= 0 && alarmhi < 24 && alarmmi >= 0 && alarmmi <= 59 && !strcmp(colon_check,":")){
				alarm_ok = qtrue; //now you can print stuff
				if (alarmhi == myTime.tm_hour && alarmmi == myTime.tm_min && myTime.tm_sec == 0) {		
					if (!alarm_went_off) { //to avoid repeating the alarm the whole second
						Cbuf_ExecuteText (0,clu.cl_drawclockalarmcmd->string); //alarm!1
						alarm_went_off = 1; 
					}
				} else {
					alarm_went_off = 0;				
				}
			} else {
				alarm_ok = qfalse;
			}
		} else {
			alarm_ok = qfalse; //this is needed here too because if we unset alarm it will usually retain 1
		}
		
		if (clu.cl_drawclockshowseconds->integer) {
			if (clu.cl_drawclock12->integer) {
			
				if			( myTime.tm_hour > 12)
					Com_sprintf( clock, sizeof ( clock ), "%02i:%02i:%02i PM", myTime.tm_hour - 12, myTime.tm_min, myTime.tm_sec);
				else if	( myTime.tm_hour == 12) 
					Com_sprintf( clock, sizeof ( clock ), "%02i:%02i:%02i PM", 12, myTime.tm_min, myTime.tm_sec); 
				else if (myTime.tm_hour == 0) // 12h clock needs 12a.m at midnight 
					Com_sprintf( clock, sizeof ( clock ), "%02i:%02i:%02i AM", 12, myTime.tm_min, myTime.tm_sec);
				else
					Com_sprintf( clock, sizeof ( clock ), "%02i:%02i:%02i AM", myTime.tm_hour, myTime.tm_min, myTime.tm_sec);
					
			
			} else {
				Com_sprintf( clock, sizeof ( clock ), "%02i:%02i:%02i", myTime.tm_hour, myTime.tm_min, myTime.tm_sec);
			}
		} else {
			if (clu.cl_drawclock12->integer) {	
			
				if			( myTime.tm_hour > 12) 
					Com_sprintf( clock, sizeof ( clock ), "%02i:%02i PM", myTime.tm_hour - 12, myTime.tm_min);
				else if	( myTime.tm_hour == 12) 
					Com_sprintf( clock, sizeof ( clock ), "%02i:%02i PM", 12, myTime.tm_min); 
				else if (myTime.tm_hour == 0) // 12h clock needs 12a.m at midnight 
					Com_sprintf( clock, sizeof ( clock ), "%02i:%02i AM", 12, myTime.tm_min);
				else
					Com_sprintf( clock, sizeof ( clock ), "%02i:%02i AM", myTime.tm_hour, myTime.tm_min);
			
			} else {
					Com_sprintf( clock, sizeof ( clock ), "%02i:%02i", myTime.tm_hour, myTime.tm_min);
			}
		}
	
		if (alarm_ok)
			Com_sprintf( alarm, sizeof ( alarm ), "  Alarm %02i:%02i", alarmhi,alarmmi);
	  
		if (posx >= 0 && posx < 21)
			posxx = posx * 30;
		else 
			posxx = 0;
	
		if (posy >= 0 && posy < 43) 
			posyy = posy * 11;
		else
			posyy = 0;
	 
		SCR_DrawStringExt( posxx, posyy, fontsize, clock, g_color_table[color], qtrue,qtrue);
		if (alarm_ok) {
		
			if (alarm_went_off)
				SCR_DrawStringExt( posxx + fontsize * 10, posyy, fontsize, alarm, g_color_table[1], qtrue,qtrue);
			else 
				SCR_DrawStringExt( posxx + fontsize * 10, posyy, fontsize, alarm, g_color_table[3], qtrue,qtrue);
		
		}
	}

#endif
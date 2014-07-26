// under GPL v2

#if !defined(DEDICATED) 
	
	#include "ioq3-urt.h"	
	#include "../client/client.h"
	unsigned long long Sys_Microseconds (void);
	
	//draw mouse stats
	void SCR_DrawMouse (void) {
		int ups = 0, systemtime = Sys_Milliseconds(), posxx, posyy;
		static int thiscount, prv_time, avg, dpsx, dpsy, sumx, sumy, maxx, maxy;
		char s1[32], s2[32], s3[32], s4[32], s5[32], s6[32], s7[32], source[16];
		static unsigned long long sum;

		float interval	= clu.mouse.cl_drawmouseinterval->value;
		int posx			= clu.mouse.cl_drawmouseposx->integer;
		int posy			= clu.mouse.cl_drawmouseposy->integer;
		int font			= clu.mouse.cl_drawmousesize->integer;

		if (!prv_time) prv_time = systemtime; //initial

		if (systemtime - prv_time) { 
			ups = 1000 * clu.mouse.count/ (systemtime - prv_time);
		}

		sum += ups;
		thiscount++;

		if ((systemtime - prv_time) >  (int)(interval * 1000)) { 

			sumx = sumy = maxx = maxy = dpsx = dpsy = 0; 
		
			prv_time = systemtime;
		
			avg = sum / thiscount; 
			thiscount = sum = 0;

			sumx = clu.mouse.sumx;
			sumy = clu.mouse.sumy;
			clu.mouse.sumx =  clu.mouse.sumy = 0;

			if (clu.mouse.count) {
				dpsx = sumx / clu.mouse.count;
				dpsy = sumy / clu.mouse.count;
			}

			maxx = clu.mouse.maxx;
			maxy = clu.mouse.maxy;
			clu.mouse.maxx = clu.mouse.maxy = 0;
		
			clu.mouse.count = 0;
		}

		#ifdef WIN32
			if (clu.raw.registered)	strcpy(source, "RawMouse");
			else						strcpy(source, "SDL mouse");
		#else
									strcpy(source, "SDL mouse");
		#endif	

		sprintf( s1,	 "%3iUPS",					ups);
		sprintf( s2,	 "Interval:     %4.2fs",		interval);	
		sprintf( s3,	 "Source:       %s",			source);
		sprintf( s4, "UPS:       %4i",			avg);
		sprintf( s5, "DPS:       %4ix %4iy",	dpsx, dpsy);
		sprintf( s6, "Total Dots:%4ix %4iy",	sumx, sumy);
		sprintf( s7, "Max Update:%4ix %4iy",	maxx, maxy);
	
		if (posx >= 0 && posx < 21) 
			posxx = posx * 30;
		else 
			posxx = 0;

		if (posy >= 0 && posy < 24) 
			posyy = posy * 20;
		else
			posyy = 0;

		SCR_DrawStringExt(posxx, posyy,				font, s1, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (font+2)*1,	font, s2, g_color_table[8], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (font+2)*2,	font, s3, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (font+2)*3,	font, s4, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (font+2)*4,	font, s5, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (font+2)*5,	font, s6, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (font+2)*6,	font, s7, g_color_table[7], qtrue, qtrue);
	}

		
	/*  
	=================
	SCR_DrawPing - Draw the ping, mean ping, spike from mean, mean spike and standard deviation of ping for two user-set intervals
	FIXME on all meters: Max size of vars in relation to intervals
	=================
	*/
	void SCR_DrawPing (void) {

		char 							string1[64],string2[64],string3[64],string4[64],	
										string5[64],string6[64];
		static int						*pings, *pings2, timeRec, timeRec2, count, count2, acount, acount2, avgPing,
										avgPing2, flux, flux2, aflux, aflux2, std, std2, alloc_ok, alloc_ok2;
		static unsigned long long int	sum, sum2, asum, asum2, stsum, stsum2; // some of those may be getting large
		int								timeNew, currPing, interval, interval2, fontsize, posx, posy, posxx, posyy, i;

		// vars needing refreshing each global loop:
		timeNew		=	Sys_Milliseconds(); //not affected by timescale or timing alterations by the server.
		currPing		=	cl.snap.ping;
		// we let those able to be set on the fly for fun:
		interval		=	clu.cl_drawpingfirstinterval->integer;  //how long we keep collecting data before calcs
		interval2	=	clu.cl_drawpingsecondinterval->integer; //the same being repeated for another interval
		fontsize 	= 	clu.cl_drawpingfontsize->integer;
		posx			=	clu.cl_drawpingposx->integer;
		posy			=	clu.cl_drawpingposy->integer;
	
		// preventing two potential devisions by zero and nonsensical results:
		if (interval  <= 0)	interval	  = 1;
		if (interval2 <= 0) interval2 = 1;

		//only set initially/once:
		if (!timeRec)
			timeRec	= timeRec2 = timeNew;  //to assume initially that we are at a new interval (New and Rec's difference 0)

		if ((timeNew - timeRec)/1000 <  interval) { // we're in the interval; collect info:
		
			if (!(count % 128)) { //realloc only every 128 entries and initially

				int *tmp = realloc(pings, (count + 128)  * sizeof (int)); //giving space 
		    
				if (tmp != NULL) { 
					//alloc ok
					alloc_ok = 1;
					pings = tmp;
				} else { 
				// alloc failed; reset.
					alloc_ok = 0;
					count = 0;
				}
			}
			if (alloc_ok) { 
				pings[count] = currPing; //collecting all pings for interval-end
						          			//it's more accurate compared to doing it on the fly and using a 'previous mean' in later calculations
				count++;
			}
		} else if (count) { // we have hit the point the interval ends (time >= interval); let's do the calcs:
							//plus don't go further if there's no count (only if mem allocation totally failed above)
							//also avoids potential divisions by zero

			for (i=0; i < count; i++)
				sum += pings[i];
		
			avgPing = sum / count; // mean

			flux = 0; //max spike has to reset at this point

			for (i=0; i < count; i++) {	 //now that we have the mean, std deviation and spikes calcs
				//std dev sum:
				stsum += pow((double)(pings[i] - avgPing), 2); //(double) because more than 2 functions exist in library (+float)
				//spikes:
				if (pings[i] > avgPing) { //we had a spike (of the high side)
					acount++;
					asum += (pings[i] - avgPing);  //summing the spikes for later avg of spikes
					if (pings[i] > flux + avgPing) {	 //we had a 'champion' spike 
						flux = pings[i] - avgPing;  //for printing the current max spike for the interval; resets each loop
					}
				}
			}

			std = sqrt((double)(stsum / count));  // standard deviation

			if (acount)
				aflux = asum / acount; //average of high spikes
			else	
				aflux = 0;	//there was no flux for this interval; this is common.
							//+ avoids division by zero

			timeRec = timeNew; //record a new point in time to check for next interval end
		
			//reset static vars that are being used in interval calculations:
			sum = count = asum = acount = stsum = 0;
			free (pings);
			pings = NULL; //don't forget this. free() is not enough for realloc() and debuggers will not be kind to explain why.
		}
	
		//pure repetition of the above (for 2nd interval):
		//FIXME: should have been a function
		if ((timeNew - timeRec2)/1000 <  interval2) { 

			if (!(count2 % 128)) { 

				int *tmp = realloc(pings2, (count2 + 128)  * sizeof (int)); 
		    
				if (tmp != NULL) { 
					alloc_ok2 = 1;
					pings2 = tmp;
				} else { 
					alloc_ok2 = 0;
					count2 = 0;
				}
			}
			if (alloc_ok2) { 
				pings2[count2] = currPing; 
						          	 
				count2++;
			}

		} else if (count2) { 

			for (i=0; i < count2; i++)
				sum2 += pings2[i];

			avgPing2 = sum2 / count2; 

			flux2 = 0;

			for (i=0; i < count2; i++) {	 
			
				stsum2 += pow((double)(pings2[i] - avgPing2), 2);
			
				if (pings2[i] > avgPing2) { 
					acount2++;
					asum2 += (pings2[i] - avgPing2);  
					if (pings2[i] > flux2 + avgPing2) 
						flux2 = pings2[i] - avgPing2;  
				}
			}

			std2 = sqrt((double)(stsum2 / count2));  
		
			if (acount2)
				aflux2   = asum2 / acount2; 
			else		
				aflux2 	= 0;	

			timeRec2 = timeNew; 

		
			sum2 = count2 = asum2 = acount2 = stsum2 = 0;
			free (pings2); 
			pings2 = NULL; 
		}
		//end of pure repetition

		Com_sprintf( string1, sizeof ( string1 ), "%3ims",currPing);
		Com_sprintf( string2, sizeof ( string2 ), "Interval   %3is  %3is",interval, interval2);
		Com_sprintf( string3, sizeof ( string3 ), "Mean      %3ims %3ims",avgPing, avgPing2);
		Com_sprintf( string4, sizeof ( string4 ), "Max Spike %3ims %3ims",flux,    flux2);
		Com_sprintf( string5, sizeof ( string5 ), "Avg Spike %3ims %3ims",aflux,   aflux2);
		Com_sprintf( string6, sizeof ( string6 ), "Std Dev   %3ims %3ims",std,     std2);

		//positioning of printing (it could be one var for simplicity but this is more intuitive)
		if (posx >= 0 && posx < 21) // well, you can't go beyond 640 virtual pixels; so avoiding hiding it
			posxx = posx * 30;
		else
			posxx = 0;

		if (posy >= 0 && posy < 24) //avoiding going beyond 480 virtual pixels
			posyy = posy * 20;
		else
			posyy = 0;

		//fontsize + 2 came from experimentation, no harry potter involved
		SCR_DrawStringExt(posxx, posyy                 , fontsize, string1, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*1, fontsize, string2, g_color_table[4], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*2, fontsize, string3, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*3, fontsize, string4, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*4, fontsize, string5, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*5, fontsize, string6, g_color_table[7], qtrue, qtrue);
	}

	/*
	=================
	SCR_DrawFPS - equivalent to drawping for FPS
	MicroGranularity and float supported
	=================
	*/
	void SCR_DrawFPS (void) {
		float						fps, interval;
		char 						string1[64],string2[64],string3[64],string4[64],	string5[64],string6[64];
		static float					*fpses, avgfps, drop, dcount, stdfps, adrop, sumfps, stsumfps, dsum, alloc_fps; 
		int							fontsize, posx, posy, posxx, posyy, i;
		static int					countfps;
		static unsigned long long	microold, timeRecfps;
		unsigned long long			micronew;
		long long					div;

		/*	System time is not affected by timescale as urt's cg_drawfps. Notice on baseq3 latest's comments, 
			if system's time isn't used there may be inconsistencies  due to compensations by the server for latency  etc. 

			Now microGranularity and float supported		*/

		if (clu.sys_microGranularity->integer)	
			micronew	 = Sys_Microseconds();
		else
			micronew = Sys_Milliseconds() * 1000LL; // conversion with '(long long)' (or LL) or the calculation overflows.

		if(!microold)
			microold	 = micronew;

		div = micronew - microold; //fixme? ensure unoverflowability
		
		microold	 = micronew; //recall the previous time
		
		if (div) // division by zero avoidance; theoreticaly possible on timedemo.
			fps = 1000000.0 / div;  /*	how long did it take it to come back here? We don't buffer on purpose, 	that's why
										we have an interval in this. */
		else
			fps = 1000.0; 

		interval		=	clu.cl_drawfpsinterval->value;  
		fontsize 	= 	clu.cl_drawfpsfontsize->integer;
		posx			=	clu.cl_drawfpsposx->integer;
		posy			=	clu.cl_drawfpsposy->integer;
	
		if (interval <= 0.0) interval = 0.02;

		if (!timeRecfps)
			timeRecfps = micronew;

		if ((float)(micronew - timeRecfps)/1000000 <  interval) { 

			if (!(countfps % 128)) { 

				float *tmp = realloc(fpses, (countfps + 128)  * sizeof (float)); 
		    
				if (tmp != NULL) { 
					alloc_fps = 1;
					fpses = tmp;
				} else { 
					alloc_fps = 0;
					countfps = 0;
				}
			}
			if (alloc_fps) { 
				fpses[countfps] = fps; 		          	 
				countfps++;
			}

		} else if (countfps) { 

			for (i=0; i < countfps; i++)  
				sumfps += fpses[i];

			avgfps	= sumfps /countfps;

			drop = 0.0;

			for (i=0; i < countfps; i++) {	
			
				stsumfps += pow(fpses[i] - avgfps, 2);
				if (fpses[i] < avgfps) { 
					dcount++;
					dsum += (avgfps - fpses[i]);
					if (fpses[i] < avgfps - drop)
						drop = avgfps - fpses[i];
				}
			}

			stdfps = sqrt(stsumfps / countfps);

			if (dcount)	
				adrop = dsum / dcount;		
			else			
				adrop = 0.0;

			timeRecfps = micronew; 
		
			sumfps = countfps = stsumfps = dsum = dcount = 0.0;
			free (fpses);
			fpses = NULL; 
		} else {
			timeRecfps = micronew; /*	If the interval is small enough to not let it satisfy either of the previous two conditions,
										timeRecfps is not replenished and that makes the 'new time - old' check impossible to be ever
										satisfied again, unless a big enough interval is set in time. Hence this assignment here
										allows for extremely low intervals without explicitely disallowing them, even if they won't
										let it print much. */
			Com_Printf("cl_drawFPS: interval too low to be meaningful on this FPS. FPS is %f, consider an interval above 1/FPS = %fs\n", fps, 1.0/fps);
		}

		Com_sprintf( string1, sizeof ( string1 ), "%7.3fFPS",fps);
		Com_sprintf( string2, sizeof ( string2 ), "Interval   %.2fs",interval);
		Com_sprintf( string3, sizeof ( string3 ), "Mean     %7.3f",avgfps);
		Com_sprintf( string4, sizeof ( string4 ), "Max Drop %7.3f",drop);
		Com_sprintf( string5, sizeof ( string5 ), "Avg Drop %7.3f",adrop);
		Com_sprintf( string6, sizeof ( string6 ), "Std Dev  %7.3f",stdfps);
	
		if (posx >= 0 && posx < 21)  
			posxx = posx * 30;
		else
			posxx = 0;

		if (posy >= 0 && posy < 24)
			posyy = posy * 20;
		else
			posyy = 0;

		SCR_DrawStringExt(posxx, posyy                 , fontsize, string1, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*1, fontsize, string2, g_color_table[1], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*2, fontsize, string3, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*3, fontsize, string4, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*4, fontsize, string5, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*5, fontsize, string6, g_color_table[7], qtrue, qtrue);
	}

	/*
	=================
	SCR_DrawSnaps - equivalent to drawping for snapshots received from server - most comments in drawping
	=================
	*/
	typedef struct { //could be in clu.
		int 			sps;
		int			fps; //for internal calculation of delayed/sec etc.
		int 			state;
		qboolean		extrapolated;
	} s_snaps;

	void SCR_DrawSnaps (void) {
		char 							string1[64],string2[64],string3[64],string4[64],string5[64],string6[64],
										string7[64], string8[64];
		static int						sps, omsg, otime, timeRecsnaps, timeRecsnaps2, countsnaps, countsnaps2, avgsnaps, avgsnaps2,
										sdrop,sdrop2,asdrop,asdrop2,dcount,dcount2,stdsnaps,stdsnaps2,delayedpacks,delayedpacks2,
										alloc_snaps,alloc_snaps2,extrapolatedsnaps,extrapolatedsnaps2, fps,otimefps,	avgfps,avgfps2;
		static float						coef,coef2,delayedpacksf,delayedpacksf2,extrapolatedsnapsf,extrapolatedsnapsf2;
		int								cmsg, ctime, interval,interval2,fontsize,posx, posy, posxx, posyy, i;
		static unsigned long long int 	sumsnaps, sumsnaps2, snsum,snsum2,stsnapssum,stsnapssum2,sumfps,sumfps2;
		static s_snaps					*snapses, *snapses2;

		cmsg = cl.snap.messageNum;
		ctime = Sys_Milliseconds(); //not affected by timescale or timing alterations by the server.
	
		if (!omsg) { //initially
			omsg = cmsg;
			otime = ctime;
			otimefps = ctime;
		}

		if (ctime - otimefps) {
			fps = 1000 / (ctime - otimefps); // for internal use only
			otimefps = ctime;
		}

	
		if (ctime - otime && cmsg-omsg) {
			sps = 1000 * (cmsg - omsg) / (ctime - otime);
			omsg  = cmsg;
			otime = ctime;
		} 

		interval		=	clu.cl_drawsnapsfirstinterval->integer;  
		interval2	=	clu.cl_drawsnapssecondinterval->integer; 
		fontsize 	= 	clu.cl_drawsnapsfontsize->integer;
		posx			=	clu.cl_drawsnapsposx->integer;
		posy			=	clu.cl_drawsnapsposy->integer;
	
		if (interval  <= 0) interval  = 1;
		if (interval2 <= 0) interval2 = 1;

		if (!timeRecsnaps)
			timeRecsnaps	= timeRecsnaps2 = ctime;  
   
		if ((ctime - timeRecsnaps)/1000 <  interval) { 

			if (!(countsnaps % 128)) { 

				s_snaps *tmp = realloc(snapses, (countsnaps + 128)  * sizeof (s_snaps)); 
		    
				if (tmp != NULL) { 
					alloc_snaps = 1;
					snapses = tmp;
				} else { 
					alloc_snaps = 0;
					countsnaps = 0;
				}
			}
			if (alloc_snaps) { 
				snapses[countsnaps].sps = sps; 
				snapses[countsnaps].state = cl.snap.snapFlags;
				snapses[countsnaps].extrapolated = cl.extrapolatedSnapshot;
				snapses[countsnaps].fps = fps;
				countsnaps++;
			}

		} else if (countsnaps) { 

			for (i=0; i < countsnaps; i++) {
				sumsnaps += snapses[i].sps;
				sumfps +=snapses[i].fps;
			}

			avgsnaps = sumsnaps /countsnaps;
			avgfps = sumfps / countsnaps;
			coef	 = (float)avgsnaps / (float)avgfps / (float)interval;  //(float) required because they are integers and the result would be different

			sdrop = 	delayedpacks = extrapolatedsnaps = 0;

			for (i=0; i < countsnaps; i++) {	
			
				stsnapssum += pow((double)(snapses[i].sps - avgsnaps), 2);
				if (snapses[i].sps < avgsnaps) { 
					dcount++;
					snsum += (avgsnaps - snapses[i].sps);
					if (snapses[i].sps < avgsnaps - sdrop) 
						sdrop = avgsnaps - snapses[i].sps;
				}
			
				//state:
				if (snapses[i].state & SNAPFLAG_RATE_DELAYED) 
					delayedpacks++;

				delayedpacksf = (float)delayedpacks * coef;

				if (snapses[i].extrapolated == qtrue)
					extrapolatedsnaps++;

				extrapolatedsnapsf = (float) extrapolatedsnaps * coef; 
			}

			stdsnaps = sqrt((double)(stsnapssum / countsnaps));

			if (dcount)
				asdrop = snsum / dcount;		
			else
				asdrop = 0;

			timeRecsnaps = ctime; 
		
			sumsnaps = countsnaps = stsnapssum = snsum = dcount = sumfps = 0;
			free (snapses);
			snapses = NULL; 
		}

		//2
		if ((ctime - timeRecsnaps2)/1000 <  interval2) { 

			if (!(countsnaps2 % 128)) { 

				s_snaps *tmp = realloc(snapses2, (countsnaps2 + 128)  * sizeof (s_snaps)); 
		    
				if (tmp != NULL) { 
					alloc_snaps2 = 1;
					snapses2 = tmp;
				} else { 
					alloc_snaps2 = 0;
					countsnaps2 = 0;
				}
			}
			if (alloc_snaps2) { 
				snapses2[countsnaps2].sps = sps; 
				snapses2[countsnaps2].state = cl.snap.snapFlags;
				snapses2[countsnaps2].extrapolated = cl.extrapolatedSnapshot;
				snapses2[countsnaps2].fps = fps;
				countsnaps2++;
			}

		} else if (countsnaps2) { 

			for (i=0; i < countsnaps2; i++) {
				sumsnaps2 += snapses2[i].sps;
				sumfps2 +=snapses2[i].fps;
			}

			avgsnaps2 = sumsnaps2 /countsnaps2;
			avgfps2 = sumfps2 / countsnaps2;
			coef2	 = (float)avgsnaps2 / (float)avgfps2 / (float)interval2;  

			sdrop2 =  delayedpacks2 = extrapolatedsnaps2 = 0;

			for (i=0; i < countsnaps2; i++) {	
			
				stsnapssum2 += pow((double)(snapses2[i].sps - avgsnaps2), 2);
				if (snapses2[i].sps < avgsnaps2) { 
					dcount2++;
					snsum2 += (avgsnaps2 - snapses2[i].sps);
					if (snapses2[i].sps < avgsnaps2 - sdrop2) 
						sdrop2 = avgsnaps2 - snapses2[i].sps;
				}
			
				//state:
				if (snapses2[i].state & SNAPFLAG_RATE_DELAYED) 
					delayedpacks2++;

				delayedpacksf2 = (float)delayedpacks2 * coef2;

				if (snapses2[i].extrapolated == qtrue) 
					extrapolatedsnaps2++;
			
				extrapolatedsnapsf2 = (float) extrapolatedsnaps2 * coef2; 
			}

			stdsnaps2 = sqrt((double)(stsnapssum2 / countsnaps2));

			if (dcount2) 
				asdrop2 = snsum2 / dcount2;		
			else 
				asdrop2 = 0;

			timeRecsnaps2 = ctime; 
		
			sumsnaps2 = countsnaps2 = stsnapssum2 = snsum2 = dcount2 = sumfps2 = 0;
			free (snapses2);
			snapses2 = NULL; 
		}

		Com_sprintf( string1, sizeof ( string1 ), "%3iSPS",sps);
		Com_sprintf( string2, sizeof ( string2 ), "Interval   %3is   %3is",interval, interval2);
		Com_sprintf( string3, sizeof ( string3 ), "Mean     %3iSPS %3iSPS",avgsnaps,avgsnaps2);
		Com_sprintf( string4, sizeof ( string4 ), "Max Drop %3iSPS %3iSPS",sdrop, sdrop2);
		Com_sprintf( string5, sizeof ( string5 ), "Avg Drop %3iSPS %3iSPS",asdrop, asdrop2);
		Com_sprintf( string6, sizeof ( string6 ), "Std Dev  %3iSPS %3iSPS",stdsnaps,stdsnaps2);
		Com_sprintf( string7, sizeof ( string7 ), "Delayed  %.1fSPS %.1fSPS",delayedpacksf,delayedpacksf2);
		Com_sprintf( string8, sizeof ( string8 ), "Extrap.  %.1fSPS %.1fSPS",extrapolatedsnapsf,extrapolatedsnapsf2);
	
		if (posx >= 0 && posx < 21) 
			posxx = posx * 30;
		else
			posxx = 0;

		if (posy >= 0 && posy < 24) 
			posyy = posy * 20;
		else
			posyy = 0;

		SCR_DrawStringExt(posxx, posyy                 , fontsize, string1, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*1, fontsize, string2, g_color_table[3], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*2, fontsize, string3, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*3, fontsize, string4, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*4, fontsize, string5, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*5, fontsize, string6, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*6, fontsize, string7, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*7, fontsize, string8, g_color_table[7], qtrue, qtrue);
	}

	/*
	=================
	SCR_DrawPackets - equivalent to drawping for packets from client to server
	=================
	*/
	void SCR_DrawPackets (void) {
		int			posx, posy, posxx, posyy, fontsize, sent, newtime, interval,interval2, i;
		char			string1[64],string2[64],string3[64],string4[64],string5[64],string6[64];	
		static int	*packs, *packs2, pps, count, count2, timerec	,timerec2, lastrecorded, avgpps,avgpps2,
					mdrop, mdrop2, adrop, adrop2, dcount,dcount2, std, std2, alloc_ok, alloc_ok2;
		static unsigned long long int sum,sum2,stdsum,stdsum2,dsum,dsum2;

		interval		=	clu.cl_drawpacketsfirstinterval->integer;  
		interval2	=	clu.cl_drawpacketssecondinterval->integer; 
		fontsize    = 	clu.cl_drawpacketsfontsize->integer;
		posx			=	clu.cl_drawpacketsposx->integer;
		posy			=	clu.cl_drawpacketsposy->integer;
	

		newtime = Sys_Milliseconds(); //not affected by timescale or timing alterations by the server.
		sent = clu.sent;
		!timerec ? timerec	= timerec2 = lastrecorded = newtime : 0;  //initially
		interval <= 0 ? 	interval = 1 : 0;
		interval2 <= 0 ? interval2 = 1 :0;

		if (sent && (newtime - lastrecorded)) {
				pps = 1000 / (newtime - lastrecorded);
				lastrecorded = newtime;
		}


		if ((newtime - timerec)/1000 <  interval) { 

			if (!(count % 128)) { 

				int *tmp = realloc(packs, (count + 128)  * sizeof (int)); 
		    
				if (tmp != NULL) { 
					alloc_ok = 1;
					packs = tmp;
				} else { 
					alloc_ok = 0;
					count = 0;
				}
			}
			if (alloc_ok) { 
				packs[count] = pps; 	          	 
				count++;
			}

		} else if (count) { 

			for (i=0; i < count; i++)
				sum += packs[i];

			avgpps = sum /count;

			mdrop = 0;
		

			for (i=0; i < count; i++) {	
			
				stdsum += pow((double)(packs[i] - avgpps), 2);
				if (packs[i] < avgpps) { 
					dcount++;
					dsum += (avgpps - packs[i]);
					if (packs[i] < avgpps - mdrop) 
						mdrop = avgpps - packs[i];
				}

			}

			std = sqrt((double)(stdsum / count));

			if (dcount)
				adrop = dsum / dcount;		
			else 
				adrop = 0;

			timerec = newtime; 
		
			sum = count = stdsum = dsum = dcount = 0;
			free (packs);
			packs = NULL; 
		}

		//2
		if ((newtime - timerec2)/1000 <  interval2) { 

			if (!(count2 % 128)) { 

				int *tmp = realloc(packs2, (count2 + 128)  * sizeof (int)); 
		    
				if (tmp != NULL) { 
					alloc_ok2 = 1;
					packs2 = tmp;
				} else { 
					alloc_ok2 = 0;
					count2 = 0;
				}
			}
			if (alloc_ok2) { 
				packs2[count2] = pps; 	          	 
				count2++;
			}

		} else if (count2) { 

			for (i=0; i < count2; i++)
				sum2 += packs2[i];

			avgpps2 = sum2 /count2;

			mdrop2 = 0;
		
			for (i=0; i < count2; i++) {	
			
				stdsum2 += pow((double)(packs2[i] - avgpps2), 2);
				if (packs2[i] < avgpps2) { 
					dcount2++;
					dsum2 += (avgpps2 - packs2[i]);
					if (packs2[i] < avgpps2 - mdrop2) 
						mdrop2 = avgpps2 - packs2[i];
				}

			}

			std2 = sqrt((double)(stdsum2 / count2));

			if (dcount2) 
				adrop2 = dsum2 / dcount2;		
			else 
				adrop2 = 0;

			timerec2 = newtime; 
		
			sum2 = count2 = stdsum2 = dsum2 = dcount2 = 0;
			free (packs2);
			packs2 = NULL; 
		}

		Com_sprintf( string1, sizeof ( string1 ), "%3iPPS",pps);
		Com_sprintf( string2, sizeof ( string2 ), "Interval   %3is   %3is",interval, interval2);
		Com_sprintf( string3, sizeof ( string3 ), "Mean     %3iPPS %3iPPS",avgpps,avgpps2);
		Com_sprintf( string4, sizeof ( string4 ), "Max Drop %3iPPS %3iPPS",mdrop, mdrop2);
		Com_sprintf( string5, sizeof ( string5 ), "Avg Drop %3iPPS %3iPPS",adrop, adrop2);
		Com_sprintf( string6, sizeof ( string6 ), "Std Dev  %3iPPS %3iPPS",std,std2);
	
		if (posx >= 0 && posx < 21) 
			posxx = posx * 30;
		else 
			posxx = 0;

		if (posy >= 0 && posy < 24) 
			posyy = posy * 20;
		else
			posyy = 0;

		SCR_DrawStringExt(posxx, posyy                 , fontsize, string1, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*1, fontsize, string2, g_color_table[2], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*2, fontsize, string3, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*3, fontsize, string4, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*4, fontsize, string5, g_color_table[7], qtrue, qtrue);
		SCR_DrawStringExt(posxx, posyy + (fontsize+2)*5, fontsize, string6, g_color_table[7], qtrue, qtrue);
	}

#endif
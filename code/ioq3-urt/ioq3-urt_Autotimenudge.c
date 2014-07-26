// under GPL v2

#if !defined(DEDICATED) 

	/*
	autotimenudge - set timenudge based on standard deviation of ping; it accounts for flood protection.
	*highly experimental* because: 
	major   FIXME: on 'live' mode (off by default), gameplay aspects such as TS round change or spectating, crap out nonsensical ping values
	normal  FIXME: timenudge gets saved to disk each time, this should be avoided during gameplay
	some code from SCR_DrawPing(), comments in it
	previous choice and std dev are global between the 2 functions
	For some reason, std dev sometimes is reported zero (at least on on-demand version).		
	*/

	#include "ioq3-urt.h"
	#include "../client/client.h"

	void CL_Autotimenudge (void) {
		static int						timeRec, count, avgPing, alloc_ok, flood_prot_timer, firsttime;
		static unsigned long long int	sum, stsum; // some of those may be getting large
		int								timeNew, currPing, interval, i, flood_prot;
		char								command[61], buf[64], text[128];
		static int						*pings;

		if (clu.cl_autotimenudge->integer) {
			timeNew		=	Sys_Milliseconds(); //not affected by timescale or timing alterations by the server.
			currPing		=	cl.snap.ping;
			interval		=	clu.cl_autotimenudgeinterval->integer;  //how long we keep collecting data before calcs
			flood_prot  =   clu.cl_autotimenudgefloodprotsec->integer; //game's flood protection limit in seconds

			if (interval <= 0) interval = 1;

			if (!timeRec) timeRec = flood_prot_timer = timeNew;

			if (!clu.atchoice) clu.atchoice = 0;

			if ((timeNew - timeRec)/1000 <  interval) { 
			
				if (!(count % 128)) { 

					int *tmp = realloc(pings, (count + 128)  * sizeof (int)); 
			    
					if (tmp != NULL) { 
						//alloc ok
						alloc_ok = 1;
						pings = tmp;
					} else { 
						alloc_ok = 0;
						count = 0;
					}
				}
				if (alloc_ok) { 
					pings[count] = currPing; 
					count++;
				}
			} else if (count) { 
			
				for (i = 0; i < count; i++)
					sum += pings[i];
			
				avgPing = sum / count; 

				for (i=0; i < count; i++)  
					stsum += pow((double)(pings[i] - avgPing), 2);
				clu.atstd = sqrt((double)(stsum / count));  

				timeRec = timeNew; 
			
				sum = count = stsum = 0;
				free (pings);
				pings = NULL; 

				if (clu.cl_autotimenudgelive->integer) { //very quirky especially on round-based games
				
					//better than an unprotected malloc or shorter than a malloc with checks:
					if (strlen(clu.cl_autotimenudgecommand->string) <= 61) 
						strcpy(command, clu.cl_autotimenudgecommand->string); 
					else
						strcpy(command, "ut_timenudge"); 

					strcpy(text,"Live autotimenudge: %s set \nbased on standard deviation of ping: %ims\n");
				
					//avoid flood protects on the beginning of games *workaround
					//wait for 10sec - fixme? this may need increase or expansion in process
					//though in general it's still a workaround, such issues should be handled by game
					if ((timeNew - flood_prot_timer)/1000 > 10)
						firsttime = 1;
					if (!firsttime) 
						return;
				
					if ((timeNew - flood_prot_timer)/1000 > flood_prot) {
						sprintf(buf,"%s %i", command, clu.atstd/*/2 maybe best to be the same*/);
						Cbuf_ExecuteText(EXEC_NOW, buf );
						Com_Printf(text, buf, clu.atstd);
					/*	//fixme: choice doesn't account for manual change, though less important on live
						if (                   clu.atstd <= 4 && clu.atchoice != 1) { 
							sprintf(buf,"%s 0", command);
							Cbuf_ExecuteText(EXEC_NOW, buf );
							clu.atchoice = 1;
							Com_Printf(text, buf, clu.atstd);
						}
						if (clu.atstd >= 5  && clu.atstd <=14 && clu.atchoice != 2) {
							sprintf(buf,"%s 10", command);
							Cbuf_ExecuteText(EXEC_NOW, buf);
							clu.atchoice = 2;
							Com_Printf(text, buf, clu.atstd);
						}
						if (clu.atstd >= 15 && clu.atstd <=24 && clu.atchoice != 3) {
							sprintf(buf,"%s 20", command);
							Cbuf_ExecuteText(EXEC_NOW, buf);
							clu.atchoice = 3;
							Com_Printf(text, buf, clu.atstd);
						}
						if (clu.atstd >= 25 && clu.atstd <=34 && clu.atchoice != 4) {
							sprintf(buf,"%s 30", command);
							Cbuf_ExecuteText(EXEC_NOW, buf);
							clu.atchoice = 4;
							Com_Printf(text, buf, clu.atstd);
						}
						if (clu.atstd >= 35 && clu.atstd <=44 && clu.atchoice != 5) { 
							sprintf(buf,"%s 40", command);
							Cbuf_ExecuteText(EXEC_NOW, buf);
							clu.atchoice = 5;
							Com_Printf(text, buf, clu.atstd);
						}
						if (clu.atstd >= 45             && clu.atchoice != 6) {
							sprintf(buf,"%s 50", command);
							Cbuf_ExecuteText(EXEC_NOW, buf);
							clu.atchoice = 6;
							Com_Printf(text, buf, clu.atstd);
						}*/
						flood_prot_timer = timeNew;
					}
				}
			}
		}
	}

	/*
	on demand auto-timenudge
	less experimental
	eases the 'non-sensical info on round or spectating' issue, though still possible
	avoids flood protection issues largely
	retains the 'access to disk' issue though relatively normal here, an explicit cvar setting would do too
	requires the main function and cl_autotimenudge to collect data for not duplicating code unnecessarily
	*/
	void CL_Autotimenudge_f(void) {
		char			command[61], buf[64]; 
		char			text[128] = "On-demand autotimenudge: %s set \nbased on standard deviation of ping: %ims\n";
	//	char			text2[128]= "On-demand autotimenudge: %s already set \nbased on half of standard deviation of ping: %ims\n";

		if (!clu.cl_autotimenudge->integer) {
			Com_Printf("On-demand autotimenudge requires cl_autotimenudge to collect data. Exiting.\n");
			return;
		}
	
		if (strlen(clu.cl_autotimenudgecommand->string) <= 61)
			strcpy(command, clu.cl_autotimenudgecommand->string); 
		else
			strcpy(command, "ut_timenudge"); 

		//fixme: clu.choice doesn't account for manual change 
		//[no access to ut_timenudge itself to account for it, though it could be workarounded]
		//for now bypass it, left on live but less important: 
			
		//clu.atchoice = 0; 


		sprintf(buf,"%s %i", command, clu.atstd/*/2 maybe best to be the same*/);
		Cbuf_ExecuteText(EXEC_NOW, buf );
		Com_Printf(text, buf, clu.atstd);


		/*if (                    clu.atstd <= 4) {
			if (clu.atchoice != 1) { 
				sprintf(buf,"%s 0", command);
				Cbuf_ExecuteText(EXEC_NOW, buf );
				clu.atchoice = 1;
				Com_Printf(text, buf, clu.atstd);
			} else {
				sprintf(buf,"%s 0", command); 
				Com_Printf(text2, buf, clu.atstd);
			}
		}
		if (clu.atstd >= 5  && clu.atstd <=14) {
			if (clu.atchoice != 2) {
				sprintf(buf,"%s 10", command);
				Cbuf_ExecuteText(EXEC_NOW, buf);
				clu.atchoice = 2;
				Com_Printf(text, buf, clu.atstd);
			} else {
				sprintf(buf,"%s 10", command);
				Com_Printf(text2, buf, clu.atstd);
			} 
		}
		if (clu.atstd >= 15 && clu.atstd <=24) {
			if (clu.atchoice != 3) {
				sprintf(buf,"%s 20", command);
				Cbuf_ExecuteText(EXEC_NOW, buf);
				clu.atchoice = 3;
				Com_Printf(text, buf, clu.atstd);
			} else {
				sprintf(buf,"%s 20", command);
				Com_Printf(text2, buf, clu.atstd);
			}
		}
		if (clu.atstd >= 25 && clu.atstd <=34) {
			if (clu.atchoice != 4) {
				sprintf(buf,"%s 30", command);
				Cbuf_ExecuteText(EXEC_NOW, buf);
				clu.atchoice = 4;
				Com_Printf(text, buf, clu.atstd);
			} else {
				sprintf(buf,"%s 30", command);
				Com_Printf(text2, buf, clu.atstd);
			}
		}
		if (clu.atstd >= 35 && clu.atstd <=44) {
			if (clu.atchoice != 5) { 
				sprintf(buf,"%s 40", command);
				Cbuf_ExecuteText(EXEC_NOW, buf);
				clu.atchoice = 5;
				Com_Printf(text, buf, clu.atstd);
			} else {
				sprintf(buf,"%s 40", command);
				Com_Printf(text2, buf, clu.atstd);
			}
		}
		if (clu.atstd >= 45) {
			if (clu.atchoice != 6) {
				sprintf(buf,"%s 50", command);
				Cbuf_ExecuteText(EXEC_NOW, buf);
				clu.atchoice = 6;
				Com_Printf(text, buf, clu.atstd);
			} else {
				sprintf(buf,"%s 50", command);
				Com_Printf(text2, buf, clu.atstd);
			}
		}*/
	}

#endif

// under GPL v2

#if !defined(DEDICATED) 

	/*
	Workaround the 'server is full->server is for low pings' behavior plus false positives of 'server is for low pings'.
	We don't actually 'hijack the proper way' since assessing a connection with a single ping wasn't proper anyway. 
	e.g. The first one is often unrealistically high.
	A further workround in CL_Connect_f avoids console closing down each explicit reconnect; done_reconnect counter
	(used to not flood legitimate high ping denial) is global (static in nature) to allow for resetting it on same session. 
	NOTE: perhaps a type of waiting (before reconnect_f and no counter).
	Added: clu.done_reconnect is reset when explicitely /reconnect'ing with [the addition of] CL_Reconnect_f_clu
	*/

	#include "ioq3-urt.h"
	#include "../client/client.h"

	void reconnect_workaround (char *txt) {

		if (cls.state < CA_CONNECTED) { //protection against this hack being hijacked during gameplay; require disconnection
		
			//attempt to reconnect on 'server is for low pings' three times (to not flood legitimate denial), then give up.
			if (!strncmp(txt, "Server is for low pings only", strlen("Server is for low pings only"))) {
				if (clu.done_reconnect < 3)	{ 
					CL_Reconnect_f(); //plus, a workaround on CL_Connect_f called by this to avoid console being closed
					clu.done_reconnect++; //counter resets in CL_FirstSnapshot() to allow for further attempts on same session
				}
			}
			//reset the counter on 'server is full'
			//a nice side-effect of this is that one doesn't have to wait on the client to hit /reconnect on 'server is full'.
			if (!strncmp(txt, "Server is full.", strlen("Server is full."))) clu.done_reconnect=0;

			//"bonus", I get this on a certain ISP when on wireless, a simple /reconnect always works
			if (!strncmp(txt, "No or bad challenge for address.", strlen("No or bad challenge for address.")) 
				|| !strncmp(txt, "No or bad challenge for your address.", strlen("No or bad challenge for your address."))) {
				if (!clu.done_reconnect < 2) { //let's not flood it in case it's legitimate, try twice only.
					CL_Reconnect_f();
					clu.done_reconnect++; 
				}
			}

		}

	}
	//reset the counter of autoreconnect workaround when explicitely reconnecting 
	//it replaces /reconnect
	void  CL_Reconnect_f_clu (void) {
		clu.done_reconnect = 0;
		CL_Reconnect_f();
	}
	

#endif

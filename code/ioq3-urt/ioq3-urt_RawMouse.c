// under GPL v2

#if !defined(DEDICATED) && defined(WIN32)

	#include "ioq3-urt.h"
	#include "SDL_syswm.h"	// WindowProc function
	#include "../client/keycodes.h"

	// High-Def Mouse Motion support according to http://msdn.microsoft.com/en-us/library/ee418864%28VS.85%29.aspx:

	// Register the raw input device:
	void IN_RawInit (void) {
		PRAWINPUTDEVICELIST devices;
		UINT num;
		#define RAW_NAME_SIZE 512
		UINT size = RAW_NAME_SIZE;
		char name[RAW_NAME_SIZE];
		short int i,mice;

		RAWINPUTDEVICE			Rid[1];	//One Device (Mouse)
		Rid[0].usUsagePage	=	0x01;	//Generic Desktop 
		Rid[0].usUsage		=	0x02;	//Generic Desktop Mouse
		Rid[0].dwFlags		=	0;		//we could have used RIDEV_NOLEGACY but possible SDL issue creates cursor quirks.
		Rid[0].hwndTarget	=	NULL;	//Follow the keyboard focus

		//detect the existence of raw devices
		GetRawInputDeviceList(NULL, &num, sizeof(RAWINPUTDEVICELIST));
		if(!(devices = malloc( sizeof(RAWINPUTDEVICELIST) * num))) {
			//this won't spam the console because it sets rawmouse to 0 and hence stops the attempts via that:
			Com_Printf("..Raw Mouse malloc fail. Reverting to non-raw input.\n");
			clu.raw.in_rawmouse->integer = 0; //fixme? *maybe* unsafer than cvar_set but we don't want to alter the config.
			return; 
		}
		GetRawInputDeviceList(devices, &num, sizeof(RAWINPUTDEVICELIST));
			
		for (i=mice=0;i<num;i++){
			if(devices[i].dwType == RIM_TYPEMOUSE) {
				GetRawInputDeviceInfo(devices[i].hDevice, RIDI_DEVICENAME, name, (PUINT)&size);
				if (!strncmp(name,"\\\\?\\Root#RDP_MOU#",17)) continue; /*	ignore remote desktop mouse; -3 escapes
																			fixme? there may be more similarly non-applicable devices */
				mice++;
			}
		}
		free(devices);

		if (!mice) {
			Com_Printf("..No Raw Mouse Devices are detected. Reverting to non-raw mouse input\n");
			clu.raw.in_rawmouse->integer = 0; 
			return;
		}

		// Attempt to register the device:
		if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]))) {  
			Com_Printf("* Raw Mouse (High-Definition) Ready\n");
			clu.raw.registered = qtrue;
		} else {	
			Com_Printf("Raw Mouse registration FAILED even if %u devices had been found. Reverting to non-raw mouse input\n", mice);
			clu.raw.in_rawmouse->integer = 0; 
		}

	}
	
	//Collect the WM_INPUT data in the WindowProc loop that follows.
	void IN_Collect_WM_INPUT (HRAWINPUT lParam) {
			
		static UINT olddwSize;
		static LPVOID lpb;
		UINT dwSize; //size of lbp; autodetermined; we could be adventurous with hardcoded values but this is also easily multiplatform
		RAWINPUT* raw;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof (RAWINPUTHEADER)); //only saves size of dwSize due to NULL
		if (olddwSize != dwSize) { //hopefully a faster process than malloc()
			free(lpb); //leakage otherwise.
			if(!(lpb = malloc(dwSize))) return; //pointer to data coming from lParam/RAWINPUT
			olddwSize = dwSize;
		}

		//get actual data on lpb:
		GetRawInputData (
			(HRAWINPUT)lParam, //provided by WM_INPUT: RAWINPUT struct.
			RID_INPUT, //Get the raw data (as opposed to getting header information)
			lpb, //commented above
			&dwSize, //commented above
			sizeof(RAWINPUTHEADER) //func is defined to need this exactly.
		);
			
		raw = (RAWINPUT*)lpb; //assignment needed because ..
    
		//commented out a currentlly redundant check:
		//if (raw->header.dwType == RIM_TYPEMOUSE) { //as opposed to RIM_TYPEKEYBOARD or RIM_TYPEHID 
			
			if (	!raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {  //assume relative positioning, the usual. MOUSE_MOVE_RELATIVE is not reported for some reason.
				if ((raw->data.mouse.lLastX || raw->data.mouse.lLastY) && mouseActive) { //mouseActive or it ninjas moves after returning in game	
					clu.raw.x += raw->data.mouse.lLastX; //they could be longs but engine's queue gets ints.
					clu.raw.y += raw->data.mouse.lLastY;
					if (clu.mouse.cl_drawmouse->integer) { 
						clu.mouse.count++;
						clu.mouse.sumx += abs(raw->data.mouse.lLastX);
						clu.mouse.sumy += abs(raw->data.mouse.lLastY);
						if(clu.mouse.maxx < abs(raw->data.mouse.lLastX)) clu.mouse.maxx = abs(raw->data.mouse.lLastX);
						if(clu.mouse.maxy < abs(raw->data.mouse.lLastY)) clu.mouse.maxy = abs(raw->data.mouse.lLastY);
					}
				}
			} else if (raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) { //absolute positioning; touch screens? let's hope it works even if we don't have a device to check it.
				if ((raw->data.mouse.lLastX || raw->data.mouse.lLastY) && mouseActive) {
					clu.raw.x = raw->data.mouse.lLastX; 
					clu.raw.y = raw->data.mouse.lLastY;	
				}
			}

			if (raw->data.mouse.usButtonFlags) {
				if (clu.raw.butsnum == 64) clu.raw.butsnum = 0; // simple flood protection while loading
				clu.raw.buts	[clu.raw.butsnum]		=	raw->data.mouse.usButtonFlags;	//buttons
				clu.raw.wheeldat[clu.raw.butsnum]	=	raw->data.mouse.usButtonData;	//wheel data
				clu.raw.butsnum++;
			}

		//}
	}

	/*	Collect WindowProc msgs here (instead of requiring SDL); initially based on http://forums.indiegamer.com/showthread.php?t=2138
		This is still not restricted by FPS.
		The important benefit is that this way stock SDL is useable for raw input, let alone it makes code management easier. */
	WNDPROC lpPrevWndFunc;
	HWND hWnd;
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 	{

		if (uMsg == WM_INPUT) IN_Collect_WM_INPUT((HRAWINPUT)lParam);

		/*	call explicitely the previous WindowProc address;
			return value has no material effect here */
		return CallWindowProc(lpPrevWndFunc, hWnd, uMsg, wParam, lParam); 
	}

	//"[..] call [it] after SDL_Init() but before SDL_SetVideoMode()."
	void init_WndProc(void) { //called right before SDL_SetVideoMode();

		SDL_SysWMinfo wminfo;
		SDL_VERSION(&wminfo.version)
		if (SDL_GetWMInfo(&wminfo) != 1) {/* error: wrong SDL version*/	}
			
		hWnd = wminfo.window; //we got the SDL window handle
			
		/*  Set a new address for WindowProc()/window procedure (with GWLP_WNDPROC) on SDL window's hWnd handle.
			The previous value for GWLP_WNDPROC is returned, i.e. the previous address to WindowProc.
			Usable on both 64 and 32bit, unlike SetWindowLong() */
		lpPrevWndFunc =  (WNDPROC) SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR )WindowProc);
	}

	/*Actually handle the data
	This is called in sdl_input.c's IN_ProcessEvents().
	We *could* have it directly on WM_INPUT but it creates a hell of handling the flooding of queue of events.*/
	void IN_RawProcess (void) {

		if (	(clu.raw.x || clu.raw.y) && mouseActive) { //mouseActive needed or it'll move in ninja ways.
			Com_QueueEvent( 0, SE_MOUSE, clu.raw.x, clu.raw.y, 0, NULL );
			clu.raw.x = clu.raw.y = 0;
		}

		if (clu.raw.butsnum ) { 
			short int i;
			for (i=0;i<clu.raw.butsnum;i++){ //a simple switch() is not enough, bitwise AND is used because some keys may occur _simultaneously_.
				
				if (clu.raw.buts[i] & RI_MOUSE_BUTTON_1_DOWN)	Com_QueueEvent( 0, SE_KEY, K_MOUSE1, qtrue, 0, NULL );
				if (clu.raw.buts[i] & RI_MOUSE_BUTTON_2_DOWN)	Com_QueueEvent( 0, SE_KEY, K_MOUSE2, qtrue, 0, NULL );
				if (clu.raw.buts[i] & RI_MOUSE_BUTTON_3_DOWN)	Com_QueueEvent( 0, SE_KEY, K_MOUSE3, qtrue, 0, NULL );
				if (clu.raw.buts[i] & RI_MOUSE_BUTTON_4_DOWN)	Com_QueueEvent( 0, SE_KEY, K_MOUSE4, qtrue, 0, NULL );
				if (clu.raw.buts[i] & RI_MOUSE_BUTTON_5_DOWN)	Com_QueueEvent( 0, SE_KEY, K_MOUSE5, qtrue, 0, NULL );
				if (clu.raw.buts[i] & RI_MOUSE_BUTTON_1_UP)		Com_QueueEvent( 0, SE_KEY, K_MOUSE1, qfalse, 0, NULL );
				if (clu.raw.buts[i] & RI_MOUSE_BUTTON_2_UP)		Com_QueueEvent( 0, SE_KEY, K_MOUSE2, qfalse, 0, NULL );
				if (clu.raw.buts[i] & RI_MOUSE_BUTTON_3_UP)		Com_QueueEvent( 0, SE_KEY, K_MOUSE3, qfalse, 0, NULL );
				if (clu.raw.buts[i] & RI_MOUSE_BUTTON_4_UP)		Com_QueueEvent( 0, SE_KEY, K_MOUSE4, qfalse, 0, NULL );
				if (clu.raw.buts[i] & RI_MOUSE_BUTTON_5_UP)		Com_QueueEvent( 0, SE_KEY, K_MOUSE5, qfalse, 0, NULL );

				if (clu.raw.buts[i] & RI_MOUSE_WHEEL) {			 
					if (			clu.raw.wheeldat[i] > 0) 	{
						Com_QueueEvent( 0, SE_KEY, K_MWHEELUP, qtrue, 0, NULL );
						Com_QueueEvent( 0, SE_KEY, K_MWHEELUP, qfalse, 0, NULL );
					} else if (	clu.raw.wheeldat[i] < 0) 	{
						Com_QueueEvent( 0, SE_KEY, K_MWHEELDOWN, qtrue, 0, NULL );
						Com_QueueEvent( 0, SE_KEY, K_MWHEELDOWN, qfalse, 0, NULL );
					}
				}
			}
			clu.raw.butsnum = 0;
		}
	}

	void IN_RawDeregister (void) {
				
		RAWINPUTDEVICE			Rid[1];	
		Rid[0].usUsagePage	=	0x01;	
		Rid[0].usUsage		=	0x02;	
		Rid[0].dwFlags		=	RIDEV_REMOVE;	//Deregistration
		Rid[0].hwndTarget	=	NULL;			//required to NULL for deregistration 

		if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])))
			Com_Printf("..Raw Mouse (High-Definition) Deregistered\n");
		else		
			Com_Printf("..Raw Mouse (High-Definition) Deregistration FAILED. Assuming that it's not registered.\n");
			
		clu.raw.registered = qfalse; // applicable even if above fails (who knows why) since functionality isn't disturbed.
	}
		

#endif

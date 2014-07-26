// under GPL v2

#if !defined(DEDICATED) 
	
	#include "ioq3-urt.h"

	/* 
		Warn harshly initially if com_Sleep is used
	*/
	void Com_SleepCheck (void) {
		static qboolean warned;
		if (clu.com_Sleep->integer && !warned) {
			Com_Printf(S_COLOR_YELLOW
				"WARNING: com_Sleep is enabled; this reduces CPU contribution at all times.\n"
				"To disable it set it to 0. Use com_SleepWhenUnfocused/Minimized\n"
				"if you only require the feature when minimized or unfocused.\n" );
			warned = qtrue;
		}
	}


#endif

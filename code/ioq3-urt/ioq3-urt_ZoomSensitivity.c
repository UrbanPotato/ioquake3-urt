// under GPL v2

#if !defined(DEDICATED) 

	#include "ioq3-urt.h"

	//zoom sensitivity for R_SetupProjection()
	void R_Zoom_Sens(viewParms_t *dest) {
		static float pfovX;
		if (pfovX != dest->fovX) { // did it change fov?
			pfovX = dest->fovX; // save current zooming state
			if (pfovX < clu.cl_zoomsensitivityfovthreshold->integer) clu.do_zoomsens = qtrue; //we are zoomed, sensitivity x multiplier. 
			else clu.do_zoomsens = qfalse; //fov is too high to be zooming, sensitivity x 1. This is also the initial assumption.
		}
	}
	//for CL_MouseMove() for /zoomsensitivy
	float CL_Zoom_Sens (void) {
		if (clu.do_zoomsens)		return clu.cl_zoomsensitivity->value;
		else						return 1.0;
	}

#endif

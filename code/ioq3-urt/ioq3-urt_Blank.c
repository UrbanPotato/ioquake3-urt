// under GPL v2

#if !defined(DEDICATED) 

	#include "ioq3-urt.h"
	
	//For blanking the screen temporarily
	void R_Blank (void) {
		Cmd_TokenizeString(clu.r_blankrgb->string);
		glClearColor(atof(Cmd_Argv(0))/255,atof(Cmd_Argv(1))/255,atof(Cmd_Argv(2))/255,atof(Cmd_Argv(3)));
		glClear(GL_COLOR_BUFFER_BIT);
		//done by game: //SDL_GL_SwapBuffers();
	}

	void R_Blank_f( void ) {
		static qboolean	blanktest;
		cvar_t *draw2dtest;
		switch( Cmd_Argv( 0 )[0] ) {
				case '+':
					if (clu.r_blank->integer) blanktest = qtrue;
					clu.r_blank->integer = 1; //fixme? maybe unsafe
					draw2dtest = Cvar_Get ("cg_draw2d", "0", CVAR_ARCHIVE); //ugly workaround (no access to mod)
					if (draw2dtest->integer) Cbuf_ExecuteText(EXEC_NOW, "cg_draw2d 0"); 
					else draw2dtest->integer = 2;
					break;
				case '-':
					if (!blanktest) clu.r_blank->integer = qfalse;
					draw2dtest = Cvar_Get ("cg_draw2d", "0", CVAR_ARCHIVE); 
					if (draw2dtest->integer != 2) Cbuf_ExecuteText(EXEC_NOW, "cg_draw2d 1"); 
					blanktest = qfalse;
					break;
		}
	}

#endif

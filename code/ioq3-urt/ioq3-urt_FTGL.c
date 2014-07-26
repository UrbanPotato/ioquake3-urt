// under GPL v2

#if defined(BUILD_FREETYPE) && !defined(DEDICATED) 

	/* 
		FTGL Support for OGL rendering of Unicode text with TTF fonts 
	*/

	#include "ioq3-urt.h"
	#ifdef _MSC_VER
		#undef __cplusplus // IntelliSense FTGL issue avoidance
	#endif
	#include <FTGL/ftgl.h>
	#include "ioq3-urt_FTGL.h"
	#include "../sys/sys_loadlib.h"

	/* 
		'DLOPEN', based on cURL's 

		FTGL won't be found in most UNIXes so it helps greatly for binary distributions
		(not so much for Windows since it usually has nothing anyway and uses same dir DLLs).

		Freetype doesn't have the same behavior but it's usually available in all UNIXes
		with a graphical environment.
	*/

	#ifndef FTGL_NODLOPEN // The convenient way is the default way
				
		static void *FTGLLibrary = NULL;

		FTGLfont* (*qftglCreateTextureFont)(const char *file);
		int (*qftglSetFontFaceSize)(FTGLfont* font, unsigned int size, unsigned int res);
		void (*qftglRenderFont)(FTGLfont* font, const char *string, int mode);
		void (*qftglDestroyFont)(FTGLfont* font);
				
		// Based on cURL's GPA():
		static void *FTGL_LoadFunction(char *str) {
			void *rv;

			rv = Sys_LoadFunction(FTGLLibrary, str);
			if(!rv)
			{
				Com_Printf("Can't load symbol %s\n", str);
				clu.FTGLEnabled = qfalse;
				return NULL;
			}
			else
			{
				Com_DPrintf("Loaded symbol %s (0x%p)\n", str, rv);
				return rv;
			}
		}

	#else
		#define 	qftglCreateTextureFont ftglCreateTextureFont
		#define 	qftglSetFontFaceSize ftglSetFontFaceSize
		#define 	qftglRenderFont ftglRenderFont
		#define qftglDestroyFont ftglDestroyFont
	#endif

			
	qboolean FTGL_Init (void) {
				
		#ifndef FTGL_NODLOPEN
					
			if( (FTGLLibrary = Sys_LoadLibrary(clu.Unicode_Library->string)) == 0 ) {

				Com_Printf ("/Unicode_Library lib '%s' not loaded; trying default '%s'.\n", clu.Unicode_Library->string, DEFAULT_FTGL_LIB);
				if( (FTGLLibrary = Sys_LoadLibrary(DEFAULT_FTGL_LIB)) == 0 ) {
					Com_Printf ("default lib not loaded; attempting alternative '%s'.\n", ALTERNATIVE_FTGL_LIB);
					if( (FTGLLibrary = Sys_LoadLibrary(ALTERNATIVE_FTGL_LIB)) == 0 ) {
						Com_Printf(S_COLOR_YELLOW "No appropriate FTGL library found; needed for Unicode rendering support.\n");
						return qfalse;
					}
				}
				Com_Printf ("FTGL library loaded.\n");
			}

			clu.FTGLEnabled = qtrue;

			qftglCreateTextureFont = FTGL_LoadFunction("ftglCreateTextureFont");
			qftglSetFontFaceSize = FTGL_LoadFunction("ftglSetFontFaceSize");
			qftglRenderFont = FTGL_LoadFunction("ftglRenderFont");
			qftglDestroyFont =  FTGL_LoadFunction("ftglDestroyFont");

			if(!clu.FTGLEnabled)
			{
				Com_Printf("One or more symbols not found\n");
				return qfalse;
			}
			if (!com_quiet->integer)
				Com_Printf("OK\n");
	
			return qtrue;

		#else
			// It reached here hence it's ok
			clu.FTGLEnabled = qtrue;
			return qtrue;
		#endif
	}

#endif
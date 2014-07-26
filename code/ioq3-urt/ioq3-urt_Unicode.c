// under GPL v2

#if defined(BUILD_FREETYPE) && !defined(DEDICATED) 

	/* 
		FTGL Support for OGL rendering of Unicode text with TTF fonts 
	*/

	#include "../client/client.h"	
	#include "ioq3-urt.h"
	#ifdef _MSC_VER
		#undef __cplusplus // IntelliSense FTGL issue avoidance
	#endif
	#include "ioq3-urt_FTGL.h"
	//for uint32_t etc. (overkill compliance ftw):
	#if defined(_MSC_VER) && (_MSC_VER <= 1500) //MSVC <= 2008 doesn't have stdint.h
		#include "msvc08_stdint.h"	
	#else
		#include <stdint.h> 
	#endif
	#include "SDL_mutex.h"

	FTGLfont *FTGL_font;

	#define MAX_UNICODE_LINES 32
	struct Unicode_Text_t {
		char Message[512];
		int FadeTimeout;
		int FadeInitTime;
	} Unicode_Text[MAX_UNICODE_LINES];
	uint8_t Unicode_used_slots; // Not important for record keeping but it saves cycles.

	/*	Find a text slot and populate it. We could be using a fancy FIFO, but in practice it may be hard to manage
		differing attributes of data such as fadeout timeout. 
	*/
	#define MAX_LINE_SIZE 1024 /*	UTF-8 can be up to 4 bytes each char and console can
									have up to 256 (functions' input limits it to 256) */
	#define MAX_SPLIT_PARTS 32 // to limit automatic variables

	void Unicode_FindSlot (char* Message, int FadeTimeout) {
		int i, y, split_size = 0, done = 0, linesize;
		char split[MAX_SPLIT_PARTS][MAX_LINE_SIZE], stripped[MAX_LINE_SIZE]; 

		if ( clu.Unicode_LineSize->integer < (MAX_LINE_SIZE / MAX_SPLIT_PARTS) ) // Hence split size can be limited.
			linesize = MAX_LINE_SIZE / MAX_SPLIT_PARTS;
		else if (clu.Unicode_LineSize->integer > MAX_LINE_SIZE)
			linesize = MAX_LINE_SIZE;
		else
			linesize = clu.Unicode_LineSize->integer;

		/*	Strip the message from q3 colors and new lines. New lines could be dealt with differently, but
			we disrupt the normal new line procedure anyway with the process that follows. */
		for (i = 0, y =0; i < strlen(Message); i++) {
			if (Message[i] == '^') { 
				i++;
				continue;
			}
			if (Message[i] == '\n') continue;
			stripped[y++] = Message[i];
		}
		stripped[y] = '\0';

		// Split it into lines if necessary.
		if (strlen(stripped) > linesize) {
			
			for ( ; done < strlen(stripped) ; ) {

				if ( strlen(stripped) > done + linesize ) {
					strncpy( split[split_size], stripped + done, linesize );
					split[split_size][linesize ] = '\0';
					done += linesize;
				} else {
					strncpy( split[split_size], stripped + done, strlen(stripped + done) );
					split[split_size][strlen(stripped + done)] = '\0';
					done += strlen(split[split_size]);
				}

				split_size++;
			}

		} else {
			strcpy(split[0], stripped);
			split_size++;
		}

			for (i = 0; i < split_size; i++) {

				// Find a slot
				for (y = 0; y < clu.Unicode_Lines->integer && y < MAX_UNICODE_LINES; y++) {
					if (Unicode_Text[y].FadeTimeout == 0) {// empty slot identified
						strcpy(Unicode_Text[y].Message, split[i]);
						Unicode_Text[y].FadeTimeout = FadeTimeout;
						Unicode_Text[y].FadeInitTime = Sys_Milliseconds();
						Unicode_used_slots++;
						break;
					}
				}
			}

	}

	void Unicode_Init (void) {
				
		static qboolean spammed; 

		if(clu.Unicode_Fontsize->modified) clu.Unicode_Fontsize->modified = qfalse;
		if(clu.Unicode_Font->modified) clu.Unicode_Font->modified = qfalse;

		// The Texture font type is relatively fast (since the textures are on GPU memory) and antialised.
		FTGL_font = qftglCreateTextureFont(clu.Unicode_Font->string);
				
		if(FTGL_font) {
			if (!spammed) { // we go through this each vid_restart
				Com_Printf("* Unicode support Initialized\n");
				Unicode_FindSlot(clu.Unicode_Greeting->string,  clu.Unicode_MessageTime->integer + 3000);
				spammed = qtrue;
			}
		} else {
			Com_Printf("^3Unicode support was not Initialized. Is /Unicode_Font '%s' in the path?\n", clu.Unicode_Font->string);
		}

		// Set the font size; based only on width (we usually read that way)
		qftglSetFontFaceSize(FTGL_font, (int)(clu.Unicode_Fontsize->value * glConfig.vidWidth) , 0);
				
		// Set the character map
		//ftglSetFontCharMap(font, ft_encoding_unicode);
	}
			
	// Based on RB_SetGL2D(): FIXME? Code duplication can probably be avoided.
	static void Unicode_Setup2D (void) {
		backEnd.projection2D = qtrue;

		// set 2D virtual screen size
		qglViewport( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
		qglScissor( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
		qglMatrixMode(GL_PROJECTION);
		qglLoadIdentity ();
		qglOrtho (0, glConfig.vidWidth, glConfig.vidHeight, 0, -200, 200); // This changed. FIXME? Perhaps do it beter.
		qglMatrixMode(GL_MODELVIEW);
		qglLoadIdentity ();
				
		GL_State( GLS_DEPTHTEST_DISABLE |
					GLS_SRCBLEND_SRC_ALPHA |
					GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

		qglDisable( GL_CULL_FACE );
		qglDisable( GL_CLIP_PLANE0 );
		qglDisable( GL_CLIP_PLANE1 );

		// set time for 2D shaders
		//backEnd.refdef.time = ri.Milliseconds();
		//backEnd.refdef.floatTime = backEnd.refdef.time * 0.001f;
	}

	void Unicode_Render (void){
	
		int i, list_position;
	
		if (
			!Unicode_used_slots || // Mainly for saving cycles
			!FTGL_font ||
			cls.state == CA_CINEMATIC || // Not when a video is playing (it also distorts it)
			cls.state == CA_LOADING // It's plain ugly otherwise.
		) return; 
			
		// If font size or file changed, reload the font;
		if (clu.Unicode_Fontsize->modified || clu.Unicode_Font->modified) {

			Com_Printf("TTF Font file or font size changed; reinitializing FTGL.\n");
			clu.Unicode_Shutdown();
			Unicode_Init();

		}
				
		Unicode_Setup2D(); // Set up 2D Projection
			
		for (i = 0, list_position = 0; i < clu.Unicode_Lines->integer &&  i < MAX_UNICODE_LINES; i++) {
			GLfloat fade_value;
			int32_t fade_pos;
		
			if (!Unicode_Text[i].FadeTimeout) continue; // empty slot, do nothing

			fade_pos =Unicode_Text[i].FadeInitTime + Unicode_Text[i].FadeTimeout - Sys_Milliseconds();
			if (fade_pos < 0) {
				Unicode_Text[i].FadeTimeout = 0; // mark it as an empty slot
				Unicode_used_slots--;
				continue;
			}
						
			if (fade_pos > clu.Unicode_MessageFadeTime->integer) 
				fade_value = (GLfloat)1 * clu.Unicode_Alpha->value;
			else
				fade_value = (GLfloat) fade_pos * clu.Unicode_Alpha->value/clu.Unicode_MessageFadeTime->integer;

				
			// Shadow:
			glPushMatrix();	
							
				Cmd_TokenizeString(clu.Unicode_Shadow->string);
				glTranslatef(
					(GLfloat)glConfig.vidWidth * clu.Unicode_Fontsize->value * clu.Unicode_PositionX->value * atof(Cmd_Argv(0)), // shadow moves slightly
					(GLfloat)glConfig.vidHeight * clu.Unicode_Fontsize->value * atof(Cmd_Argv(1)) * (clu.Unicode_PositionY->value - list_position * clu.Unicode_LineSpace->value), 
					0 
				);

				glRotatef(180.0f, 1.0f, 0.0f, 0.0f); // rotated.. fixme?
					
				glColor4f(0.0, 0.0, 0.0, fade_value);

				qftglRenderFont(FTGL_font, Unicode_Text[i].Message, FTGL_RENDER_ALL);		
						
			glPopMatrix();	
	
			// Visible text:
			glPushMatrix();	

				glTranslatef(
					(GLfloat)glConfig.vidWidth * clu.Unicode_Fontsize->value * clu.Unicode_PositionX->value,

					(GLfloat)glConfig.vidHeight * clu.Unicode_Fontsize->value * (clu.Unicode_PositionY->value - list_position * clu.Unicode_LineSpace->value) , 
								
					0 
				);

				glRotatef(180.0f, 1.0f, 0.0f, 0.0f);

				Cmd_TokenizeString(clu.Unicode_Color->string);
				glColor4f(
					(GLfloat)atof(Cmd_Argv(0))/255.0, 
					(GLfloat)atof(Cmd_Argv(1))/225.9, 
					(GLfloat)atof(Cmd_Argv(2))/255.0,
					fade_value
				);
				qftglRenderFont(FTGL_font, Unicode_Text[i].Message, FTGL_RENDER_ALL);
			glPopMatrix();

			list_position++;
		}
	}
			
	// Used on vid_restart and font change:
	void Unicode_Shutdown (void) {
		
		// Destroy the font object.
		if (FTGL_font)
			qftglDestroyFont(FTGL_font);
	}

	void Unicode_ChatOutput_f (char* Message) {
		
		// this is based solely on Com_Translate_Auto():

		int i, y;
		char copy[256], *pointer, name[128], name_loc[128];
		const char dead[] = "(DEAD) ", spec[] = "(SPEC) ", dead_spec[] = "(DEAD) (SPEC) ";
		size_t ignore = 0;
		qboolean proceed = qfalse;

		// remove '^' and the next character (q3 coloring)
		for (i = 0, y =0; i < strlen(Message); i++) {
			if ( Message[i] == '^') { 
				i++;
				continue;
			}
			copy[y++] = (Message)[i];
		}
		copy[y] = '\0';

		// Make sure the message came from one of the players
		for (i = 0; i < MAX_CLIENTS; i++) {
			// credit to Roman Tetelman from the mailing list:
			pointer = Info_ValueForKey( cl.gameState.stringData + cl.gameState.stringOffsets[ CS_PLAYERS +i ], "n" );
			if (strlen(pointer)) {

				// Append the ': '; this avoids quirks on kill messages and with names ending with ':'
				strcpy(name, pointer);
				strcat(name, ": "); 

				// Location names like UrT's:
				strcpy(name_loc, pointer);
				strcat(name_loc, " ("); 

				// If it has '(DEAD) ' or '(SPEC) ' of UrT ignore it
				if (
					(
						(pointer = strstr(copy, dead)) &&
						strlen(pointer) == strlen(copy)	// It was in the beginning
					) ||
					(
						(pointer = strstr(copy, spec))  &&
						strlen(pointer) == strlen(copy)
					)
				) ignore = strlen(dead);

				if ( // No 'else if' because dead will catch dead_spec
					(
						(pointer = strstr(copy, dead_spec))  && /*	a dead that went spec prints '(DEAD) (SPEC)'
																	or a buggy condition */
						strlen(pointer) == strlen(copy)
					)    
				) ignore = strlen(dead_spec);
				
				if	(		
						(
							(pointer = strstr(copy + ignore, name)) &&
							(strlen(pointer) == strlen(copy + ignore)) 
						) 
						||
						(
							(pointer = strstr(copy + ignore, name_loc)) &&
							(strlen(pointer) == strlen(copy + ignore))
						)
					) {
						proceed = qtrue;
						break;
				}
			}
		}

		if (proceed)
			Unicode_FindSlot(Message, clu.Unicode_MessageTime->integer);	
	}

#endif
#include <FTGL/ftgl.h>

#if !defined(FTGL_NODLOPEN)
	FTGLfont* (*qftglCreateTextureFont)(const char *file);
	int (*qftglSetFontFaceSize)(FTGLfont* font, unsigned int size, unsigned int res);
	void (*qftglRenderFont)(FTGLfont* font, const char *string, int mode);
	void (*qftglDestroyFont)(FTGLfont* font);			
#else
	#define 	qftglCreateTextureFont ftglCreateTextureFont
	#define 	qftglSetFontFaceSize ftglSetFontFaceSize
	#define 	qftglRenderFont ftglRenderFont
	#define qftglDestroyFont ftglDestroyFont
#endif
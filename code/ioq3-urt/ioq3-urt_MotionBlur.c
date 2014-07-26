// under GPL v2

#if !defined(DEDICATED) 

	#include "ioq3-urt.h"

	/*	Basic Motion Blur according to http://tech.bombinaid.com/post.php?id=12
		This appears to be unsupported by ATI even if it's in OpenGL since 1.5.
		Presumably their excuse is that such functionality is usually achieved
		lately via GLSL. */
	void R_MotionBlur_f (void){	
		if (clu.r_motionBlur->integer) {\
			glAccum(GL_MULT, (GLfloat)clu.r_motionBlurStrength->value);
			glAccum(GL_ACCUM, (GLfloat)1 - clu.r_motionBlurStrength->value);
			glAccum(GL_RETURN, (GLfloat)1);
		}
	}

#endif

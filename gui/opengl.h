/*	Public domain	*/

#ifndef _AGAR_GUI_OPENGL_H_
#define _AGAR_GUI_OPENGL_H_

#include <agar/config/have_opengl.h>
#ifdef HAVE_OPENGL

# ifdef _WIN32
#  include <agar/core/win32.h>
# endif
# ifdef _USE_OPENGL_FRAMEWORK
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
# ifdef _AGAR_GUI_INTERNAL
#  include <agar/config/have_glx.h>
#  ifdef HAVE_GLX
#   include <GL/glx.h>
#  endif
# endif

# include <agar/gui/drv_gl_common.h>

#endif /* HAVE_OPENGL */

#endif /* _AGAR_GUI_OPENGL_H_ */

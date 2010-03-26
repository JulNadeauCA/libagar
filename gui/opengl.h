/*	Public domain	*/

#ifndef _AGAR_GUI_OPENGL_H_
#define _AGAR_GUI_OPENGL_H_

#include <agar/config/have_opengl.h>
#include <agar/config/have_cygwin.h>

#ifdef HAVE_OPENGL
# ifdef _WIN32
#  include <agar/core/win32.h>
# endif
# ifdef __APPLE__
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
#endif

#endif /* _AGAR_GUI_OPENGL_H_ */

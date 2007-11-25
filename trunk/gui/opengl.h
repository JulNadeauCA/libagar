/*	Public domain	*/

#ifndef _AGAR_GUI_OPENGL_H_
#define _AGAR_GUI_OPENGL_H_

#ifdef _AGAR_INTERNAL
# include <config/have_opengl.h>
#else
# include <agar/config/have_opengl.h>
#endif

#ifdef HAVE_OPENGL
# ifdef _WIN32
#  include <windows.h>
# endif
# ifdef __APPLE__
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
#endif

#endif /* _AGAR_GUI_OPENGL_H_ */

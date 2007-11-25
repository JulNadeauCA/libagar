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
#  ifdef _AGAR_INTERNAL
#   include <core/queue_close.h>	/* Conflicts with <windows.h> */
#  else
#   include <agar/core/queue_close.h>
#  endif
#  include <windows.h>
#  ifdef _AGAR_INTERNAL
#   include <core/queue_close.h>
#   include <core/queue.h>
#  else
#   include <agar/core/queue_close.h>
#   include <agar/core/queue.h>
#  endif
# endif
# ifdef __APPLE__
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
#endif

#endif /* _AGAR_GUI_OPENGL_H_ */

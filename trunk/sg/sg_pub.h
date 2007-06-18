/*	Public domain	*/

#ifndef _AGAR_SG_PUBLIC_H_
#define _AGAR_SG_PUBLIC_H_

#include <agar/core/core_begin.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>
#include <agar/core/core_close.h>

#include <agar/config/have_opengl.h>
#ifdef HAVE_OPENGL
# ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
# else
#  include <GL/gl.h>
#  include <GL/glu.h>
# endif
#endif

#endif /* _AGAR_SG_PUBLIC_H_ */

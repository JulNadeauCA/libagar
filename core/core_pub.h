/*	Public domain	*/

#ifndef _AGAR_CORE_PUBLIC_H_
#define _AGAR_CORE_PUBLIC_H_
#include <agar/core/core_begin.h>

#include <agar/core/error_pub.h>

#include <agar/core/data_source.h>
#include <agar/core/load_den.h>
#include <agar/core/load_color.h>
#include <agar/core/load_integral.h>
#include <agar/core/load_real.h>
#include <agar/core/load_string.h>
#include <agar/core/load_surface.h>
#include <agar/core/load_version.h>

#include <agar/core/object.h>
#include <agar/core/event.h>
#include <agar/core/config.h>
#include <agar/core/rcs.h>
#include <agar/core/typesw.h>
#include <agar/core/core_init.h>

#include <agar/config/have_opengl.h>
#ifdef HAVE_OPENGL
# ifdef __APPLE__
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
#endif

#include <agar/core/core_close.h>
#endif

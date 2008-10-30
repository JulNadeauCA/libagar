/*	Public domain	*/
/*
 * Output compile information for ag_core and ag_gui libraries.
 */

static void PrintUsage(char *);
static void OutputCFLAGS(void);
static void OutputLIBS(void);

#include "agar-config-generic.h"

#include <config/threads.h>
#include <config/network.h>

#include <config/have_sdl.h>
#ifdef HAVE_SDL
#include <config/sdl_libs.h>
#include <config/sdl_cflags.h>
#endif

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL
#include <config/opengl_libs.h>
#include <config/opengl_cflags.h>
#endif

#include <config/have_freetype.h>
#ifdef HAVE_FREETYPE
#include <config/freetype_libs.h>
#include <config/freetype_cflags.h>
#endif

#include <config/have_jpeg.h>
#ifdef HAVE_JPEG
#include <config/jpeg_libs.h>
#include <config/jpeg_cflags.h>
#endif

#include <config/have_math.h>
#ifdef HAVE_MATH
#include <config/math_libs.h>
#include <config/math_cflags.h>
#endif

#include <config/have_pthreads.h>
#ifdef HAVE_PTHREADS
#include <config/pthreads_libs.h>
#include <config/pthreads_cflags.h>
#endif

#include <config/enable_nls.h>
#ifdef ENABLE_NLS
#include <config/gettext_libs.h>
#include <config/gettext_cflags.h>
#endif

#include <config/dso_libs.h>
#include <config/dso_cflags.h>

#if defined(__APPLE__) || defined(__MACOSX__)
# include <AvailabilityMacros.h>
#endif

const struct config_string_opt stringOpts[] = {
	GENERIC_STRING_OPTS,
#ifdef THREADS
	{ "--threads",	"yes" },
#else
	{ "--threads",	"no" },
#endif
#ifdef NETWORK
	{ "--network",	"yes" },
#else
	{ "--network",	"no" },
#endif
#ifdef HAVE_SDL
	{ "--sdl",	"yes" },
#else
	{ "--sdl",	"no" },
#endif
#ifdef HAVE_OPENGL
	{ "--opengl",	"yes" },
#else
	{ "--opengl",	"no" },
#endif
#ifdef HAVE_FREETYPE
	{ "--freetype",	"yes" },
#else
	{ "--freetype",	"no" },
#endif
};
const int nStringOpts = sizeof(stringOpts) / sizeof(stringOpts[0]);

static void
PrintUsage(char *name)
{
	fprintf(stderr, GENERIC_USAGE_STRING, name);
	fprintf(stderr,
	    "[--threads] [--network] [--sdl] [--opengl] [--freetype]\n");
}

static void
OutputCFLAGS(void)
{
	printf("-I%s ", INCLDIR);
#ifdef SDL_CFLAGS
	printf("%s ", SDL_CFLAGS);
#endif
#ifdef FREETYPE_CFLAGS
	printf("%s ", FREETYPE_CFLAGS);
#endif
#ifdef OPENGL_CFLAGS
	printf("%s ", OPENGL_CFLAGS);
#endif
#ifdef MATH_CFLAGS
	printf("%s ", MATH_CFLAGS);
#endif
#ifdef JPEG_CFLAGS
	printf("%s ", JPEG_CFLAGS);
#endif
#ifdef HAVE_PTHREADS
	printf("%s ", PTHREADS_CFLAGS);
#endif
#ifdef ENABLE_NLS
	printf("%s ", GETTEXT_CFLAGS);
#endif
#ifdef DSO_CFLAGS
	printf("%s ", DSO_CFLAGS);
#endif
	printf("\n");
}

static void
OutputLIBS(void)
{
	printf("-L%s ", LIBDIR);
	printf("-lag_gui -lag_core ");
#ifdef SDL_LIBS
	printf("%s ", SDL_LIBS);
#endif
#ifdef FREETYPE_LIBS
	printf("%s ", FREETYPE_LIBS);
#endif
#ifdef OPENGL_LIBS
	printf("%s ", OPENGL_LIBS);
#endif
#ifdef MATH_LIBS
	printf("%s ", MATH_LIBS);
#endif
#ifdef JPEG_LIBS
	printf("%s ", JPEG_LIBS);
#endif
#ifdef HAVE_PTHREADS
	printf("%s ", PTHREADS_LIBS);
#endif
#ifdef ENABLE_NLS
	printf("%s ", GETTEXT_LIBS);
#endif
#ifdef DSO_LIBS
	printf("%s ", DSO_LIBS);
#endif
#if (defined(__APPLE__) || defined(__MACOSX__)) && defined(MAC_OS_X_VERSION_10_5)
	printf("-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib");
#endif
	printf("\n");
}

int
main(int argc, char *argv[])
{
	return GenericFooConfig(stringOpts, nStringOpts, argc, argv);
}

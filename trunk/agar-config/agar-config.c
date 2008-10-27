/*
 * Copyright (c) 2004-2007 Hypertriton, Inc. <http://hypertriton.com/>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <config/version.h>
#include <config/debug.h>
#include <config/release.h>
#include <config/prefix.h>
#include <config/sysconfdir.h>
#include <config/incldir.h>
#include <config/libdir.h>
#include <config/sharedir.h>
#include <config/ttfdir.h>
#include <config/localedir.h>

#include <config/have_sdl.h>
#include <config/have_opengl.h>
#include <config/have_freetype.h>
#include <config/have_jpeg.h>
#include <config/have_math.h>
#include <config/have_pthreads.h>
#include <config/threads.h>
#include <config/network.h>
#include <config/enable_nls.h>

#ifdef HAVE_SDL
# include <config/sdl_libs.h>
# include <config/sdl_cflags.h>
#endif
#ifdef HAVE_OPENGL
# include <config/opengl_libs.h>
# include <config/opengl_cflags.h>
#endif
#ifdef HAVE_FREETYPE
# include <config/freetype_libs.h>
# include <config/freetype_cflags.h>
#endif
#ifdef HAVE_JPEG
# include <config/jpeg_libs.h>
# include <config/jpeg_cflags.h>
#endif
#ifdef HAVE_MATH
# include <config/math_libs.h>
# include <config/math_cflags.h>
#endif
#ifdef HAVE_PTHREADS
# include <config/pthreads_libs.h>
# include <config/pthreads_cflags.h>
#endif
#ifdef ENABLE_NLS
# include <config/gettext_libs.h>
# include <config/gettext_cflags.h>
#endif

#include <config/dso_libs.h>
#include <config/dso_cflags.h>

#include <stdio.h>
#include <string.h>

#if defined(__APPLE__) || defined(__MACOSX__)
# include <AvailabilityMacros.h>
#endif

const struct {
	const char *opt;
	const char *data;
} stringOpts[] = {
	{ "--version",		VERSION },
	{ "--release",		RELEASE },
	{ "--prefix",		PREFIX },
	{ "--sysconfdir",	SYSCONFDIR },
	{ "--incldir",		INCLDIR },
	{ "--libdir",		LIBDIR },
	{ "--sharedir",		SHAREDIR },
	{ "--ttfdir",		TTFDIR },
	{ "--localedir",	LOCALEDIR },
#ifdef THREADS
	{ "--threads",		"yes" },
#else
	{ "--threads",		"no" },
#endif
#ifdef NETWORK
	{ "--network",		"yes" },
#else
	{ "--network",		"no" },
#endif

#ifdef HAVE_SDL
	{ "--have-sdl",		"yes" },
#else
	{ "--have-sdl",		"no" },
#endif
#ifdef HAVE_OPENGL
	{ "--have-opengl",	"yes" },
#else
	{ "--have-opengl",	"no" },
#endif
#ifdef HAVE_FREETYPE
	{ "--have-freetype",	"yes" },
#else
	{ "--have-freetype",	"no" },
#endif
};
const int nStringOpts = sizeof(stringOpts) / sizeof(stringOpts[0]);

static void
PrintUsage(char *name)
{
	fprintf(stderr,
	    "Usage: %s [--version] [--release] [--prefix] "
	    "[--sysconfdir] [--incldir] [--libdir] [--sharedir] "
	    "[--ttfdir] [--localedir] "
	    "[--threads] [--network] "
	    "[--have-sdl] [--have-opengl] [--have-freetype "
	    "[--cflags] [--libs]\n", name);
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
	int i, j;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--cflags") == 0) {
			OutputCFLAGS();
		} else if (strcmp(argv[i], "--libs") == 0) {
			OutputLIBS();
		} else {
			for (j = 0; j < nStringOpts; j++) {
				if (strcmp(argv[i], stringOpts[j].opt) == 0)
					break;
			}
			if (j < nStringOpts) {
				printf("%s\n", stringOpts[j].data);
				continue;
			}
			printf("No such option: %s\n", argv[i]);
			PrintUsage(argv[0]);
			return (1);
		}
	}
	if (i <= 1) {
		PrintUsage(argv[0]);
		return (1);
	}
	return (0);
}


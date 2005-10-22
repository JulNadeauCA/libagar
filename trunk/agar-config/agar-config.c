/*	$Csoft: agar-config.c,v 1.18 2005/09/18 04:59:10 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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
#include <config/enable_nls.h>
#include <config/prefix.h>
#include <config/sysconfdir.h>
#include <config/incldir.h>
#include <config/inclpdir.h>
#include <config/libdir.h>
#include <config/sharedir.h>
#include <config/ttfdir.h>
#include <config/localedir.h>
#include <config/network.h>
#include <config/bsd_source_needed.h>

#include <config/have_freetype.h>
#include <config/have_opengl.h>
#include <config/have_jpeg.h>
#include <config/have_libqnet.h>

#include <config/sdl_libs.h>
#include <config/sdl_cflags.h>

#include <config/math_libs.h>

#ifdef HAVE_FREETYPE
#include <config/freetype_libs.h>
#include <config/freetype_cflags.h>
#endif
#ifdef HAVE_OPENGL
#include <config/opengl_libs.h>
#include <config/opengl_cflags.h>
#endif
#ifdef HAVE_JPEG
#include <config/jpeg_libs.h>
#include <config/jpeg_cflags.h>
#endif
#ifdef HAVE_LIBQNET
#include <config/libqnet_libs.h>
#include <config/libqnet_cflags.h>
#endif

#include <stdio.h>
#include <string.h>

int
main(int argc, char *argv[])
{
	int i;

	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--version") == 0) {
			printf("%s\n", VERSION);
		} else if (strcmp(argv[i], "--prefix") == 0) {
			printf("%s\n", PREFIX);
		} else if (strcmp(argv[i], "--sysconfdir") == 0) {
			printf("%s\n", SYSCONFDIR);
		} else if (strcmp(argv[i], "--incldir") == 0) {
			printf("%s\n", INCLPDIR);
		} else if (strcmp(argv[i], "--libdir") == 0) {
			printf("%s\n", LIBDIR);
		} else if (strcmp(argv[i], "--sharedir") == 0) {
			printf("%s\n", SHAREDIR);
		} else if (strcmp(argv[i], "--ttfdir") == 0) {
			printf("%s\n", TTFDIR);
		} else if (strcmp(argv[i], "--localedir") == 0) {
			printf("%s\n", LOCALEDIR);
		} else if (strcmp(argv[i], "--cflags") == 0) {
#ifdef BSD_SOURCE_NEEDED
			printf("-D_BSD_SOURCE ");
#endif
			printf("-I%s ", INCLPDIR);
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
#if defined(LIBQNET_CFLAGS) && defined(NETWORK)
			printf("%s ", LIBQNET_CFLAGS);
#endif
			printf("\n");
		} else if (strcmp(argv[i], "--libs") == 0) {
			printf("-L%s ", LIBDIR);
			printf("-lag_core -lag_core_monitor -lag_game_map "
			       "-lag_game -lag_rg -lag_vg -lag_gui "
			       "-lag_core_loaders "
			       "-lag_compat -lag_mat ");
#if defined(ENABLE_NLS) && !defined(__linux__) /* XXX */
			printf("-lag_intl ");
#endif
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
#if defined(LIBQNET_LIBS) && defined(NETWORK)
			printf("%s ", LIBQNET_LIBS);
#endif
			printf("\n");
		}
	}
	if (i <= 1) {
		fprintf(stderr,
		    "Usage: %s [--version] [--prefix] [--sysconfdir] "
		    "[--incldir] [--libdir] [--sharedir] [--ttfdir] "
		    "[--localedir] [--cflags] [--libs]\n", argv[0]);
		return (1);
	}
	return (0);
}


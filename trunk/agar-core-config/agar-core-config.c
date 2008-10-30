/*	Public domain	*/
/*
 * Output compile information for ag_core library.
 */

static void PrintUsage(char *);
static void OutputCFLAGS(void);
static void OutputLIBS(void);

#include "../agar-config/agar-config-generic.h"

/* XXX */
#include <config/have_sdl.h>
#ifdef HAVE_SDL
#include <config/sdl_libs.h>
#include <config/sdl_cflags.h>
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

#include <config/threads.h>
#include <config/network.h>
#include <config/dso_libs.h>
#include <config/dso_cflags.h>

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
};
const int nStringOpts = sizeof(stringOpts) / sizeof(stringOpts[0]);

static void
PrintUsage(char *name)
{
	fprintf(stderr, GENERIC_USAGE_STRING, name);
	fprintf(stderr, "[--threads] [--network]\n");
}

static void
OutputCFLAGS(void)
{
	printf("-I%s ", INCLDIR);
#ifdef SDL_CFLAGS
	printf("%s ", SDL_CFLAGS);
#endif
#ifdef MATH_CFLAGS
	printf("%s ", MATH_CFLAGS);
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
	printf("-L%s -lag_core ", LIBDIR);
#ifdef SDL_LIBS
	printf("%s ", SDL_LIBS);
#endif
#ifdef MATH_LIBS
	printf("%s ", MATH_LIBS);
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
	printf("\n");
}

int
main(int argc, char *argv[])
{
	return GenericFooConfig(stringOpts, nStringOpts, argc, argv);
}

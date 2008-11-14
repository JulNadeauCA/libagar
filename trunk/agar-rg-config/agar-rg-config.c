/*	Public domain	*/
/*
 * Output compile information for ag_rg library.
 */

static void PrintUsage(char *);
static void OutputCFLAGS(void);
static void OutputLIBS(void);

#include "../agar-config/agar-config-generic.h"

#include <config/enable_nls.h>
#ifdef ENABLE_NLS
#include <config/gettext_libs.h>
#include <config/gettext_cflags.h>
#endif

#include <config/ag_threads.h>

const struct config_string_opt stringOpts[] = {
	GENERIC_STRING_OPTS,
#ifdef AG_THREADS
	{ "--threads",	"yes" },
#else
	{ "--threads",	"no" },
#endif
};
const int nStringOpts = sizeof(stringOpts) / sizeof(stringOpts[0]);

static void
PrintUsage(char *name)
{
	fprintf(stderr, GENERIC_USAGE_STRING, name);
	fprintf(stderr, "[--threads]\n");
}

static void
OutputCFLAGS(void)
{
	printf("-I%s ", INCLDIR);
#ifdef ENABLE_NLS
	printf("%s ", GETTEXT_CFLAGS);
#endif
	printf("\n");
}

static void
OutputLIBS(void)
{
	printf("-L%s ", LIBDIR);
	printf("-lag_rg ");
#ifdef ENABLE_NLS
	printf("%s ", GETTEXT_LIBS);
#endif
	printf("\n");
}

int
main(int argc, char *argv[])
{
	return GenericFooConfig(stringOpts, nStringOpts, argc, argv);
}

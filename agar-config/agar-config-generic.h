/*	Public domain	*/
/*
 * Generic definitions for the agar-foo-config(1) programs.
 */

#include <stdio.h>
#include <string.h>

#include <config/version.h>
#include <config/ag_debug.h>
#include <config/release.h>
#include <config/prefix.h>
#include <config/sysconfdir.h>
#include <config/incldir.h>
#include <config/libdir.h>
#include <config/sharedir.h>
#include <config/ttfdir.h>
#include <config/localedir.h>

struct config_string_opt {
	const char *opt;
	const char *data;
};

#define GENERIC_USAGE_STRING \
	"Usage: %s [--help] [--version] [--release] [--cflags] [--libs]" \
	"[--prefix] [--sysconfdir] [--incldir] [--libdir] [--sharedir] " \
	"[--ttfdir] [--localedir] "

#define GENERIC_STRING_OPTS \
	{ "--version",	 VERSION }, \
	{ "--release",	 RELEASE }, \
	{ "--prefix",	 PREFIX }, \
	{ "--sysconfdir",SYSCONFDIR }, \
	{ "--incldir",	 INCLDIR }, \
	{ "--libdir",	 LIBDIR }, \
	{ "--sharedir",	 SHAREDIR }, \
	{ "--ttfdir",	 TTFDIR }, \
	{ "--localedir", LOCALEDIR }

static int
GenericFooConfig(const struct config_string_opt *opts, int nOpts, int argc,
    char **argv)
{
	int i, j;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--cflags") == 0) {
			OutputCFLAGS();
		} else if (strcmp(argv[i], "--libs") == 0) {
			OutputLIBS();
		} else {
			for (j = 0; j < nOpts; j++) {
				if (strcmp(argv[i], opts[j].opt) == 0)
					break;
			}
			if (j < nOpts) {
				printf("%s\n", opts[j].data);
				continue;
			}
			if (strcmp(argv[i], "--help") != 0) {
				printf("No such option: %s\n", argv[i]);
			}
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


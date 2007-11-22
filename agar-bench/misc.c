/*	$Csoft: surfaceops.c,v 1.3 2005/10/03 07:17:31 vedge Exp $	*/
/*	Public domain	*/

#include "agar-bench.h"

#include <stdarg.h>
#include <string.h>

static char buf1[1024];
static char buf2[1024];
#define STRING64 "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
static float flt = 1.0, dbl = 1.0;


static void
do_valist(int d, ...)
{
	va_list ap;

	va_start(ap, d);
	va_arg(ap, int);
	va_end(ap);
}

static void T_Valist(void) {
	do_valist(1, 2);
}
static void T_Strlcpy64(void) {
	AG_Strlcpy(buf1, STRING64, sizeof(buf1));
}
static void T_Strlcat64(void) {
	AG_Strlcat(buf1, STRING64, sizeof(buf1));
}
static void T_Strlcpy1k(void) {
	AG_Strlcpy(buf1, buf2, sizeof(buf1));
}
static void T_Strlcat1k(void) {
	AG_Strlcat(buf1, buf2, sizeof(buf1));
}
static void T_Snprintf64(void) {
	snprintf(buf1, sizeof(buf1), STRING64);
}
static void T_Snprintf4(void) {
	snprintf(buf1, sizeof(buf1), "%d,%d,%s,%s", 1, 1, STRING64, STRING64);
}

static struct testfn_ops testfns[] = {
 { "va_list(int)", NULL, NULL, T_Valist },
 { "strlcpy(1k)", NULL, NULL, T_Strlcpy1k },
 { "strlcat(1k)", NULL, NULL, T_Strlcat1k },
 { "strlcpy(64B)", NULL, NULL, T_Strlcpy64 },
 { "strlcat(64B)", NULL, NULL, T_Strlcat64 },
 { "snprintf(64B)", NULL, NULL, T_Snprintf64 },
 { "snprintf(%d,%d,%s,%s)", NULL, NULL, T_Snprintf4 },
};

struct test_ops misc_test = {
	"Misc",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0]),
	0,
	4, 1000, 0
};

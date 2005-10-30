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
do_vsnprintf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf1, sizeof(buf1), fmt, ap);
	va_end(ap);
}

static void
do_valist(int d, ...)
{
	va_list ap;

	va_start(ap, d);
	va_arg(ap, int);
	va_end(ap);
}

static void T_Valist(void) { do_valist(1, 2); }
static void T_Vsnprintf64(void) { do_vsnprintf(STRING64); }

static void T_Vsnprintf4(void) {
	do_vsnprintf("%d %d %s %s", 1, 1, STRING64, STRING64);
}
static void T_Strlcpy64(void) {
	strlcpy(buf1, STRING64, sizeof(buf1));
}
static void T_Strlcat64(void) {
	strlcat(buf1, STRING64, sizeof(buf1));
}
static void T_Strlcpy1k(void) {
	strlcpy(buf1, buf2, sizeof(buf1));
}
static void T_Strlcat1k(void) {
	strlcat(buf1, buf2, sizeof(buf1));
}
static void T_Snprintf64(void) {
	snprintf(buf1, sizeof(buf1), STRING64);
}
static void T_Snprintf4(void) {
	snprintf(buf1, sizeof(buf1), "%d,%d,%s,%s", 1, 1, STRING64, STRING64);
}

struct vector {
	double x, y, z, w;
};

struct vector
scopy_mul(struct vector v1, struct vector v2)
{
	struct vector vr;

	vr.x = v1.x * v2.x;
	vr.y = v1.y * v2.y;
	vr.z = v1.z * v2.z;
	vr.w = v1.w * v2.w;
	return (vr);
}
void
spass_mul(struct vector *vr, struct vector *v1, struct vector *v2)
{
	vr->x = v1->x * v2->x;
	vr->y = v1->y * v2->y;
	vr->z = v1->z * v2->z;
	vr->w = v1->w * v2->w;
}
struct vector
scopy_add(struct vector v1, struct vector v2)
{
	struct vector vr;

	vr.x = v1.x + v2.x;
	vr.y = v1.y + v2.y;
	vr.z = v1.z + v2.z;
	vr.w = v1.w + v2.w;
	return (vr);
}
void
spass_add(struct vector *vr, struct vector *v1, struct vector *v2)
{
	vr->x = v1->x + v2->x;
	vr->y = v1->y + v2->y;
	vr->z = v1->z + v2->z;
	vr->w = v1->w + v2->w;
}
static void T_StructCopyMul(void) {
	struct vector v1;
	struct vector v2;
	struct vector vr;
	vr = scopy_mul(v1, v2);
}
static void T_StructPassMul(void) {
	struct vector v1;
	struct vector v2;
	struct vector vr;
	spass_mul(&vr, &v1, &v2);
}
static void T_StructCopyAdd(void) {
	struct vector v1;
	struct vector v2;
	struct vector vr;
	vr = scopy_add(v1, v2);
}
static void T_StructPassAdd(void) {
	struct vector v1;
	struct vector v2;
	struct vector vr;
	spass_add(&vr, &v1, &v2);
}


static struct testfn_ops testfns[] = {
 { "va_list(int)", NULL, NULL, T_Valist },
 { "strlcpy(1k)", NULL, NULL, T_Strlcpy1k },
 { "strlcat(1k)", NULL, NULL, T_Strlcat1k },
 { "strlcpy(64B)", NULL, NULL, T_Strlcpy64 },
 { "strlcat(64B)", NULL, NULL, T_Strlcat64 },
 { "vsnprintf(64B)", NULL, NULL, T_Vsnprintf64 },
 { "vsnprintf(%d,%d,%s,%s)", NULL, NULL, T_Vsnprintf4 },
 { "snprintf(64B)", NULL, NULL, T_Snprintf64 },
 { "snprintf(%d,%d,%s,%s)", NULL, NULL, T_Snprintf4 },
 { "vector mul (struct copy)", NULL, NULL, T_StructCopyMul },
 { "vector mul (struct pass)", NULL, NULL, T_StructPassMul },
 { "vector add (struct copy)", NULL, NULL, T_StructCopyAdd },
 { "vector add (struct pass)", NULL, NULL, T_StructPassAdd },
};

struct test_ops misc_test = {
	"Misc",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0]),
	0,
	4, 1000
};

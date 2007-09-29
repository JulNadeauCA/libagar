/*	$Csoft: surfaceops.c,v 1.3 2005/10/03 07:17:31 vedge Exp $	*/
/*	Public domain	*/

#include "agar-bench.h"

static void *buf1, *buf2;

#define TESTBUFSIZE 1200*1024

static void
InitBuffers(void)
{
	buf1 = malloc(TESTBUFSIZE);
	buf2 = malloc(TESTBUFSIZE);
}

static void
FreeBuffers(void)
{
	free(buf1);
	free(buf2);
}

static void
memcpyQ(void *dst, const void *src, size_t len)
{
	size_t i;

	for (i = 0; i < (len>>3); i++)  {
		__asm__ __volatile__ (
		    "movq (%0), %%mm0\n"
		    "movq %%mm0, (%1)\n"
		    : : "r" (src), "r" (dst) : "memory");
		src += 8;
		dst += 8;
	}
	if (len&7)
		memcpy(dst, src, len&7);
}

static void Test_Memcpy(void) { memcpy(buf1, buf2, TESTBUFSIZE); }
static void Test_Memmove(void) { memmove(buf1, buf2, TESTBUFSIZE); }
static void Test_MemcpyQ(void) { memcpyQ(buf1, buf2, TESTBUFSIZE); }

static struct testfn_ops testfns[] = {
 { "memcpy(4M)", InitBuffers, FreeBuffers, Test_Memcpy },
 { "memcpyQ(4M)", InitBuffers, FreeBuffers, Test_MemcpyQ },
 { "memmove(4M)", InitBuffers, FreeBuffers, Test_Memmove }
};

struct test_ops memops_test = {
	"Memory operations",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0]),
	0,
	4, 10, 0
};

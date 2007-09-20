/*	$Csoft: surfaceops.c,v 1.3 2005/10/03 07:17:31 vedge Exp $	*/
/*	Public domain	*/

#include "agar-bench.h"
#include <agar/sg.h>

static void VectorLen3(void)
{
	SG_Vector v;
	float len;
	float *pLen = &len;

	v.x = 4.201401;
	v.y = 3.298172;
	v.z = 0.062581;
	len = SG_VectorLen(v);
}

static struct testfn_ops testfns[] = {
 { "VectorLen3()", NULL, NULL, VectorLen3 },
};

struct test_ops sg_test = {
	"SG",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0]),
	0,
	4, 1000
};

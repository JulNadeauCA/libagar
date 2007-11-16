/*	$Csoft: misc.c,v 1.1 2005/10/04 16:12:31 vedge Exp $	*/
/*	Public domain	*/

#include "agar-bench.h"

#include <stdarg.h>
#include <string.h>

static AG_Object obj;

static void InitObj(void)
{
	AG_ObjectInitStatic(&obj, &agObjectClass);
	AG_SetEvent(&obj, "object-foo-event", NULL, NULL);
	AG_SetEvent(&obj, "object-bar-event", NULL, "%i%i%i%i", 1,2,3,4);
	AG_SetEvent(&obj, "object-baz-event", NULL, "%f%f%f%f%d%d%d%d%p%p%p%p",
	    1.0, 1.0, 1.0, 1.0, 1,1,1,1, NULL, NULL, NULL, NULL);
	AG_SetEvent(&obj, "object-bezo-event", NULL, "%s", "foo");
}
static void FreeObj(void)
{
	AG_ObjectDestroy(&obj);
}

static void T_SetEventWithoutArgs(void) {
	AG_SetEvent(&obj, "foo-event", NULL, NULL);
}
static void T_SetEventWithArgs(void) {
	AG_SetEvent(&obj, "foo-event", NULL, "%p,%i,%f,%d,%s,%D", 1, 1.0, 1.0,
	    "foo bar baz", 1);
}
static void T_PostEventWithoutArgs(void) {
	AG_PostEvent(NULL, &obj, "object-foo-event", NULL);
}
static void T_PostEventWithArgs(void) {
	AG_PostEvent(NULL, &obj, "object-bar-event", "%p,%i,%f,%d,%s,%D",
	    1, 1.0, 1.0, "foo bar baz", 1);
}

static struct testfn_ops testfns[] = {
 { "AG_SetEvent() - Without args", InitObj,FreeObj, T_SetEventWithoutArgs },
 { "AG_SetEvent() - With 6 args", InitObj,FreeObj, T_SetEventWithArgs },
 { "AG_PostEvent() - Without args", InitObj,FreeObj, T_PostEventWithoutArgs },
 { "AG_PostEvent() - With 6 args", InitObj,FreeObj, T_PostEventWithArgs },
};

struct test_ops events_test = {
	"Events",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0]),
	0,
	4, 10000, 0
};

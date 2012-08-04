/*	Public domain	*/
/*
 * This application tests the AG_Timeout(3) interface.
 */
#include "agartest.h"

typedef struct {
	AG_TestInstance _inherit;
	AG_Timeout to1, to2;
	AG_Window *win;
} MyTestInstance;

static Uint32
Timeout1(void *obj, Uint32 ival, void *arg)
{
	TestMsg(arg, "This message should appear first");
	return (0);
}

static Uint32
Timeout2(void *obj, Uint32 ival, void *arg)
{
	TestMsg(arg, "This message should appear second");
	return (0);
}

static void
ScheduleTimeouts(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Object *ob;
	AG_Timeout *to;

	TestMsg(ti, "schedule timeout1 ival=1000");
	AG_ScheduleTimeout(ti->win, &ti->to1, 1000);
	TestMsg(ti, "schedule timeout2 ival=2000");
	AG_ScheduleTimeout(ti->win, &ti->to2, 2000);

	TestMsg(ti, "timeout queue:");

	/* print the timeout tailqueue */
	AG_TAILQ_FOREACH(ob, &agTimeoutObjQ, tobjs) {
		TestMsg(ti, "obj %s :", ob->name);
		AG_TAILQ_FOREACH(to, &ob->timeouts, timeouts) {
			char *name;

			if (to == &ti->to1) { name = "timeout1"; }
			else if (to == &ti->to2) { name = "timeout2"; }
			else { name = "unknown timeout"; }
			TestMsg(ti, "-- timeout %s at %d ticks", name, to->ticks);
		}
	}
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	ti->win = NULL;
	AG_SetTimeout(&ti->to1, Timeout1, ti, 0);
	AG_SetTimeout(&ti->to2, Timeout2, ti, 0);
	return (0);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;

	ti->win = win;
	AG_LabelNewS(win, 0, "This program test the AG_Timeout(3) facility");
	AG_ButtonNewFn(win, 0, "Schedule test timeouts", ScheduleTimeouts, "%p", ti);
	return (0);
}

const AG_TestCase timeoutsTest = {
	"timeouts",
	N_("Test AG_Timeout(3) facility"),
	"1.4.2",
	0,
	sizeof(MyTestInstance),
	Init,
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI
};

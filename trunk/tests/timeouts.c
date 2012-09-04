/*	Public domain	*/
/*
 * This application tests the AG_Timer(3) interface.
 */
#include "agartest.h"

typedef struct {
	AG_TestInstance _inherit;
	AG_Timer to1, to2;
	AG_Window *win;
} MyTestInstance;

static Uint32
Timeout1(AG_Timer *to, AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	TestMsg(ti, "This message should appear first");
	return (0);
}

static Uint32
Timeout2(AG_Timer *to, AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	TestMsg(ti, "This message should appear last");
	return (0);
}

static void
ScheduleTimers(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Object *ob;
	AG_Timer *to;

	TestMsg(ti, "schedule one-shot timers (1s, 2s)");
	AG_AddTimer(NULL, &ti->to1, 1000, Timeout1, "%p", ti);
	AG_AddTimer(NULL, &ti->to2, 2000, Timeout2, "%p", ti);

	TestMsg(ti, "timer queue:");

	/* print the timeout tailqueue */
	AG_TAILQ_FOREACH(ob, &agTimerObjQ, tobjs) {
		TestMsg(ti, "Object %s:", ob->name);
		AG_TAILQ_FOREACH(to, &ob->timers, timers) {
			TestMsg(ti, "\tTimer %d: tSched=%u, ival=%u",
			    to->id, to->tSched, to->ival);
		}
	}
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	ti->win = NULL;
	return (0);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;

	ti->win = win;
	AG_LabelNewS(win, 0, "This program test the AG_Timer(3) facility");
	AG_ButtonNewFn(win, 0, "Schedule test timeouts",
	    ScheduleTimers, "%p", ti);
	return (0);
}

const AG_TestCase timeoutsTest = {
	"timeouts",
	N_("Test AG_Timer(3) facility"),
	"1.5.0",
	0,
	sizeof(MyTestInstance),
	Init,
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

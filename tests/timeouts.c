/*	Public domain	*/
/*
 * This application tests the AG_Timer(3) interface.
 */
#include "agartest.h"

typedef struct {
	AG_TestInstance _inherit;
	AG_Timer to[3], toReg;
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
	TestMsg(ti, "This message should appear second");
	return (0);
}

static Uint32
Timeout3(AG_Timer *to, AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	TestMsg(ti, "This message should appear last");
	return (0);
}

static Uint32
TimeoutRegular(AG_Timer *to, AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	TestMsg(ti, "Tick");
	return (to->ival);
}


static void
TestOneShot(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);

	TestMsg(ti, "Testing 3 one-shot timers");
	if (AG_AddTimer(NULL, &ti->to[0], 1000, Timeout1, "%p", ti) == -1 ||
	    AG_AddTimer(NULL, &ti->to[1], 2000, Timeout2, "%p", ti) == -1 ||
	    AG_AddTimer(NULL, &ti->to[2], 2100, Timeout3, "%p", ti) == -1) {
		TestMsg(ti, "AddTimer: %s", AG_GetError());
	}
}

static void
StartRegular(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);

	TestMsg(ti, "Starting regular timer");
	if (AG_AddTimer(NULL, &ti->toReg, 1000, TimeoutRegular, "%p", ti) == -1) {
		TestMsg(ti, "AddTimer: %s", AG_GetError());
	}
}

static void
StopRegular(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);

	TestMsg(ti, "Stopping regular timer");
	AG_DelTimer(NULL, &ti->toReg);
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	ti->win = NULL;
	AG_InitTimer(&ti->to[0], "testTimer1", 0);
	AG_InitTimer(&ti->to[1], "testTimer2", 0);
	AG_InitTimer(&ti->to[2], "testTimer3", 0);
	AG_InitTimer(&ti->toReg, "testTimerReg", 0);

	return (0);
}

static void
StartTimerInspector(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Window *win;

	if ((win = DEV_TimerInspector()) != NULL) {
		AG_WindowAttach(ti->win, win);
		AG_WindowShow(win);
	}
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;

	ti->win = win;
	AG_LabelNewS(win, 0, "Test for AG_Timer(3) facility");
	AG_LabelNew(win, 0, "timeOps: %s", agTimeOps->name);
	AG_ButtonNewFn(win, 0, "Timer Inspector", StartTimerInspector, "%p", ti);
	AG_ButtonNewFn(win, 0, "Test One-Shot", TestOneShot, "%p", ti);
	AG_ButtonNewFn(win, 0, "Start Regular", StartRegular, "%p", ti);
	AG_ButtonNewFn(win, 0, "Stop Regular", StopRegular, "%p", ti);
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

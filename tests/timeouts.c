/*	Public domain	*/
/*
 * Test the AG_Timer(3) interface.
 */
#include "agartest.h"
#ifdef AG_TIMERS

typedef struct {
	AG_TestInstance _inherit;
	AG_Timer to[3], toReg;
	AG_Window *win;
	Uint tick, period;
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
	TestMsg(ti, "Tick %u", ti->tick++);
	return (to->ival);
}


static void
TestOneShot(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);

	TestMsg(ti, "Testing 3 one-shot timers");
	if (AG_AddTimer(ti->win, &ti->to[0], 1000, Timeout1, "%p", ti) == -1 ||
	    AG_AddTimer(ti->win, &ti->to[1], 2000, Timeout2, "%p", ti) == -1 ||
	    AG_AddTimer(ti->win, &ti->to[2], 2100, Timeout3, "%p", ti) == -1) {
		TestMsg(ti, "AddTimer: %s", AG_GetError());
	}
}

static void
StartPeriodic(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);

	ti->tick = 0;
	TestMsg(ti, "Testing periodic timer at %u ms", ti->period);
	if (AG_AddTimer(ti->win, &ti->toReg, ti->period,
	    TimeoutRegular, "%p", ti) == -1) {
		TestMsg(ti, "AddTimer: %s", AG_GetError());
	}
}

static void
StopPeriodic(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);

	TestMsg(ti, "Stopping periodic timer");
	AG_DelTimer(ti->win, &ti->toReg);
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	ti->win = NULL;
	ti->tick = 0;
	ti->period = 1000;
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

	if ((win = AG_DEV_TimerInspector()) != NULL) {
		AG_WindowAttach(ti->win, win);
		AG_WindowShow(win);
	}
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;
	AG_Box *hBox;
	AG_Numerical *num;

	ti->win = win;
	AG_LabelNewS(win, 0, "Test for AG_Timer(3) facility");
	AG_LabelNew(win, 0, "timeOps: %s", agTimeOps->name);
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Open Timer Inspector",
	    StartTimerInspector, "%p", ti);

	AG_SeparatorNewHoriz(win);

	hBox = AG_BoxNewHoriz(win, 0);
	{
		AG_ButtonNewFn(hBox, 0, "Test One-Shot", TestOneShot, "%p", ti);
		AG_ButtonNewFn(hBox, 0, "Start Periodic", StartPeriodic, "%p", ti);
		AG_ButtonNewFn(hBox, 0, "Stop Periodic", StopPeriodic, "%p", ti);
	}
	num = AG_NumericalNew(win, 0, "ms", "Period: ");
	AG_BindUint(num, "value", &ti->period);
	return (0);
}

const AG_TestCase timeoutsTest = {
	"timeouts",
	N_("Test AG_Timer(3) facility"),
	"1.5.0",
	0,
	sizeof(MyTestInstance),
	Init,
	NULL,
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

#endif /* AG_TIMERS */

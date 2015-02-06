/*	Public domain	*/
/*
 * This program demonstrates multithread support in Agar.
 */

#include "agartest.h"

#include <agar/config/ag_threads.h>
#ifdef AG_THREADS

#include <math.h>

typedef struct {
	AG_TestInstance _inherit;
	AG_Object workerMgr;
	int      nWorkers;
	AG_Window *winParent;
	struct {			/* For sleeping worker */
		int running;
		AG_Thread th;
		AG_Window *win;
		AG_Label *lbl;
		int min, max;
		int value;
	} sleeping;
	struct {			/* For spinning worker */
		int running;
		AG_Thread th;
		AG_Window *win;
		AG_Label *lbl;
		int min, max;
		int value;
	} spinning;
	int closeTest;
} MyTestInstance;

static void *
CreateWindowMT(void *arg)
{
	MyTestInstance *ti = arg;
	AG_Window *win;

	if ((win = AG_WindowNew(0)) == NULL)
		return (NULL);
	
	AG_WindowSetCaption(win, "Window created in thread");
	AG_LabelNew(win, 0, "This window was created in a separate thread");

	AG_WindowAttach(ti->winParent, win);
	Verbose("Showing window\n");
	AG_WindowShow(win);
	Verbose("Window shown\n");
	return (NULL);
}

static void *
SleepingWorker(void *arg)
{
	MyTestInstance *ti = arg;

	TestMsg(ti, "Sleeping worker created");
	for (;;) {
		if (ti->closeTest) {
			break;
		}
		TestMsg(ti, "Sleeping worker: %d/%d", ti->sleeping.value, ti->sleeping.max);
		if (++ti->sleeping.value == ti->sleeping.max) {
			TestMsg(ti, "Sleeping worker thread exiting");
			AG_ObjectDetach(ti->sleeping.win);
			ti->sleeping.win = NULL;
			AG_ThreadExit(NULL);
		}
		AG_LabelText(ti->sleeping.lbl,
		    "Sleeping worker progress: %d/%d", ti->sleeping.value, ti->sleeping.max);
		AG_Delay(1000);
	}
	ti->sleeping.running = 0;
	return (NULL);
}

static void *
SpinningWorker(void *arg)
{
	MyTestInstance *ti = arg;
	double x = 0.0, y = 0.0;

	TestMsg(ti, "Spinning worker created");
	for (;;) {
		int i;

		if (ti->closeTest) {
			break;
		}
		TestMsg(ti, "Spinning worker: %d/%d", ti->spinning.value, ti->spinning.max);
		if (++ti->spinning.value == ti->spinning.max) {
			TestMsg(ti, "Spinning worker thread exiting. x=%f, y=%f", x, y);
			AG_ObjectDetach(ti->spinning.win);
			ti->spinning.win = NULL;
			AG_ThreadExit(NULL);
		}
		AG_LabelText(ti->spinning.lbl,
		    "Spinning worker progress: %d/%d", ti->spinning.value, ti->spinning.max);
		for (i = 0; i < 100000; i++) {
			y += (double)i;
			x = sin(cos(y)) + tan((double)i);
			if (x != 12.345) { x = 1.0; }
		}
	}
	ti->spinning.running = 0;
	return (NULL);
}

static void
CreateWindowsInThreads(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Thread th;
	int i;
	
	for (i = 0; i < 5; i++)
		AG_ThreadCreate(&th, CreateWindowMT, ti);
}

static void
CreateSleepingWorker(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Window *win;
	AG_ProgressBar *pb;

	if (ti->sleeping.win != NULL ||
	    (win = AG_WindowNew(AG_WINDOW_NOCLOSE)) == NULL) {
		return;
	}
	ti->sleeping.win = win;
	ti->sleeping.min = 0;
	ti->sleeping.max = 10;
	ti->sleeping.value = 0;
	
	AG_WindowSetCaption(win, "Sleeping worker");

	pb = AG_ProgressBarNewHoriz(win, 0);
	AG_BindInt(pb, "value", &ti->sleeping.value);
	AG_BindInt(pb, "min", &ti->sleeping.min);
	AG_BindInt(pb, "max", &ti->sleeping.max);
	ti->sleeping.lbl = AG_LabelNew(win, AG_LABEL_HFILL, "...");
	AG_WindowAttach(ti->winParent, win);
	AG_WindowShow(win);

	AG_ThreadCreate(&ti->sleeping.th, SleepingWorker, ti);
	ti->sleeping.running = 1;
}

static void
CreateSpinningWorker(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Window *win;
	AG_ProgressBar *pb;

	if (ti->spinning.win != NULL ||
	    (win = AG_WindowNew(AG_WINDOW_NOCLOSE)) == NULL) {
		return;
	}
	ti->spinning.win = win;
	ti->spinning.min = 0;
	ti->spinning.max = 100;
	ti->spinning.value = 0;
	
	AG_WindowSetCaption(win, "Spinning worker");

	pb = AG_ProgressBarNewHoriz(win, 0);
	AG_BindInt(pb, "value", &ti->spinning.value);
	AG_BindInt(pb, "min", &ti->spinning.min);
	AG_BindInt(pb, "max", &ti->spinning.max);
	ti->spinning.lbl = AG_LabelNew(win, AG_LABEL_HFILL, "...");
	AG_WindowAttach(ti->winParent, win);
	AG_WindowShow(win);

	AG_ThreadCreate(&ti->spinning.th, SpinningWorker, ti);
	ti->spinning.running = 1;
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	AG_ObjectInitStatic(&ti->workerMgr, NULL);
	ti->nWorkers = 0;
	ti->winParent = NULL;
	ti->sleeping.win = NULL;
	ti->sleeping.running = 0;
	ti->spinning.win = NULL;
	ti->spinning.running = 0;
	ti->closeTest = 0;
	return (0);
}

static void
Destroy(void *obj)
{
	MyTestInstance *ti = obj;

	AG_ObjectDestroy(&ti->workerMgr);
}

static void
CloseTest(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	
	ti->closeTest = 1;

	while (ti->sleeping.running) {
		AG_Delay(50);
	}
	while (ti->spinning.running) {
		AG_Delay(50);
	}
	TestWindowClose(event);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;

	ti->winParent = win;
	AG_ButtonNewFn(win, 0, "Create windows in new threads", CreateWindowsInThreads, "%p", ti);
	AG_ButtonNewFn(win, 0, "Create sleeping worker", CreateSleepingWorker, "%p", ti);
	AG_ButtonNewFn(win, 0, "Create spinning worker", CreateSpinningWorker, "%p", ti);
	AG_SetEvent(win, "window-close", CloseTest, "%p", ti);
	return (0);
}
#endif /* AG_THREADS */

const AG_TestCase threadsTest = {
	"threads",
	N_("Test multithreaded widget creation"),
	"1.4.2",
	0,
#ifdef AG_THREADS
	sizeof(MyTestInstance),
	Init,
	Destroy,
	NULL,		/* test */
	TestGUI,
#else
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	NULL,		/* testGUI */
#endif
	NULL		/* bench */
};

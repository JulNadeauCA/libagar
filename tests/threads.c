/*	Public domain	*/
/*
 * This program demonstrates multithread support in Agar.
 */

#include "agartest.h"

#include <agar/config/ag_threads.h>
#ifdef AG_THREADS

#include <math.h>

typedef struct {
	AG_Object workerMgr;
	int      nWorkers;
	AG_Window *winParent;
	struct {			/* For sleeping worker */
		AG_Window *win;
		AG_Label *lbl;
		int min, max;
		int value;
	} sleeping;
	struct {			/* For spinning worker */
		AG_Window *win;
		AG_Label *lbl;
		int min, max;
		int value;
	} spinning;
} MyTestInstance;

static void *
CreateWindow(void *arg)
{
	MyTestInstance *ti = arg;
	AG_Window *win;

	if ((win = AG_WindowNew(0)) == NULL)
		return (NULL);
	
	AG_WindowSetCaption(win, "Window created in thread");

	AG_LabelNew(win, 0, "This window was created in a separate thread");

	AG_WindowAttach(ti->winParent, win);
	AG_WindowShow(win);
	return (NULL);
}

static void *
SleepingWorker(void *arg)
{
	MyTestInstance *ti = arg;

	TestMsg(ti, "Sleeping worker created");
	for (;;) {
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
}

static void *
SpinningWorker(void *arg)
{
	MyTestInstance *ti = arg;
	double x = 0.0, y = 0.0;

	TestMsg(ti, "Spinning worker created");
	for (;;) {
		int i;

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
	return (NULL);
}

static void
CreateWindowInThread(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Thread th;

	AG_ThreadCreate(&th, CreateWindow, ti);
}

static void
CreateSleepingWorker(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Window *win;
	AG_ProgressBar *pb;
	AG_Thread th;

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

	AG_ThreadCreate(&th, SleepingWorker, ti);
}

static void
CreateSpinningWorker(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Window *win;
	AG_ProgressBar *pb;
	AG_Thread th;

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

	AG_ThreadCreate(&th, SpinningWorker, ti);
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	AG_ObjectInitStatic(&ti->workerMgr, NULL);
	ti->nWorkers = 0;
	ti->winParent = NULL;
	ti->sleeping.win = NULL;
	ti->spinning.win = NULL;
	return (0);
}

static void
Destroy(void *obj)
{
	MyTestInstance *ti = obj;

	AG_ObjectDestroy(&ti->workerMgr);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;

	ti->winParent = win;
	AG_ButtonNewFn(win, 0, "Create window in new thread", CreateWindowInThread, "%p", ti);
	AG_ButtonNewFn(win, 0, "Create sleeping worker", CreateSleepingWorker, "%p", ti);
	AG_ButtonNewFn(win, 0, "Create spinning worker", CreateSpinningWorker, "%p", ti);
	AG_WindowSetCloseAction(win, AG_WINDOW_HIDE);
	return (0);
}
#endif /* AG_THREADS */

const AG_TestCase threadsTest = {
	"threads",
	N_("Test multithreaded widget creation"),
	"1.4.2",
	0,
	sizeof(MyTestInstance),
	Init,
	Destroy,
	NULL,		/* test */
#ifdef AG_THREADS
	TestGUI
#else
	NULL		/* testGUI */
#endif
};

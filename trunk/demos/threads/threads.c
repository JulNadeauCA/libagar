/*	Public domain	*/
/*
 * This program demonstrates multithread support in Agar.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <agar/config/ag_threads.h>

#ifdef AG_THREADS

AG_Object workerMgr;
int nWorkers = 0;

static void *
SleepingWorker(void *arg)
{
	AG_Window *win;
	AG_ProgressBar *pb;
	AG_Label *lbl;
	int min = 0, max = 10, value = 0;

	win = AG_WindowNew(0);
	
	AG_ObjectLock(&workerMgr);
	AG_WindowSetCaption(win, "Sleeping worker (%d)", nWorkers++);
	AG_ObjectUnlock(&workerMgr);

	pb = AG_ProgressBarNewHoriz(win, 0);
	AG_BindInt(pb, "value", &value);
	AG_BindInt(pb, "min", &min);
	AG_BindInt(pb, "max", &max);
	AG_WindowSetPosition(win, AG_WINDOW_ML, 1);

	lbl = AG_LabelNew(win, 0, "Worker thread progress: 100%%");
	
	AG_WindowShow(win);

	for (;;) {
		if (++value == max) {
			AG_ObjectDetach(win);
			AG_ThreadExit(NULL);
		}
		AG_LabelText(lbl, "Worker thread progress: %d%%",
		    AG_ProgressBarPercent(pb));
		AG_Delay(1000);
	}
}

static void *
SpinningWorker(void *arg)
{
	AG_Window *win;
	AG_ProgressBar *pb;
	AG_Label *lbl;
	int min = 0, max = 10, value = 0;
	int i;

	win = AG_WindowNew(0);
	
	AG_ObjectLock(&workerMgr);
	AG_WindowSetCaption(win, "Spinning worker (%d)", nWorkers++);
	AG_ObjectUnlock(&workerMgr);

	pb = AG_ProgressBarNewHoriz(win, 0);
	AG_BindInt(pb, "value", &value);
	AG_BindInt(pb, "min", &min);
	AG_BindInt(pb, "max", &max);
	AG_WindowSetPosition(win, AG_WINDOW_MR, 1);

	lbl = AG_LabelNew(win, 0, "Worker thread progress: 100%%");
	
	AG_WindowShow(win);

	for (;;) {
		if (++value == max) {
			AG_ObjectDetach(win);
			AG_ThreadExit(NULL);
		}
		AG_LabelText(lbl, "Worker thread progress: %d%%",
		    AG_ProgressBarPercent(pb));
		for (i = 0; i < 0xfffffff; i++)
			;;
	}
}

static void
CreateSleepingWorker(AG_Event *event)
{
	AG_Thread th;
	AG_ThreadCreate(&th, SleepingWorker, NULL);
}

static void
CreateSpinningWorker(AG_Event *event)
{
	AG_Thread th;
	AG_ThreadCreate(&th, SpinningWorker, NULL);
}

#endif /* AG_THREADS */

int
main(int argc, char *argv[])
{
	AG_Window *win;
	char *driverSpec = NULL, *optArg;
	int c;

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: threads [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore("agar-threads-demo", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

#ifdef AG_THREADS
	AG_ObjectInitStatic(&workerMgr, NULL);
	win = AG_WindowNew(AG_WINDOW_NOBUTTONS);
	AG_WindowSetCaption(win, "Threads demo");
	AG_WindowSetPosition(win, AG_WINDOW_BR, 0);
	AG_ButtonNewFn(win, 0, "Create sleeping worker",
	    CreateSleepingWorker, NULL);
	AG_ButtonNewFn(win, 0, "Create spinning worker",
	    CreateSpinningWorker, NULL);
	AG_WindowShow(win);
#else
	AG_TextError("Agar was not compiled with threads support!");
#endif

	AG_EventLoop();
	AG_Destroy();
	return (0);
}


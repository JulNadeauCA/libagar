/*	Public domain	*/
/*
 * This program demonstrates multithread support in Agar.
 */

#include <agar/config/have_sdl.h>

#include <agar/core.h>
#include <agar/gui.h>

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
	AG_WidgetBindInt(pb, "value", &value);
	AG_WidgetBindInt(pb, "min", &min);
	AG_WidgetBindInt(pb, "max", &max);
	AG_WindowSetPosition(win, AG_WINDOW_ML, 1);

	lbl = AG_LabelNew(win, 0, "Worker thread progress: 100%%");
	
	AG_WindowShow(win);

	for (;;) {
		if (++value == max) {
			AG_ViewDetach(win);
			AG_ThreadExit(NULL);
		}
		AG_LabelPrintf(lbl, "Worker thread progress: %d%%",
		    AG_ProgressBarPercent(pb));
#ifdef HAVE_SDL
		SDL_Delay(1000);
#else
		sleep(1);
#endif
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
	AG_WidgetBindInt(pb, "value", &value);
	AG_WidgetBindInt(pb, "min", &min);
	AG_WidgetBindInt(pb, "max", &max);
	AG_WindowSetPosition(win, AG_WINDOW_MR, 1);

	lbl = AG_LabelNew(win, 0, "Worker thread progress: 100%%");
	
	AG_WindowShow(win);

	for (;;) {
		if (++value == max) {
			AG_ViewDetach(win);
			AG_ThreadExit(NULL);
		}
		AG_LabelPrintf(lbl, "Worker thread progress: %d%%",
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
	
	if (AG_InitCore("agar-threads-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

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


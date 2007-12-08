/*
 * This shows the use of a custom event loop with double buffering.
 * Instead of the usual algorithm which builds a list of video regions
 * to update, we use only SDL_Flip().
 */

#include <agar/core.h>
#include <agar/gui.h>

static void
EventLoop(void)
{
	extern struct ag_objectq agTimeoutObjQ;
	SDL_Event ev;
	AG_Window *win;
	Uint32 Tr1 = SDL_GetTicks(), Tr2 = 0;

	for (;;) {
		Tr2 = SDL_GetTicks();
		if (Tr2-Tr1 >= agView->rNom) {		/* Time to redraw? */
			AG_MutexLock(&agView->lock);

			/* Render the GUI windows. */
			AG_TAILQ_FOREACH(win, &agView->windows, windows) {
				AG_MutexLock(&win->lock);
				if (!win->visible) {
					AG_MutexUnlock(&win->lock);
					continue;
				}
				AG_WidgetDraw(win);
				AG_MutexUnlock(&win->lock);
			}

			/* Flip the buffers. */
			SDL_Flip(agView->v);

			AG_MutexUnlock(&agView->lock);

			/* Recalibrate the effective refresh rate. */
			Tr1 = SDL_GetTicks();
			agView->rCur = agView->rNom - (Tr1-Tr2);
			if (agView->rCur < 1) {
				agView->rCur = 1;
			}
		} else if (SDL_PollEvent(&ev) != 0) {
			AG_ProcessEvent(&ev);
		} else if (AG_TAILQ_FIRST(&agTimeoutObjQ) != NULL) {
			AG_ProcessTimeout(Tr2);
		} else if (agView->rCur > agIdleThresh) {
			SDL_Delay(agView->rCur - agIdleThresh);
		}
	}
}

int
main(int argc, char *argv[])
{
	AG_Window *win;
	AG_Table *tbl;

	if (AG_InitCore("doublebuf", 0) == -1 ||
	    AG_InitVideo(640, 480, 32, AG_VIDEO_HWSURFACE|AG_VIDEO_DOUBLEBUF)
	     == -1) {
		return (1);
	}
	win = AG_WindowNew(0);
	AG_LabelNewStatic(win, 0, "Hello, world!");
	AG_WindowShow(win);
	EventLoop();
	return (0);
}

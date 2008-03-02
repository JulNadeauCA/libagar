/* Public domain */
/*
 * This is an example custom event loop function that uses double buffering.
 * Instead of the default AG_EventLoop() which builds a list of video regions
 * to update, this one uses SDL_Flip().
 */

#include <agar/core.h>
#include <agar/gui.h>

#include "doublebuf.h"

void
MyEventLoop_DoubleBuf(void)
{
	extern struct ag_objectq agTimeoutObjQ;
	SDL_Event ev;
	AG_Window *win;
	Uint32 Tr1 = SDL_GetTicks(), Tr2 = 0;

	for (;;) {
		Tr2 = SDL_GetTicks();
		if (Tr2-Tr1 >= agView->rNom) {		/* Time to redraw? */
			AG_LockVFS(agView);

			/* Render the GUI windows. */
			AG_TAILQ_FOREACH(win, &agView->windows, windows) {
				AG_ObjectLock(win);
				if (win->visible) {
					AG_WidgetDraw(win);
				}
				AG_ObjectUnlock(win);
			}

			/* Flip the buffers. */
			SDL_Flip(agView->v);

			AG_UnlockVFS(agView);

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

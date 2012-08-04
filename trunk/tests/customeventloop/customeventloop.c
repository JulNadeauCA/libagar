/*	Public domain	*/
/*
 * This application demonstrates the use of a custom event loop.
 */

#include "agartest.h"

int pressedKey = 0;			/* Last pressed key */
int xClick = 0, yClick = 0;		/* Last clicked x,y */
int curFPS = 0;				/* Measured frame rate */
const int nominalFPS = 1000/30;		/* Nominal frame rate */

/*
 * Our custom event loop routine. 
 */
static void
MyEventLoop(void)
{
	AG_Driver *drv;
	AG_Window *win;
	Uint32 t1, t2;
	AG_DriverEvent dev;

	t1 = AG_GetTicks();
	for (;;) {
		t2 = AG_GetTicks();

		if (t2-t1 >= nominalFPS) {
			/*
			 * Case 1: Update the video display.
			 */
			AG_LockVFS(&agDrivers);

			/* Render the Agar windows */
			if (agDriverSw) {
				/*
				 * We are using a single-window driver
				 * (e.g., sdlfb). If one of the windows is
				 * marked dirty, all windows must be redrawn.
				 */
				AG_FOREACH_WINDOW(win, agDriverSw) {
					if (win->dirty)
						break;
				}
				if (win != NULL) {
					AG_BeginRendering(agDriverSw);
					AG_FOREACH_WINDOW(win, agDriverSw) {
						if (!win->visible) {
							continue;
						}
						AG_ObjectLock(win);
						AG_WindowDraw(win);
						AG_ObjectUnlock(win);
					}
					AG_EndRendering(agDriverSw);
				}
			} else {
				/*
				 * We are using a multiple-window driver
				 * (e.g., glx). Windows marked dirty are
				 * redrawn.
				 */
				AGOBJECT_FOREACH_CHILD(drv, &agDrivers,
				    ag_driver) {
					if (!AGDRIVER_MULTIPLE(drv)) {
						continue;
					}
					win = AGDRIVER_MW(drv)->win;
					if (!win->visible || !win->dirty) {
						continue;
					}
					AG_BeginRendering(drv);
					AG_ObjectLock(win);
					AG_WindowDraw(win);
					AG_ObjectUnlock(win);
					AG_EndRendering(drv);
				}
			}
			AG_UnlockVFS(&agDrivers);

			t1 = AG_GetTicks();
			curFPS = nominalFPS - (t1-t2);
			if (curFPS < 1) { curFPS = 1; }

		} else if (AG_PendingEvents(NULL) > 0) {
			/*
			 * Case 2: There are events waiting to be processed.
			 */
			do {
				/* Retrieve the next queued event. */
				if (AG_GetNextEvent(NULL, &dev) == 1) {
					switch (dev.type) {
					case AG_DRIVER_MOUSE_BUTTON_DOWN:
						xClick = dev.data.button.x;
						yClick = dev.data.button.y;
						printf("Click at %d,%d!\n",
						    dev.data.button.x,
						    dev.data.button.y);
						break;
					case AG_DRIVER_KEY_DOWN:
						pressedKey = (int)dev.data.key.ks;
						printf("Key down: %d (0x%x)\n",
						    (int)dev.data.key.ks,
						    (Uint)dev.data.key.ucs);
						break;
					default:
						break;
					}

					/* Forward the event to Agar. */
					if (AG_ProcessEvent(NULL, &dev) == -1)
						return;
				}
			} while (AG_PendingEvents(NULL) > 0);

		} else if (AG_TIMEOUTS_QUEUED()) {
			/*
			 * Case 3: There are AG_Timeout(3) callbacks to run.
			 */
			AG_ProcessTimeouts(t2);
		} else {
			/*
			 * Case 4: Nothing to do, idle.
			 */
			AG_Delay(1);
		}
	}
}

int
TestGUI(void *obj)
{
	AG_Window *win;

	if ((win = TestWindowNew(obj, 0)) == NULL) {
		return (-1);
	}
	AG_LabelNewPolled(win, AG_LABEL_EXPAND,
	    "Testing custom event loop\n"
	    "Frame rate = %d\n"
	    "Pressed key = %d\n"
	    "Clicked at %d,%d\n",
	    &curFPS, &pressedKey, &xClick, &yClick);
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Quit", AGWINDETACH(win));
	AG_WindowSetGeometry(win, -1, -1, 300, 128);
	AG_WindowShow(win);

	MyEventLoop();
	return (0);
}

const AG_TestCase customEventLoopTest = {
	"customEventLoop",
	N_("Test Agar running with a user-provided event loop"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI
};

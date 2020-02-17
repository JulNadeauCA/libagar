/*	Public domain	*/
/*
 * This application demonstrates the use of a custom event loop.
 */

#include <agar/core.h>
#include <agar/gui.h>

int pressedKey = 0;			/* Last pressed key */
int xClick = 0, yClick = 0;		/* Last clicked x,y */
int fpsNominal=30, fpsCur=0;		/* Nominal refresh rate */

/*
 * Our custom event loop routine. 
 */
static void
MyEventLoop(void)
{
	AG_Window *win;
	Uint32 t1, t2;

	t1 = AG_GetTicks();
	printf("[%d] Entering event loop\n", t1);
	for (;;) {
		t2 = AG_GetTicks();
		if (t2 - t1 >= fpsNominal) {
			AG_WindowDrawQueued();

			t1 = AG_GetTicks();
			fpsCur = fpsNominal - (t1-t2);
			if (fpsCur < 1) { fpsCur = 1; }
		} else if (AG_PendingEvents(NULL)) {
			AG_DriverEvent dev;            /* A low-level event */

			do {
				printf("Event %d at t2=%d\n", dev.type, t2);
				/* Retrieve the next queued event. */
				if (AG_GetNextEvent(NULL, &dev) == 1) {
					switch (dev.type) {
					case AG_DRIVER_MOUSE_MOTION:
						break;
					case AG_DRIVER_MOUSE_BUTTON_DOWN:
						xClick = dev.data.button.x;
						yClick = dev.data.button.y;
						printf("[%d] Click at %d,%d!\n", t2,
						    dev.data.button.x,
						    dev.data.button.y);
						break;
					case AG_DRIVER_KEY_DOWN:
						if (dev.data.key.ks == AG_KEY_ESCAPE) {
							goto out;
						}
						pressedKey = (int)dev.data.key.ks;
						printf("[%d] Key down: %d (0x%x)\n", t2,
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
		} else {
			AG_LockTiming();
			if (!AG_TAILQ_EMPTY(&agTimerObjQ)) {
				printf("[%d] Timer expiration\n", t2);
				AG_ProcessTimeouts(t2);   /* Execute timer callbacks */
			}
			AG_UnlockTiming();
			AG_Delay(1);                                 /* Idle */
		}
		AG_WindowProcessQueued();
	}
out:
	printf("[%d] Exiting event loop\n", AG_GetTicks());
}

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
			printf("Usage: agarevloop [-d agar-driver-spec]\n");
			return (1);
		}
	}

	if (AG_InitCore(NULL, AG_SOFT_TIMERS) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindStdGlobalKeys();

	win = AG_WindowNew(AG_WINDOW_MAIN);
	AG_WindowSetCaption(win, "Agar custom event loop demo");
	AG_LabelNewPolled(win, AG_LABEL_EXPAND,
	    "Testing custom event loop\n"
	    "Press ESC to exit\n"
	    "fpsNominal = " AGSI_BOLD "%d" AGSI_RST " , fpsCur = " AGSI_BOLD "%d" AGSI_RST "\n"
	    "Pressed key = %d\n"
	    "Clicked at %d,%d\n",
	    &fpsNominal, &fpsCur, &pressedKey, &xClick, &yClick);
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Quit", AGWINDETACH(win));
	AG_WindowSetGeometry(win, -1, -1, 300, 128);
	AG_WindowShow(win);

	MyEventLoop();
	AG_Destroy();
	return (0);
}

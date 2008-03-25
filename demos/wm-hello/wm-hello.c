/*	Public domain	*/
/*
 * This is a minimmal application which uses AgarWM.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <sys/types.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int
main(int argc, char *argv[])
{
	AG_Window *win;

	if (AG_InitCore("agarwm-hello", 0) == -1 ||
	    AG_InitGUI(0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	
	if ((win = AG_WindowNewRemote(0, "localhost:4167")) == NULL) {
		fprintf(stderr, "Could not create remote window: %s\n",
		    AG_GetError());
		return (1);
	}
	AG_WindowSetCaption(win, "AgarWM application (pid %d)", getpid());
	AG_LabelNew(win, 0, "Hello, world!");
	AG_ButtonNew(win, AG_BUTTON_HFILL, "OK");

	AG_EventLoop();
	AG_Destroy();
	return (0);
}

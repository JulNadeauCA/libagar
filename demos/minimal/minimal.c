/*	Public domain	*/
/*
 * This is a minimal program using the Agar-GUI library.
 */

#include <agar/core.h>
#include <agar/gui.h>

int
main(int argc, char *argv[])
{
	/* Initialize Agar-GUI. */
	if (AG_InitCore("agar-minimal-demo", 0) == -1 ||
	    AG_InitVideo(320, 240, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	/* ... */
	AG_TextMsg(AG_MSG_INFO, "Minimal!");
	
	/* Use the standard Agar event loop. */
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

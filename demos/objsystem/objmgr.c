/*	Public domain	*/
/*
 * This application demonstrates the basic functionality of the Agar
 * object system. It uses the "Object Browser" which is part of the
 * Agar-DEV library.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/dev.h>

#include <string.h>

#include "animal.h"
#include "mammal.h"

int
main(int argc, char *argv[])
{
	if (AG_InitCore("objmgr-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	/* Register the Agar object classes which we implement. */
	AG_RegisterClass(&AnimalClass);
	AG_RegisterClass(&MammalClass);

	/* Load the entire VFS */
	AG_ObjectLoad(agWorld);

	/* Initialize the DEV library and display the object browser. */
	DEV_InitSubsystem(0);
	AG_WindowShow(DEV_Browser());
	
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

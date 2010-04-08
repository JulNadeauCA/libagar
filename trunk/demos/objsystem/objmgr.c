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

/* This will be the root of our virtual filesystem. */
AG_Object vfsRoot;

int
main(int argc, char *argv[])
{
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
			printf("Usage: objsystem [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore("agar-objsystem-demo", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	/* Register the Agar object classes which we implement. */
	AG_RegisterClass(&AnimalClass);
	AG_RegisterClass(&MammalClass);

	/*
	 * Initialize our virtual filesystem root and load its contents if it
	 * was previously saved to disk. This structure was not malloc'ed so
	 * we must use AG_ObjectInitStatic(). We use NULL as the class so the
	 * generic AG_Object(3) class will be used.
	 */
	AG_ObjectInitStatic(&vfsRoot, NULL);
	AG_ObjectSetName(&vfsRoot, "My VFS");
	(void)AG_ObjectLoad(&vfsRoot);

	/*
	 * Initialize the DEV library. DEV_Browser() is a simple object
	 * browser that allows the user to browse and manipulate the
	 * contents of the VFS.
	 */
	DEV_InitSubsystem(0);
	AG_WindowShow(DEV_Browser(&vfsRoot));
	
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

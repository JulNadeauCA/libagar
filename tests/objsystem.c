/*	Public domain	*/
/*
 * This application demonstrates the basic functionality of the Agar
 * object system. It uses the "Object Browser" which is part of the
 * Agar-DEV library.
 */

#include "agartest.h"

#include <agar/dev.h>

#include "objsystem_animal.h"
#include "objsystem_mammal.h"

typedef struct {
	AG_TestInstance _inherit;
	AG_Object vfsRoot;			/* Our test VFS */
} MyTestInstance;

static int inited = 0;

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	if (inited++ == 0) {
		DEV_InitSubsystem(0);

		/* Register the Agar object classes which we implement. */
		AG_RegisterClass(&AnimalClass);
		AG_RegisterClass(&MammalClass);
	}
	
	/*
	 * Initialize our virtual filesystem root. Since vfsRoot was not
	 * malloc'ed, we must use the AG_ObjectInitStatic() variant.
	 */
	AG_ObjectInitStatic(&ti->vfsRoot, NULL);
	AG_ObjectSetName(&ti->vfsRoot, "My VFS");
	return (0);
}

static void
Destroy(void *obj)
{
	MyTestInstance *ti = obj;

	/* Destroy our test VFS. */
	AG_ObjectDestroy(&ti->vfsRoot);

	if (--inited == 0) {
		/* Unregister our classes for a complete cleanup. */
		AG_UnregisterClass(&AnimalClass);
		AG_UnregisterClass(&MammalClass);
	}
}

static void
StartBrowser(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Window *winParent = AG_PTR(2), *win;

	if ((win = DEV_Browser(&ti->vfsRoot)) != NULL)
		AG_WindowAttach(winParent, win);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;

	if (AG_ObjectLoad(&ti->vfsRoot) == 0) {
		AG_LabelNewS(win, 0, "Test VFS loaded");
	} else {
		AG_LabelNewS(win, 0, "New test VFS");
	}
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Start VFS browser",
	    StartBrowser, "%p,%p", ti, win);
	return (0);
}

const AG_TestCase objSystemTest = {
	"objSystem",
	N_("Test basic AG_Object(3) VFS functions"),
	"1.4.2",
	0,
	sizeof(MyTestInstance),
	Init,
	Destroy,
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

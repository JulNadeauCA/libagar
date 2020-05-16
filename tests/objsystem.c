/*	Public domain	*/
/*
 * This application demonstrates the basic functionality of the Agar
 * object system. It uses the "Object Browser" from gui/dev_browser.c.
 */

#include "agartest.h"
#ifdef AG_TIMERS

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
		/* Register the Agar object classes which we implement. */
		AG_RegisterClass(&AnimalClass);
		AG_RegisterClass(&MammalClass);
	}
	
	/*
	 * Initialize our virtual filesystem root. Since vfsRoot was not
	 * malloc'ed, we must use the AG_ObjectInitStatic() variant.
	 */
	AG_ObjectInit(&ti->vfsRoot, NULL);
	ti->vfsRoot.flags |= AG_OBJECT_STATIC;
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

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;
	AG_Window *winBrowser;
	AG_Box *box = AG_BoxNewVert(win, AG_BOX_EXPAND);
	AG_Object *chld;

	if (AG_ObjectLoad(&ti->vfsRoot) == 0) {
		AG_LabelNewS(box, 0, "Test VFS loaded OK.");
		AGOBJECT_FOREACH_CHILD(chld, &ti->vfsRoot, ag_object) {
			AG_LabelNew(box, 0, "Loaded %s (a %s)\n", chld->name,
			            AGOBJECT_CLASS(chld)->name);
		}
	} else {
		AG_LabelNewS(box, 0, "Test VFS could not be loaded. "
		                     "Creating new one.");
	}
	if ((winBrowser = AG_DEV_Browser(&ti->vfsRoot)) != NULL) {
		AG_WindowAttach(win, winBrowser);
	}
	return (0);
}

const AG_TestCase objsystemTest = {
	"objsystem",
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
#endif /* AG_TIMERS */

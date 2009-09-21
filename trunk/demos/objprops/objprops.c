/*	Public domain	*/
/*
 * This application demonstrates the use of generic properties with
 * Agar objects.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/dev.h>

#include <string.h>

/* This will be the root of our virtual filesystem. */
AG_Object vfsRoot;

/* Sample function binding returning Uint32. */
static Uint32
MyUint32Fn(AG_Event *event)
{
	return (SDL_GetTicks());
}

int
main(int argc, char *argv[])
{
	AG_Object *obj;

	if (AG_InitCore("agar-objprops-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_Quit);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	/*
	 * Initialize our virtual filesystem root and load its contents if it
	 * was previously saved to disk. This structure was not malloc'ed so
	 * we must use AG_ObjectInitStatic(). We use NULL as the class so the
	 * generic AG_Object(3) class will be used.
	 */
	AG_ObjectInitStatic(&vfsRoot, NULL);
	AG_ObjectSetName(&vfsRoot, "My VFS");
	(void)AG_ObjectLoad(&vfsRoot);

	/* Initialize Agar-DEV and show the object browser. */
	DEV_InitSubsystem(0);
	AG_WindowShow(DEV_Browser(&vfsRoot));

	/* Create a generic object instance. */
	obj = AG_ObjectNew(&vfsRoot, "Foo", &agObjectClass);

	/* Create one property of every type. */
	AG_SetBool(obj, "my-boolean1", 0);
	AG_SetBool(obj, "my-boolean2", 1);
	AG_SetUint(obj, "my-uint", 32000);
	AG_SetInt(obj, "my-int", 16000);
	AG_SetUint8(obj, "my-uint8", 120);
	AG_SetSint8(obj, "my-sint8", -120);
	AG_SetUint16(obj, "my-uint16", 64000);
	AG_SetSint16(obj, "my-sint16", -32000);
	AG_SetUint32(obj, "my-uint32", 0xffffffe);
	AG_SetSint32(obj, "my-sint32", -1234);
	AG_SetFloat(obj, "my-float", 1.2345f);
	AG_SetDouble(obj, "my-double", 3.14159265358979323846);
	AG_SetString(obj, "my-string", "|(x,y)|\xc2\xb2 <= (x,x)*(y,y)");
	AG_BindUint32Fn(obj, "my-uint32-fn", MyUint32Fn, NULL);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}


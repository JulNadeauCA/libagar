/*	Public domain	*/
/*
 * This application demonstrates the use of generic properties with
 * Agar objects.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/dev.h>

#include <string.h>
#include <unistd.h>

static Uint32
ReadTicks(void *p, AG_Prop *prop)
{
	/* Return the SDL ticks. */
	return (SDL_GetTicks());
}

static Uint32
WriteTicks(void *p, AG_Prop *prop, Uint32 nv)
{
	/* Return the current value unchanged. */
	printf("%s: prop `%s' is read-only!\n", AGOBJECT(p)->name, prop->key);
	return (prop->data.u32);
}

int
main(int argc, char *argv[])
{
	AG_Object *obj;
	int c, i;
	char *s;

	if (AG_InitCore("objprops-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}

	while ((c = getopt(argc, argv, "?vgG")) != -1) {
		extern char *optarg;

		switch (c) {
		case 'v':
			exit(0);
#ifdef HAVE_OPENGL
		case 'g':
			AG_SetBool(agConfig, "view.opengl", 1);
			break;
		case 'G':
			AG_SetBool(agConfig, "view.opengl", 0);
			break;
#endif
		case '?':
		default:
			printf("%s [-vgG]\n", agProgName);
			exit(0);
		}
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_SetRefreshRate(-1);
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	/* Reload the previous state, initialize the object manager. */
	AG_ObjectLoad(agWorld);

	/* Initialize Agar-DEV and show the object browser. */
	DEV_InitSubsystem(0);
	AG_WindowShow(DEV_Browser());

	/* Create a generic object instance. */
	obj = AG_ObjectNew(agWorld, "Foo", &agObjectClass);

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
#ifdef AG_HAVE_64BIT
	AG_SetUint64(obj, "my-uint64", 0xfffffffffffffffe);
	AG_SetSint64(obj, "my-sint64", -4567);
#endif
	AG_SetFloat(obj, "my-float", 1.23456);
	AG_SetDouble(obj, "my-double", 3.14159265358979323846);
#ifdef HAVE_LONG_DOUBLE
	AG_SetLongDouble(obj, "my-long-double", 234e-12);
#endif
	AG_SetString(obj, "my-string", "|(x,y)|\xc2\xb2 <= (x,x)*(y,y)");
	
	/* Create a property with read and write operations */
	{
		AG_Prop *pTicks;

		pTicks = AG_SetUint32(obj, "sdl-ticks", 0);
		AG_SetUint32RdFn(pTicks, ReadTicks);
		AG_SetUint32WrFn(pTicks, WriteTicks);
	}
	
	AG_EventLoop();
	AG_Destroy();
	return (0);
}


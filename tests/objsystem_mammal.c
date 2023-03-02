/*	Public domain	*/

/*
 * Implementation of example class Mammal to demonstrate inheritance:
 * AG_Object -> Animal -> Mammal.
 */

#include "agartest.h"
#include "objsystem_animal.h"
#include "objsystem_mammal.h"

static void
Init(void *obj)
{
	MY_Mammal *mammal = obj;

	mammal->hairColor.h = 1.0;
	mammal->hairColor.s = 1.0;
	mammal->hairColor.v = 0.0;
}

static int
Load(void *obj, AG_DataSource *ds, const AG_Version *ver)
{
	MY_Mammal *mammal = obj;

	mammal->hairColor.h = AG_ReadFloat(ds);
	mammal->hairColor.s = AG_ReadFloat(ds);
	mammal->hairColor.v = AG_ReadFloat(ds);
	return (0);
}

static int
Save(void *obj, AG_DataSource *ds)
{
	MY_Mammal *mammal = obj;

	AG_WriteFloat(ds, mammal->hairColor.h);
	AG_WriteFloat(ds, mammal->hairColor.s);
	AG_WriteFloat(ds, mammal->hairColor.v);
	return (0);
}

static void *
Edit(void *obj)
{
	MY_Mammal *mammal = obj;
	AG_Window *win, *winSuper;
	AG_HSVPal *pal;
	AG_ObjectClass *super;
	AG_Label *lbl;
	char *s;

	if ((win = AG_WindowNew(0)) == NULL) {
		AG_FatalError(NULL);
	}
	AG_WindowSetCaption(win, "Mammal: %s", AGOBJECT(mammal)->name);
	lbl = AG_LabelNewS(win, AG_LABEL_HFILL, AGOBJECT(mammal)->name);
	AG_SetFontSize(lbl, "200%");

	if ((s = AG_ObjectGetName(mammal)) != NULL) {
		AG_LabelNew(win, 0, "Name: %s", s);
		AG_Free(s);
	}
	s = AG_ObjectGetClassName(mammal, 1);
	AG_LabelNew(win, 0, "Instance of %s", s);
	AG_Free(s);

	super = AG_ObjectSuperclass(mammal);
	if (super->edit != NULL) {
		winSuper = super->edit(mammal);
		AG_WindowSetPosition(winSuper, AG_WINDOW_UPPER_CENTER, 0);
		AG_WindowShow(winSuper);
	}

	AG_LabelNew(win, 0, "Hair color:");

	pal = AG_HSVPalNew(win, AG_HSVPAL_EXPAND | AG_HSVPAL_SHOW_HSV);
	AG_BindFloat(pal, "hue",        &mammal->hairColor.h);
	AG_BindFloat(pal, "saturation", &mammal->hairColor.s);
	AG_BindFloat(pal, "value",      &mammal->hairColor.v);

	return (win);
}

AG_ObjectClass myMammalClass = {
	"MY_Animal:MY_Mammal",
	sizeof(MY_Mammal),
	{ 1,0, 0, 0 },
	Init,
	NULL,			/* reset */
	NULL,			/* destroy */
	Load,
	Save,
	Edit
};

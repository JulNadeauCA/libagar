/*	Public domain	*/

/*
 * Implementation of the example "Mammal" class which inherits from the
 * "Animal" class. This demonstrates inheritance under the Agar Object system.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include "animal.h"
#include "mammal.h"

/* Initialization routine. */
static void
Init(void *obj)
{
	Mammal *mammal = obj;

	mammal->hairColor.h = 1.0;
	mammal->hairColor.s = 1.0;
	mammal->hairColor.v = 0.0;
}

/* Load routine. */
static int
Load(void *obj, AG_DataSource *ds, const AG_Version *ver)
{
	Mammal *mammal = obj;

	mammal->hairColor.h = AG_ReadFloat(ds);
	mammal->hairColor.s = AG_ReadFloat(ds);
	mammal->hairColor.v = AG_ReadFloat(ds);
	return (0);
}

/* Save routine. */
static int
Save(void *obj, AG_DataSource *ds)
{
	Mammal *mammal = obj;

	AG_WriteFloat(ds, mammal->hairColor.h);
	AG_WriteFloat(ds, mammal->hairColor.s);
	AG_WriteFloat(ds, mammal->hairColor.v);
	return (0);
}

/* Edition routine. */
static void *
Edit(void *obj)
{
	Mammal *mammal = obj;
	AG_Window *win;
	AG_HSVPal *pal;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Mammal: %s", AGOBJECT(mammal)->name);

	AG_LabelNewStatic(win, 0, "Hair color:");
	pal = AG_HSVPalNew(win, AG_HSVPAL_EXPAND);
	AG_WidgetBindFloat(pal, "hue", &mammal->hairColor.h);
	AG_WidgetBindFloat(pal, "saturation", &mammal->hairColor.s);
	AG_WidgetBindFloat(pal, "value", &mammal->hairColor.v);
	return (win);
}

/* Class description */
const AG_ObjectOps MammalOps = {
	"Animal:Mammal",	/* Our class. We inherit from "Animal". */
	sizeof(Mammal),		/* Size of structure */
	{ 0,0 },		/* Dataset version */
	Init,
	NULL,			/* reinit */
	NULL,			/* destroy */
	Load,
	Save,
	Edit
};

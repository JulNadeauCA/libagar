/*	Public domain	*/

/*
 * Implementation of the example "Mammal" class which inherits from the
 * "Animal" class. This demonstrates inheritance under the Agar Object system.
 */

#include "agartest.h"
#include "objsystem_animal.h"
#include "objsystem_mammal.h"

/*
 * Initialization routine. Note that the object system will automatically
 * invoke the initialization routines of all parent classes first.
 */
static void
Init(void *obj)
{
	Mammal *mammal = obj;

	mammal->hairColor.h = 1.0;
	mammal->hairColor.s = 1.0;
	mammal->hairColor.v = 0.0;
}

/*
 * Load routine. This operation restores the state of the object from
 * a data source.
 *
 * The object system will automatically invoke the load routines of
 * the parent beforehand.
 */
static int
Load(void *obj, AG_DataSource *ds, const AG_Version *ver)
{
	Mammal *mammal = obj;

	mammal->hairColor.h = AG_ReadFloat(ds);
	mammal->hairColor.s = AG_ReadFloat(ds);
	mammal->hairColor.v = AG_ReadFloat(ds);
	return (0);
}

/*
 * Save routine. This operation saves the state of the object to a
 * data source.
 *
 * The object system will automatically invoke the save routines of
 * the parent beforehand.
 */
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
	AG_Window *win, *winSuper;
	AG_HSVPal *pal;
	AG_ObjectClass *super;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Mammal: %s", AGOBJECT(mammal)->name);

	/* Invoke the "edit" operation of the superclass. */
	super = AG_ObjectSuperclass(mammal);
	if (super->edit != NULL) {
		winSuper = super->edit(mammal);
		AG_WindowSetPosition(winSuper, AG_WINDOW_UPPER_CENTER, 0);
		AG_WindowShow(winSuper);
	}

	/* Allow user to edit paramters specific to this class. */
	AG_LabelNew(win, 0, "Hair color:");
	pal = AG_HSVPalNew(win, AG_HSVPAL_EXPAND);
	AG_BindFloat(pal, "hue", &mammal->hairColor.h);
	AG_BindFloat(pal, "saturation", &mammal->hairColor.s);
	AG_BindFloat(pal, "value", &mammal->hairColor.v);

	return (win);
}

/* Class description */
AG_ObjectClass MammalClass = {
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

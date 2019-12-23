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
 * Load routine. This restores the state of the object from serialized data.
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
 * Save routine. This saves the state of the object to a DataSource.
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

/*
 * Edition routine. We use Agar-GUI so we return an AG_Window.
 *
 * We could have also returned another container widget (such as a
 * parent-less AG_Box).
 */
static void *
Edit(void *obj)
{
	Mammal *mammal = obj;
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
	AG_SetStyle(lbl, "font-size", "200%");

	/* Get the name of this object instance. */
	if ((s = AG_ObjectGetName(mammal)) != NULL) {
		AG_LabelNew(win, 0, "Name: %s", s);
		AG_Free(s);
	}

	/*
	 * Get the name of the class which mammal is an instance of.
	 *
	 * Also accessible via AGOBJECT_CLASS(mammal)->hier (full hierarchy)
	 * or AGOBJECT_CLASS(mammal)->name (name of the last subclass).
	 */
	s = AG_ObjectGetClassName(mammal, 1);
	AG_LabelNew(win, 0, "Instance of %s", s);
	AG_Free(s);

	/* Invoke the "edit" operation of the superclass. */
	super = AG_ObjectSuperclass(mammal);
	if (super->edit != NULL) {
		winSuper = super->edit(mammal);
		AG_WindowSetPosition(winSuper, AG_WINDOW_UPPER_CENTER, 0);
		AG_WindowShow(winSuper);
	}

	/* Allow user to edit paramters specific to this class. */
	AG_LabelNew(win, 0, "Hair color:");
	pal = AG_HSVPalNew(win, AG_HSVPAL_EXPAND | AG_HSVPAL_SHOW_HSV);
	AG_BindFloat(pal, "hue", &mammal->hairColor.h);
	AG_BindFloat(pal, "saturation", &mammal->hairColor.s);
	AG_BindFloat(pal, "value", &mammal->hairColor.v);

	return (win);
}

/*
 * The AG_ObjectClass structure describes an Agar object class.
 *
 * Although we are using the base AG_ObjectClass in this case, deriving
 * this structure provides a good way for adding new methods and other
 * class-specific data members that can be shared between all instances
 * of a class.
 */
AG_ObjectClass MammalClass = {
	"Animal:Mammal",	/* Inheritance hierarchy (implies AG_Object) */
	sizeof(Mammal),		/* Size of instance structures */
	{ 0,0 },		/* Version (Major, Minor) */
	Init,
	NULL,			/* reset */
	NULL,			/* destroy */
	Load,
	Save,
	Edit
};

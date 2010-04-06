/*	Public domain	*/

/*
 * Implementation of the example "Animal" class demonstrating use of
 * inheritance under the Agar Object system.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include "animal.h"

/*
 * Constructor function. This is optional and only a convenience for
 * developers who want to use our "Animal" API directly.
 */
Animal *
AnimalNew(void *parent)
{
	Animal *animal;

	animal = AG_Malloc(sizeof(Animal));
	AG_ObjectInit(animal, &AnimalClass);	/* Will invoke Init() */
	AG_ObjectAttach(parent, animal);
	return (animal);
}

/*
 * Example of an event handler function. Event handlers use a stack of
 * arguments. Both AG_SetEvent() and AG_PostEvent() can push arguments
 * onto this stack.
 */
static void
Die(AG_Event *event)
{
	Animal *animal = AG_SELF();
	AG_Object *killer = AG_SENDER();

	printf("%s: killed by %s!\n", AGOBJECT(animal)->name, killer->name);
}

/* Example of a timer callback. */
static Uint32
Tick(void *obj, Uint32 ival, void *arg)
{
	Animal *animal = obj;

	animal->age += 1.0;
	animal->cellCount *= 2;
	return (ival);
}

/* Handle the "attached" event by starting our timer. */
static void
Attached(AG_Event *event)
{
	Animal *animal = AG_SELF();

	AG_ScheduleTimeout(animal, &animal->time, 1000);
}

/*
 * Initialization routine. Note that the object system will automatically
 * invoke the initialization routines of all parent classes first.
 */
static void
Init(void *obj)
{
	Animal *animal = obj;

	animal->age = 0.0;
	animal->cellCount = 1;

	/* Event handler functions and timers are usually configured here. */
	AG_SetEvent(animal, "die", Die, NULL);
	AG_SetEvent(animal, "attached", Attached, NULL);
	AG_SetTimeout(&animal->time, Tick, NULL, 0);
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
	Animal *animal = obj;

	animal->age = AG_ReadFloat(ds);
	animal->cellCount = (int)AG_ReadUint32(ds);
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
	Animal *animal = obj;

	AG_WriteFloat(ds, animal->age);
	AG_WriteUint32(ds, (int)animal->cellCount);
	return (0);
}

/*
 * Edit routine. This is a generic operation that returns a generic pointer,
 * and is not dependent on any particular user interface.
 *
 * This program uses Agar-GUI, so we will return an Agar window.
 */
static void *
Edit(void *obj)
{
	Animal *animal = obj;
	AG_Window *win;
	AG_Numerical *num;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Animal: %s", AGOBJECT(animal)->name);

	num = AG_NumericalNewFlt(win, AG_NUMERICAL_HFILL, "sec",
	    "Age: ", &animal->age);
	num = AG_NumericalNewInt(win, AG_NUMERICAL_HFILL, NULL,
	    "Cell count: ", &animal->cellCount);

	return (win);
}

/*
 * This structure describes our class. Any of the function members may be
 * NULL.
 */
AG_ObjectClass AnimalClass = {
	"Animal",		/* Name of class */
	sizeof(Animal),		/* Size of structure */
	{ 0,0 },		/* Dataset version */
	Init,
	NULL,			/* reinit */
	NULL,			/* destroy */
	Load,
	Save,
	Edit
};

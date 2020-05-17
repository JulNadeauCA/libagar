/*	Public domain	*/

/*
 * Implementation of the example "Animal" class demonstrating use of
 * inheritance under the Agar Object system.
 */

#include "agartest.h"
#include "objsystem_animal.h"

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

#ifdef AG_TIMERS

/* Example of a timer callback routine. */
static Uint32
Tick(AG_Timer *to, AG_Event *event)
{
	Animal *animal = ANIMAL_SELF();

	animal->age += 1.0;
	animal->cellCount <<= 1;

	if (animal->cellCount < 1) {
		return (0);
	}
	return (to->ival);
}

#endif /* AG_TIMERS */

/* Handler for "attached" event (raised by AG_ObjectAttach() call). */
static void
OnAttach(AG_Event *event)
{
	Animal *animal = ANIMAL_SELF();
	const AG_Object *parent = AG_PTR(1);

	Verbose("%s: attached to %s\n", AGOBJECT(animal)->name,
	                                AGOBJECT(parent)->name);
#ifdef AG_TIMERS
	AG_AddTimer(animal, &animal->time, 1000, Tick, NULL);
#endif
}

/* An example method: "find-primes"(int nPrimes) */
static void
FindPrimes(AG_Event *event)
{
	Animal *animal = ANIMAL_PTR(1);
	const int nPrimes = AG_NumericalGetInt(AG_NUMERICAL_PTR(2));
	int n, i, nFound=1;

	for (n=0; nFound <= nPrimes; n++) {
		int flag = 0;

		for (i = 2; i <= (n >> 1); ++i) {
			if ((n % i) == 0) {
				flag = 1;
				break;
			}
		}
		if (n != 1) {
			if (!flag) {
				Verbose("%s: %d is prime #%d\n",
				    AGOBJECT(animal)->name, n, nFound);
				nFound++;
			}
		}
	}

	/* Count total */
	AG_SetInt(animal,"found-primes",
	    AG_GetInt(animal,"found-primes") + nFound);
}

static void
OnDetach(AG_Event *event)
{
	const Animal *animal = ANIMAL_SELF();

	Verbose("%s: detached from parent\n", AGOBJECT(animal)->name);
}

/*
 * Initialization routine. Note that the object system will automatically
 * invoke the initialization routines of all parent classes first.
 */
static void
Init(void *obj)
{
	Animal *animal = obj;

	/* Instance variables */
	animal->age = 0.0;
	animal->cellCount = 1;
#ifdef AG_TIMERS
	AG_InitTimer(&animal->time, "tick", 0);
#endif
	/* Dynamic AG_Object variables */
	AG_SetInt(animal, "found-primes", 0);

	/* Event handlers and methods */
	AG_SetEvent(animal, "attached", OnAttach, NULL);
	AG_SetEvent(animal, "detached", OnDetach, NULL);
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
	const Animal *animal = obj;

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
	AG_Box *box;
	AG_Numerical *numPrimes;
	AG_Label *lbl;

	if ((win = AG_WindowNew(0)) == NULL) {
		AG_FatalError(NULL);
	}
	AG_WindowSetCaption(win, "Animal: %s", AGOBJECT(animal)->name);

	lbl = AG_LabelNewS(win, AG_LABEL_HFILL, AGOBJECT(animal)->name);
	AG_SetStyle(lbl, "font-size", "200%");

	AG_NumericalNewFlt(win, AG_NUMERICAL_HFILL, "sec", "Age: ", &animal->age);
	AG_NumericalNewInt(win, AG_NUMERICAL_HFILL, NULL, "Cell count: ", &animal->cellCount);

	AG_SeparatorNewHoriz(win);

	box = AG_BoxNewHoriz(win, AG_BOX_EXPAND);
	numPrimes = AG_NumericalNewS(box, AG_NUMERICAL_INT | AG_NUMERICAL_HFILL,
	                             NULL, _("Find "));
	AG_NumericalSizeHint(numPrimes, "<88>");
	AG_LabelNewS(box, 0, _("primes"));
	AG_ButtonNewFn(box, 0, _("Start"), FindPrimes,"%p,%p", animal, numPrimes);

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
AG_ObjectClass AnimalClass = {
	"Animal",		/* Inheritance hierarchy (implies AG_Object) */
	sizeof(Animal),		/* Size of instance structures */
	{ 0,0 },		/* Version (Major, Minor) */
	Init,
	NULL,			/* reset */
	NULL,			/* destroy */
	Load,
	Save,
	Edit
};

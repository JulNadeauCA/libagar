/*	Public domain	*/

/*
 * Implementation of example class Animal to demonstrate inheritance:
 * AG_Object -> Animal.
 */

#include "agartest.h"
#include "objsystem_animal.h"

MY_Animal *
MY_AnimalNew(void *parent)
{
	MY_Animal *animal;

	animal = AG_Malloc(sizeof(MY_Animal));
	AG_ObjectInit(animal, &myAnimalClass);
	AG_ObjectAttach(parent, animal);
	return (animal);
}

#ifdef AG_TIMERS
/*
 * Example timer expiration routine.
 */
static Uint32
Tick(AG_Timer *to, AG_Event *event)
{
	MY_Animal *animal = MY_ANIMAL_SELF();

	animal->age += 1.0;
	animal->cellCount <<= 1;

	if (animal->cellCount < 1) {
		return (0);
	}
	return (to->ival);
}
#endif /* AG_TIMERS */

/* Called on attach to parent. */
static void
OnAttach(AG_Event *event)
{
	MY_Animal *animal = MY_ANIMAL_SELF();
	AG_Object *parent = AG_PTR(1);

	AG_Debug(animal, "Now attached to %s\n", AGOBJECT(parent)->name);
#ifdef AG_TIMERS
	AG_AddTimer(animal, &animal->time, 1000, Tick, NULL);
#endif
}

/* An example procedure */
static void
FindPrimes(AG_Event *event)
{
	MY_Animal *animal = MY_ANIMAL_PTR(1);
	AG_Numerical *num = AG_NUMERICAL_PTR(2);
	int nPrimes, n, i, nFound=1;

	nPrimes = AG_NumericalGetInt(num);

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
Init(void *obj)
{
	MY_Animal *animal = obj;

	animal->age = 0.0;
	animal->cellCount = 1;
#ifdef AG_TIMERS
	AG_InitTimer(&animal->time, "tick", 0);
#endif
	AG_SetInt(animal, "found-primes", 0);

	AG_SetEvent(animal, "attached", OnAttach, NULL);
}

static int
Load(void *obj, AG_DataSource *ds, const AG_Version *ver)
{
	MY_Animal *animal = obj;

	animal->age = AG_ReadFloat(ds);
	animal->cellCount = (int)AG_ReadUint32(ds);
	return (0);
}

static int
Save(void *obj, AG_DataSource *ds)
{
	const MY_Animal *animal = obj;

	AG_WriteFloat(ds, animal->age);
	AG_WriteUint32(ds, (int)animal->cellCount);
	return (0);
}

static void *
Edit(void *obj)
{
	MY_Animal *animal = obj;
	AG_Window *win;
	AG_Box *box;
	AG_Numerical *numPrimes;
	AG_Label *lbl;

	if ((win = AG_WindowNew(0)) == NULL) {
		AG_FatalError(NULL);
	}
	AG_WindowSetCaption(win, "Animal: %s", AGOBJECT(animal)->name);

	lbl = AG_LabelNewS(win, AG_LABEL_HFILL, AGOBJECT(animal)->name);
	AG_SetFontSize(lbl, "200%");

	AG_NumericalNewFlt(win, AG_NUMERICAL_HFILL, "sec",
	    "Age: ", &animal->age);
	AG_NumericalNewInt(win, AG_NUMERICAL_HFILL, NULL,
	    "Cell count: ", &animal->cellCount);

	AG_SeparatorNewHoriz(win);

	box = AG_BoxNewHoriz(win, AG_BOX_EXPAND);
	{
		numPrimes = AG_NumericalNewS(box,
		    AG_NUMERICAL_INT | AG_NUMERICAL_HFILL,
		    NULL, _("Find "));
		AG_NumericalSizeHint(numPrimes, "<88>");
		AG_LabelNewS(box, 0, _("primes"));
		AG_ButtonNewFn(box, 0, _("Start"),
		    FindPrimes, "%p,%p", animal, numPrimes);
	}

	return (win);
}

AG_ObjectClass myAnimalClass = {
	"MY_Animal",
	sizeof(MY_Animal),
	{ 1,0, 0, 0 },
	Init,
	NULL,    /* reset */
	NULL,    /* destroy */
	Load,
	Save,
	Edit
};

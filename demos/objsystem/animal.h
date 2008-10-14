/*	Public domain	*/

#include <agar/begin.h>

typedef struct animal {
	struct ag_object obj;		/* Parent class */
	float age;			/* Age */
	int cellCount;			/* Cell count */
	AG_Timeout time;		/* Timer */
} Animal;

extern AG_ObjectClass AnimalClass;
Animal *AnimalNew(void *);

#include <agar/close.h>

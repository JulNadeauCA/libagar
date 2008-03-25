/*	Public domain	*/

#include "begin_code.h"

typedef struct animal {
	struct ag_object obj;		/* Parent class */
	float age;			/* Age */
	int cellCount;			/* Cell count */
	AG_Timeout time;		/* Timer */
} Animal;

extern AG_ObjectClass AnimalClass;
Animal *AnimalNew(void *);

#include "close_code.h"

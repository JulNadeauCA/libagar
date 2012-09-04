/*	Public domain	*/

typedef struct animal {
	struct ag_object obj;		/* Parent class */
	float age;			/* Age */
	int cellCount;			/* Cell count */
	AG_Timer time;			/* Timer */
} Animal;

extern AG_ObjectClass AnimalClass;
Animal *AnimalNew(void *);

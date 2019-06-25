/*	Public domain	*/

typedef struct animal {
	struct ag_object obj;		/* Parent class */
	float age;			/* Age */
	int cellCount;			/* Cell count */
	AG_Timer time;			/* Timer */
} Animal;

#define ANIMAL(obj)           ((Animal *)(obj))
#define ANIMAL_SELF()         AG_OBJECT(0,"Animal:*")
#define ANIMAL_PTR(n)         AG_OBJECT((n),"Animal:*")
#define ANIMAL_NAMED(n)       AG_OBJECT_NAMED((n),"Animal:*")
#define CONST_ANIMAL_SELF()   AG_CONST_OBJECT(0,"Animal:*")
#define CONST_ANIMAL_PTR(n)   AG_CONST_OBJECT((n),"Animal:*")
#define CONST_ANIMAL_NAMED(n) AG_CONST_OBJECT_NAMED((n),"Animal:*")

extern AG_ObjectClass AnimalClass;
Animal *AnimalNew(void *);

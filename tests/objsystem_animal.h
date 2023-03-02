/*	Public domain	*/

typedef struct my_animal {
	struct ag_object obj;		/* Parent class */
	float age;			/* Age */
	int cellCount;			/* Cell count */
	AG_Timer time;			/* Timer */
} MY_Animal;

#define   MYANIMAL(o)        ((MY_Animal *)(o))
#define  MYcANIMAL(o)        ((const MY_Animal *)(o))
#define  MY_ANIMAL_SELF()    MYANIMAL(  AG_OBJECT(0,         "MY_Animal:*") )
#define  MY_ANIMAL_PTR(n)    MYANIMAL(  AG_OBJECT((n),       "MY_Animal:*") )
#define  MY_ANIMAL_NAMED(n)  MYANIMAL(  AG_OBJECT_NAMED((n), "MY_Animal:*") )
#define MYc_ANIMAL_SELF()   MYcANIMAL( AG_cOBJECT(0,         "MY_Animal:*") )
#define MYc_ANIMAL_PTR(n)   MYcANIMAL( AG_cOBJECT((n),       "MY_Animal:*") )
#define MYc_ANIMAL_NAMED(n) MYcANIMAL( AG_cOBJECT_NAMED((n), "MY_Animal:*") )

extern AG_ObjectClass myAnimalClass;
MY_Animal *MY_AnimalNew(void *);

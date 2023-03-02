/*	Public domain	*/

typedef struct my_mammal {
	struct my_animal animal;	/* MY_Animal -> MY_Mammal */
	struct {
		float h, s, v;		/* Hair color */
	} hairColor;
} MY_Mammal;

#define   MYMAMMAL(o)        ((MY_Mammal *)(o))
#define  MYcMAMMAL(o)        ((const MY_Mammal *)(o))
#define  MY_MAMMAL_SELF()    MYMAMMAL(  AG_OBJECT(0,         "Animal:Mammal:*") )
#define  MY_MAMMAL_PTR(n)    MYMAMMAL(  AG_OBJECT((n),       "Animal:Mammal:*") )
#define  MY_MAMMAL_NAMED(n)  MYMAMMAL(  AG_OBJECT_NAMED((n), "Animal:Mammal:*") )
#define MY_cMAMMAL_SELF()   MYcMAMMAL( AG_cOBJECT(0,         "Animal:Mammal:*") )
#define MY_cMAMMAL_PTR(n)   MYcMAMMAL( AG_cOBJECT((n),       "Animal:Mammal:*") )
#define MY_cMAMMAL_NAMED(n) MYcMAMMAL( AG_cOBJECT_NAMED((n), "Animal:Mammal:*") )

extern AG_ObjectClass myMammalClass;

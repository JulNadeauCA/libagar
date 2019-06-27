/*	Public domain	*/

typedef struct mammal {
	struct animal animal;		/* Parent class */
	struct {
		float h, s, v;		/* Hair color */
	} hairColor;
} Mammal;

#define MAMMAL(obj)              ((Mammal *)(obj))
#define CMAMMAL(obj)             ((const Mammal *)(obj))
#define AG_MAMMAL_SELF()          MAMMAL( AG_OBJECT(0,"Animal:Mammal:*") )
#define AG_MAMMAL_PTR(n)          MAMMAL( AG_OBJECT((n),"Animal:Mammal:*") )
#define AG_MAMMAL_NAMED(n)        MAMMAL( AG_OBJECT_NAMED((n),"Animal:Mammal:*") )
#define AG_CONST_MAMMAL_SELF()   CMAMMAL( AG_CONST_OBJECT(0,"Animal:Mammal:*") )
#define AG_CONST_MAMMAL_PTR(n)   CMAMMAL( AG_CONST_OBJECT((n),"Animal:Mammal:*") )
#define AG_CONST_MAMMAL_NAMED(n) CMAMMAL( AG_CONST_OBJECT_NAMED((n),"Animal:Mammal:*") )

extern AG_ObjectClass MammalClass;

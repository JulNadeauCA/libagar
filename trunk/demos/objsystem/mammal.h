/*	Public domain	*/

#include <agar/begin.h>

typedef struct mammal {
	struct animal animal;		/* Parent class */
	struct {
		float h, s, v;		/* Hair color */
	} hairColor;
} Mammal;

extern AG_ObjectClass MammalClass;

#include <agar/close.h>

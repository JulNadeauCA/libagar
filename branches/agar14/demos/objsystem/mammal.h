/*	Public domain	*/

#include "begin_code.h"

typedef struct mammal {
	struct animal animal;		/* Parent class */
	struct {
		float h, s, v;		/* Hair color */
	} hairColor;
} Mammal;

extern AG_ObjectClass MammalClass;

#include "close_code.h"

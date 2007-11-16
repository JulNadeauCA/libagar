/*	Public domain	*/

#include "begin_code.h"

typedef struct mammal {
	struct animal animal;		/* Parent class */
	struct {
		float h, s, v;		/* Hair color */
	} hairColor;
} Mammal;

__BEGIN_DECLS
extern const AG_ObjectOps MammalOps;
__END_DECLS

#include "close_code.h"

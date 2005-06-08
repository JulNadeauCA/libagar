/*	$Csoft: mouse.h,v 1.2 2005/05/08 11:09:20 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

typedef struct ag_mouse {
	struct ag_input in;
	int index;
} AG_Mouse;

extern const AG_InputOps agMouseOps;

__BEGIN_DECLS
AG_Mouse	*AG_MouseNew(int);
Uint8		 AG_MouseGetState(int *, int *);
__END_DECLS

#include "close_code.h"

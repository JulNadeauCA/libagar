/*	$Csoft: mouse.h,v 1.1 2003/09/07 00:24:09 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct mouse {
	struct input in;
	int index;
};

__BEGIN_DECLS
struct mouse	*mouse_new(int);
Uint8		 mouse_get_state(int *, int *);
__END_DECLS

#include "close_code.h"

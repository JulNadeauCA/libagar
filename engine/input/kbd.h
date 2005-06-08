/*	$Csoft: kbd.h,v 1.1 2003/09/07 00:24:09 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

typedef struct ag_keyboard {
	struct ag_input in;
	int index;
} AG_Keyboard;

extern const AG_InputOps agKeyboardOps;

__BEGIN_DECLS
AG_Keyboard *AG_KeyboardNew(int);
__END_DECLS

#include "close_code.h"

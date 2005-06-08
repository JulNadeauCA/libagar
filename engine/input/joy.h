/*	$Csoft: joy.h,v 1.1 2003/09/07 00:24:09 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

typedef struct ag_joystick {
	struct ag_input in;
	SDL_Joystick *joy;
	int index;
} AG_Joystick;

extern const AG_InputOps agJoystickOps;

__BEGIN_DECLS
AG_Joystick *AG_JoystickNew(int);
__END_DECLS

#include "close_code.h"

/*	$Csoft$	*/
/*	Public domain	*/

#include "begin_code.h"

struct joy {
	struct input in;

	SDL_Joystick	*joy;
	int		 index;
};

__BEGIN_DECLS
struct joy	*joy_new(int);
__END_DECLS

#include "close_code.h"

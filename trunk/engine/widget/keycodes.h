/*	$Csoft: keycodes.h,v 1.11 2003/06/15 08:54:19 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_KEYCODES_H_
#define _AGAR_WIDGET_KEYCODES_H_
#include "begin_code.h"

struct keycode {
	SDLKey	 key;
	int	 modmask;
	void	(*func)(struct textbox *, SDLKey, int, const char *, Uint32);
	char	*arg;
};

extern const struct keycode keycodes[];		/* keymap */

#include "close_code.h"
#endif	/* _AGAR_WIDGET_KEYCODES_H_ */

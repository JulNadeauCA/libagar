/*	$Csoft: keycodes.h,v 1.13 2003/09/01 10:34:03 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_KEYCODES_H_
#define _AGAR_WIDGET_KEYCODES_H_
#include "begin_code.h"

struct keycode {
	SDLKey	 key;
	int	 modmask;
	int	(*func)(struct textbox *, SDLKey, int, const char *, Uint32);
	char	*arg;
	int	 clr_compo;
};

extern const struct keycode keycodes[];		/* keymap */

#include "close_code.h"
#endif	/* _AGAR_WIDGET_KEYCODES_H_ */

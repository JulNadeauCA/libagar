/*	$Csoft: keycodes.h,v 1.9 2003/04/25 09:47:10 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_KEYCODES_H_
#define _AGAR_WIDGET_KEYCODES_H_
#include "begin_code.h"

struct keycode {
	char	*name;
	SDLKey	 key;
	int	 modmask;
	void	(*func)(struct textbox *, SDLKey, int, char *);
	char	*arg;
};

extern const struct keycode keycodes[];		/* keymap */

#include "close_code.h"
#endif	/* _AGAR_WIDGET_KEYCODES_H_ */

/*	$Csoft: keycodes.h,v 1.14 2005/02/07 13:17:16 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_KEYCODES_H_
#define _AGAR_WIDGET_KEYCODES_H_
#include "begin_code.h"

struct ag_keycode {
	SDLKey key;
	int modmask;
	int (*func)(AG_Textbox *, SDLKey, int, const char *, Uint32);
	char *arg;
	int clr_compo;
};

extern const struct ag_keycode agKeyCodes[];	/* keymap */

#include "close_code.h"
#endif	/* _AGAR_WIDGET_KEYCODES_H_ */

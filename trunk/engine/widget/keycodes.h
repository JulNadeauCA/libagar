/*	$Csoft: keycodes.h,v 1.8 2003/01/04 14:10:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_KEYCODES_H_
#define _AGAR_WIDGET_KEYCODES_H_
#include "begin_code.h"

struct keycode {
	char	*name;
	SDLKey	 key;
	int	 modmask;

	/*
	 * Callback routine must be invoked when the textbox's text
	 * substructure is locked.
	 */
	void	(*callback)(struct textbox *, SDLKey, int, char *);
	char	*arg;
};

#define KEYMAP_US		0x01
#define KEYMAP_UTU		0x02

#ifndef KEYCODES_KEYMAP
#define KEYCODES_KEYMAP		KEYMAP_US
#endif

extern const struct keycode keycodes[];		/* keymap */

__BEGIN_DECLS
extern DECLSPEC void	keycodes_init(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_KEYCODES_H_ */

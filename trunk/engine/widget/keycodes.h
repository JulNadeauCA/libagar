/*	$Csoft: keycodes.h,v 1.7 2002/09/07 04:34:14 vedge Exp $	*/
/*	Public domain	*/

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

void	keycodes_init(void);


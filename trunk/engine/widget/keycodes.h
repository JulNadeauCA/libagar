/*	$Csoft: keycodes.h,v 1.6 2002/06/09 10:08:08 vedge Exp $	*/
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

#define KEYCODES_CACHE_START	0x20	/* sp */
#define KEYCODES_CACHE_END	0x7e	/* ~ */

extern const struct keycode keycodes[];		/* keymap */
extern SDL_Surface *keycodes_cache[];		/* keymap */

void	keycodes_init(void);
void	keycodes_loadglyphs(void);
void	keycodes_freeglyphs(void);


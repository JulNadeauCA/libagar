/*	Public domain	*/

#ifndef _AGAR_WIDGET_KEYMAP_H_
#define _AGAR_WIDGET_KEYMAP_H_
#include <agar/gui/begin.h>

struct ag_editable;
struct ag_editable_buffer;

struct ag_keycode {
	AG_KeySym key;			/* Match keysym */
	Uint      modKeys;		/* Match set of modifier keys */
	int (*_Nonnull func)(struct ag_editable *_Nonnull,
	                     struct ag_editable_buffer *_Nonnull,
	                     AG_KeySym, Uint, AG_Char);
	const char *_Nonnull flags;
};

struct ag_key_composition {
	AG_Char comp;		/* This first key */
	AG_Char key;		/* And this second key */
	AG_Char res;		/* Combines into this native char */
};

struct ag_key_mapping {
	AG_KeySym key;		/* This key */
	Uint modmask;		/* With this modifier mask */
	AG_Char ch;		/* Maps to this native char */ 
};

__BEGIN_DECLS
/* Map AG_Keysym to AG_Editable(3) ops */
extern const struct ag_keycode agKeymap[];

#ifdef AG_UNICODE

/* Alternate (ALT+SHIFT) entry method for LATIN1 chars. */
extern const struct ag_key_mapping agKeymapLATIN1[];

/* Alternate key composition map. */
extern const struct ag_key_composition agCompositionMap[];
extern const int                       agCompositionMapSize;

int AG_KeyInputCompose(struct ag_editable *_Nonnull, AG_Char, AG_Char *_Nonnull);

#endif /* AG_UNICODE */
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_WIDGET_KEYMAP_H_ */

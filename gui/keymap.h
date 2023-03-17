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
extern const struct ag_keycode agKeymap[];    /* Map keysym to Editable ops */

#ifdef AG_UNICODE
/* Built-in input method for LATIN-1 characters. */
extern const struct ag_key_mapping agKeymapLatin1[];
extern const struct ag_key_composition agKeyComposeMapLatin[];
extern const int                       agKeyComposeMapLatinSize;

int AG_KeyComposeLatin(struct ag_editable *_Nonnull, AG_Char, AG_Char *_Nonnull);
#endif /* AG_UNICODE */
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_WIDGET_KEYMAP_H_ */

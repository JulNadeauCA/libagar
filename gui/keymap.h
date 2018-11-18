/*	Public domain	*/

#ifndef _AGAR_WIDGET_KEYMAP_H_
#define _AGAR_WIDGET_KEYMAP_H_
#include <agar/gui/begin.h>

struct ag_editable;
struct ag_editable_buffer;

struct ag_keycode {
	AG_KeySym key;
	const char *_Nonnull modFlags;
	int (*_Nonnull func)(struct ag_editable *_Nonnull,
	                     struct ag_editable_buffer *_Nonnull,
	                     AG_KeySym, Uint, Uint32);
	const char *_Nonnull flags;
};

struct ag_key_composition {
	Uint32 comp;		/* First key */
	Uint32 key;		/* Second key */
	Uint32 res;		/* Resulting character */
};

struct ag_key_mapping {
	AG_KeySym key;		/* Key */
	Uint modmask;		/* Modifier mask */
	Uint32 unicode;		/* UCS-4 mapping */
};

__BEGIN_DECLS
extern const struct ag_keycode     agKeymap[];       /* AG_KeySym to Editable ops */
extern const struct ag_key_mapping agKeymapLATIN1[]; /* Latin1 to AG_Keysym */

extern const struct ag_key_composition agCompositionMap[]; /* Key compose map */
extern const int                       agCompositionMapSize;

int AG_KeyInputCompose(struct ag_editable *_Nonnull, Uint32, Uint32 *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_WIDGET_KEYMAP_H_ */

/*	Public domain	*/

#ifndef _AGAR_WIDGET_KEYMAP_H_
#define _AGAR_WIDGET_KEYMAP_H_
#include <agar/gui/begin.h>

struct ag_keycode {
	AG_KeySym key;
	int modmask;
	int (*func)(AG_Editable *, AG_EditableBuffer *buf, AG_KeySym ks,
	            int mod, Uint32 ch);
};

struct ag_key_composition {
	Uint32 comp;		/* First key */
	Uint32 key;		/* Second key */
	Uint32 res;		/* Resulting character */
};

struct ag_key_mapping {
	AG_KeySym key;		/* Key */
	int modmask;		/* Modifier mask */
	Uint32 unicode;		/* UCS-4 mapping */
};

__BEGIN_DECLS
extern const struct ag_keycode		agKeymap[];
extern const struct ag_key_mapping	agKeymapLATIN1[];
extern const struct ag_key_composition  agCompositionMap[];
extern const int                        agCompositionMapSize;

Uint32 AG_ApplyModifiersASCII(Uint32, int);
int    AG_KeyInputCompose(AG_Editable *, Uint32, Uint32 *);
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_WIDGET_KEYMAP_H_ */

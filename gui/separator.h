/*	Public domain	*/

#ifndef _AGAR_WIDGET_SEPARATOR_H_
#define _AGAR_WIDGET_SEPARATOR_H_

#include <agar/gui/widget.h>

#include <agar/gui/begin.h>

enum ag_separator_type {
	AG_SEPARATOR_HORIZ,
	AG_SEPARATOR_VERT
};

typedef struct ag_separator {
	struct ag_widget wid;
	enum ag_separator_type type;
	Uint padding;				/* Padding in pixels */
	int visible;				/* Visible flag */
} AG_Separator;

__BEGIN_DECLS
extern AG_WidgetClass agSeparatorClass;

AG_Separator *AG_SeparatorNew(void *, enum ag_separator_type);
AG_Separator *AG_SpacerNew(void *, enum ag_separator_type);
void          AG_SeparatorSetPadding(AG_Separator *, Uint);

#define AG_SeparatorNewHoriz(p) AG_SeparatorNew((p),AG_SEPARATOR_HORIZ)
#define AG_SeparatorNewVert(p) AG_SeparatorNew((p),AG_SEPARATOR_VERT)
#define AG_SpacerNewHoriz(p) AG_SpacerNew((p),AG_SEPARATOR_HORIZ)
#define AG_SpacerNewVert(p) AG_SpacerNew((p),AG_SEPARATOR_VERT)
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_SEPARATOR_H_ */

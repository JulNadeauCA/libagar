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
	struct ag_widget wid;			/* AG_Widget -> AG_Separator */
	enum ag_separator_type type;
	Uint minLen;				/* Minimum length in pixels */
} AG_Separator;

__BEGIN_DECLS
extern AG_WidgetClass agSeparatorClass;

AG_Separator *_Nonnull AG_SeparatorNewHoriz(void *_Nullable);
AG_Separator *_Nonnull AG_SeparatorNewVert(void *_Nullable);
AG_Separator *_Nonnull AG_SeparatorNew(void *_Nullable, enum ag_separator_type);

AG_Separator *_Nonnull AG_SpacerNewHoriz(void *_Nullable);
AG_Separator *_Nonnull AG_SpacerNewVert(void *_Nullable);
AG_Separator *_Nonnull AG_SpacerNew(void *_Nullable, enum ag_separator_type);

void AG_SeparatorSetLength(AG_Separator *_Nonnull, Uint);

#ifdef AG_LEGACY
void AG_SeparatorSetPadding(AG_Separator *_Nonnull, Uint)
                           DEPRECATED_ATTRIBUTE;                
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_SEPARATOR_H_ */

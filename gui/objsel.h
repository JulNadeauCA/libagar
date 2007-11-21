/*	Public domain	*/

#ifndef _AGAR_WIDGET_OBJSEL_H_
#define _AGAR_WIDGET_OBJSEL_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/combo.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/combo.h>
#endif

#include "begin_code.h"

typedef struct ag_object_selector {
	struct ag_combo com;
	int flags;
#define AG_OBJSEL_PAGE_DATA	0x01
#define AG_OBJSEL_PAGE_GFX	0x02
	char type_mask[AG_OBJECT_TYPE_MAX];
	void *pobj;
	void *root;
	void *object;
} AG_ObjectSelector;

__BEGIN_DECLS
extern AG_WidgetClass agObjectSelectorClass;

AG_ObjectSelector *AG_ObjectSelectorNew(void *, int, void *, void *,
		                        const char *, ...)
					FORMAT_ATTRIBUTE(printf, 5, 6)
					NONNULL_ATTRIBUTE(5);
AG_TlistItem *AG_ObjectSelectorSelect(AG_ObjectSelector *, void *);
void	      AG_ObjectSelectorMaskType(AG_ObjectSelector *, const char *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_OBJSEL_H_ */

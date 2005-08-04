/*	$Csoft: objsel.h,v 1.10 2005/05/24 08:12:48 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_OBJSEL_H_
#define _AGAR_WIDGET_OBJSEL_H_

#include <engine/widget/widget.h>
#include <engine/widget/combo.h>

#include "begin_code.h"

struct objsel {
	struct combo com;
	int flags;
#define OBJSEL_PAGE_DATA	0x01
#define OBJSEL_PAGE_GFX		0x02
#define OBJSEL_PAGE_AUDIO	0x04
	char type_mask[OBJECT_TYPE_MAX];
	void *pobj;
	void *root;
	void *object;
};

__BEGIN_DECLS
struct objsel *objsel_new(void *, int, void *, void *, const char *, ...)
	         FORMAT_ATTRIBUTE(printf, 5, 6)
		 NONNULL_ATTRIBUTE(5);

void objsel_init(struct objsel *, const char *, int, void *, void *);
struct tlist_item *objsel_select(struct objsel *, void *);
void objsel_mask_type(struct objsel *, const char *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_OBJSEL_H_ */

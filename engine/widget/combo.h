/*	$Csoft: combo.h,v 1.3 2003/10/09 22:39:34 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_COMBO_H_
#define _AGAR_WIDGET_COMBO_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>

#include "begin_code.h"

#define COMBO_LABEL_MAX		32

struct combo {
	struct widget wid;

	int	 flags;
#define COMBO_MULTI		0x01
#define COMBO_MULTI_STICKY	0x02
#define COMBO_POLL		0x04
#define COMBO_TREE		0x08

	struct textbox	*tbox;
	struct button	*button;
	struct window	*win;
	struct tlist	*list;
};

__BEGIN_DECLS
struct combo	*combo_new(void *, int, const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 3, 4)
		     NONNULL_ATTRIBUTE(3);

void	 combo_init(struct combo *, const char *, int);
void	 combo_scale(void *, int, int);
void	 combo_destroy(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_COMBO_H_ */

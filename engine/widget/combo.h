/*	$Csoft: combo.h,v 1.15 2003/06/06 03:18:14 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_COMBO_H_
#define _AGAR_WIDGET_COMBO_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>

#include "begin_code.h"

#define COMBO_LABEL_MAX	32

struct combo {
	struct widget wid;

	struct textbox	*tbox;
	struct button	*button;
	
	struct window	*win;
	struct tlist	*list;
};

__BEGIN_DECLS
extern DECLSPEC struct combo	*combo_new(void *, const char *, ...);
extern DECLSPEC void		 combo_init(struct combo *, const char *);
extern DECLSPEC void		 combo_scale(void *, int, int);
extern DECLSPEC void		 combo_destroy(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_COMBO_H_ */

/*	$Csoft: combo.h,v 1.5 2003/10/14 02:16:29 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_COMBO_H_
#define _AGAR_WIDGET_COMBO_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>

#include "begin_code.h"

struct combo {
	struct widget wid;

	int flags;
#define COMBO_POLL	0x01		/* Polled list */
#define COMBO_TREE	0x02		/* Tree display */

	struct textbox *tbox;		/* Text input */
	struct button *button;		/* [...] button */
	struct tlist *list;		/* List of items */
	struct window *win;		/* Pop-up window */
	int saved_h;			/* Saved pop-up window height */
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

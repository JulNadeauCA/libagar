/*	$Csoft: spinbutton.h,v 1.8 2004/03/25 09:00:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_SPINBUTTON_H_
#define _AGAR_WIDGET_SPINBUTTON_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>

#include "begin_code.h"

struct spinbutton {
	struct widget wid;

	int value;			/* Default value binding */
	int min, max;			/* Default range bindings */
	pthread_mutex_t lock;
	int incr;			/* Increment for buttons */
	int writeable;			/* 0 = read-only */

	struct textbox *input;
	struct button *incbu;
	struct button *decbu;
};

__BEGIN_DECLS
struct spinbutton	*spinbutton_new(void *, const char *, ...)
		 	     FORMAT_ATTRIBUTE(printf, 2, 3)
			     NONNULL_ATTRIBUTE(2);

void	spinbutton_init(struct spinbutton *, const char *);
void	spinbutton_destroy(void *);
void	spinbutton_scale(void *, int, int);
void	spinbutton_draw(void *);

void	spinbutton_add_value(struct spinbutton *, int);
void	spinbutton_set_value(struct spinbutton *, ...);
void	spinbutton_set_min(struct spinbutton *, int);
void	spinbutton_set_max(struct spinbutton *, int);
void	spinbutton_set_range(struct spinbutton *, int, int);
void	spinbutton_set_increment(struct spinbutton *, int);
void	spinbutton_set_writeable(struct spinbutton *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_SPINBUTTON_H_ */

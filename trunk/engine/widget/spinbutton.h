/*	$Csoft: spinbutton.h,v 1.2 2003/06/10 07:59:15 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_SPINBUTTON_H_
#define _AGAR_WIDGET_SPINBUTTON_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>

#include "begin_code.h"

#define SPINBUTTON_LABEL_MAX	32

struct spinbutton {
	struct widget wid;

	int		 value;			/* Value binding */
	int		 min, max;		/* Range binding */

	pthread_mutex_t	 lock;
	int		 incr;			/* Default increment */

	struct textbox	*tbox;
	struct button	*incbu;
	struct button	*decbu;
};

__BEGIN_DECLS
struct spinbutton *spinbutton_new(void *, const char *, ...);

void	spinbutton_init(struct spinbutton *, const char *);
void	spinbutton_destroy(void *);
void	spinbutton_scale(void *, int, int);
void	spinbutton_draw(void *);

void	spinbutton_add(struct spinbutton *, int);
void	spinbutton_set_value(struct spinbutton *, int);
void	spinbutton_set_min(struct spinbutton *, int);
void	spinbutton_set_max(struct spinbutton *, int);
void	spinbutton_set_increment(struct spinbutton *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_SPINBUTTON_H_ */

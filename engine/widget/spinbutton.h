/*	$Csoft: spinbutton.h,v 1.15 2003/06/06 03:18:14 vedge Exp $	*/
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
extern DECLSPEC struct spinbutton *spinbutton_new(void *, const char *, ...);

extern DECLSPEC void spinbutton_init(struct spinbutton *, const char *);
extern DECLSPEC void spinbutton_destroy(void *);
extern DECLSPEC void spinbutton_scale(void *, int, int);
extern DECLSPEC void spinbutton_draw(void *);

extern DECLSPEC void spinbutton_increment(struct spinbutton *, int);
extern DECLSPEC void spinbutton_decrement(struct spinbutton *, int);
extern DECLSPEC void spinbutton_set_value(struct spinbutton *, int);
extern DECLSPEC void spinbutton_set_min(struct spinbutton *, int);
extern DECLSPEC void spinbutton_set_max(struct spinbutton *, int);
extern DECLSPEC void spinbutton_set_increment(struct spinbutton *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_SPINBUTTON_H_ */

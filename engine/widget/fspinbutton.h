/*	$Csoft: spinbutton.h,v 1.4 2003/10/09 22:39:34 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_FSPINBUTTON_H_
#define _AGAR_WIDGET_FSPINBUTTON_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>

#include "begin_code.h"

struct fspinbutton {
	struct widget wid;

	double	value;			/* Value binding */

	pthread_mutex_t	lock;
	double	min, max;		/* Value bounds */
	char	format[8];		/* Format for printf */
	double	incr;			/* Default increment */

	struct textbox	*tbox;
	struct button	*incbu;
	struct button	*decbu;
};

__BEGIN_DECLS
struct fspinbutton	*fspinbutton_new(void *, const char *, ...)
		 	     FORMAT_ATTRIBUTE(printf, 2, 3)
			     NONNULL_ATTRIBUTE(2);

void	fspinbutton_init(struct fspinbutton *, const char *);
void	fspinbutton_destroy(void *);
void	fspinbutton_scale(void *, int, int);
void	fspinbutton_draw(void *);

void	fspinbutton_add(struct fspinbutton *, double);
void	fspinbutton_set_value(struct fspinbutton *, double);
void	fspinbutton_set_min(struct fspinbutton *, double);
void	fspinbutton_set_max(struct fspinbutton *, double);
void	fspinbutton_set_increment(struct fspinbutton *, double);
void	fspinbutton_set_precision(struct fspinbutton *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_FSPINBUTTON_H_ */

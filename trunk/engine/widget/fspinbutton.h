/*	$Csoft: fspinbutton.h,v 1.2 2003/11/15 03:52:45 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_FSPINBUTTON_H_
#define _AGAR_WIDGET_FSPINBUTTON_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/ucombo.h>
#include <engine/widget/units.h>

#include "begin_code.h"

struct fspinbutton {
	struct widget wid;

	pthread_mutex_t	lock;
	double value;			/* Value binding */
	double min, max;		/* Value boundaries */
	char format[8];			/* Format for printf (for precision) */
	float incr;			/* Increment for [+]/[-] buttons */
	const struct unit *unit;	/* Chosen conversion unit */

	struct textbox *input;		/* Text input */
	struct ucombo *units;		/* Unit selection */
	struct button *incbu;		/* [+] button */
	struct button *decbu;		/* [-] button */
};

__BEGIN_DECLS
struct fspinbutton	*fspinbutton_new(void *, double, double, float,
			                 const char *, ...)
		 	     FORMAT_ATTRIBUTE(printf, 5, 6)
			     NONNULL_ATTRIBUTE(5);

void	fspinbutton_init(struct fspinbutton *, double, double, float,
	                 const char *);
void	fspinbutton_destroy(void *);
void	fspinbutton_scale(void *, int, int);
void	fspinbutton_draw(void *);
void	fspinbutton_add(struct fspinbutton *, double);
void	fspinbutton_set_value(struct fspinbutton *, double);
void	fspinbutton_set_min(struct fspinbutton *, double);
void	fspinbutton_set_max(struct fspinbutton *, double);
void	fspinbutton_set_increment(struct fspinbutton *, float);
void	fspinbutton_set_precision(struct fspinbutton *, int);
void	fspinbutton_set_units(struct fspinbutton *, const struct unit[],
	                      double, double);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_FSPINBUTTON_H_ */

/*	$Csoft: mfspinbutton.h,v 1.9 2004/03/25 04:35:45 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_MFSPINBUTTON_H_
#define _AGAR_WIDGET_MFSPINBUTTON_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/ucombo.h>
#include <engine/widget/units.h>

#include "begin_code.h"

struct mfspinbutton {
	struct widget wid;

	pthread_mutex_t	lock;
	double xvalue, yvalue;		/* Default x/y value bindings */
	double min, max;		/* Default range bindings */
	double inc;			/* Increment for buttons */
	char format[32];		/* Printing format */
	const char *sep;		/* x/y field separator */
	const struct unit *unit;	/* Conversion unit */
	int writeable;			/* 0 = read-only */

	struct textbox *input;
	struct ucombo *units;
	struct button *xincbu, *xdecbu;
	struct button *yincbu, *ydecbu;
};

__BEGIN_DECLS
struct mfspinbutton	*mfspinbutton_new(void *, const struct unit *,
			                  const char *, const char *, ...)
		 	     FORMAT_ATTRIBUTE(printf, 4, 5)
			     NONNULL_ATTRIBUTE(4);

void	mfspinbutton_init(struct mfspinbutton *, const struct unit *,
	                  const char *, const char *);
void	mfspinbutton_destroy(void *);
void	mfspinbutton_scale(void *, int, int);
void	mfspinbutton_draw(void *);

void	mfspinbutton_set_value(struct mfspinbutton *, const char *, double);
void	mfspinbutton_add_value(struct mfspinbutton *, const char *, double);
void	mfspinbutton_set_min(struct mfspinbutton *, double);
void	mfspinbutton_set_max(struct mfspinbutton *, double);
void	mfspinbutton_set_range(struct mfspinbutton *, double, double);
void	mfspinbutton_set_increment(struct mfspinbutton *, double);
void	mfspinbutton_set_units(struct mfspinbutton *, const struct unit[]);
void	mfspinbutton_select_unit(struct mfspinbutton *, const char *);
void	mfspinbutton_set_precision(struct mfspinbutton *, const char *, int);
void	mfspinbutton_set_writeable(struct mfspinbutton *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_MFSPINBUTTON_H_ */

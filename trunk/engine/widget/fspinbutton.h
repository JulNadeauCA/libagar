/*	$Csoft: fspinbutton.h,v 1.11 2004/08/22 12:08:16 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_FSPINBUTTON_H_
#define _AGAR_WIDGET_FSPINBUTTON_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/ucombo.h>

#include "begin_code.h"

struct fspinbutton {
	struct widget wid;

	pthread_mutex_t	lock;
	double value;			/* Default value binding */
	double min, max;		/* Default range bindings */
	double inc;			/* Increment for buttons */
	char format[32];		/* Printing format */
	const struct unit *unit;	/* Conversion unit in use */
	int writeable;			/* 0 = read-only */

	struct textbox *input;
	struct ucombo *units;
	struct button *incbu;
	struct button *decbu;
};

__BEGIN_DECLS
struct fspinbutton *fspinbutton_new(void *, const char *, const char *, ...)
		       FORMAT_ATTRIBUTE(printf, 3, 4)
		       NONNULL_ATTRIBUTE(3);

void	fspinbutton_init(struct fspinbutton *, const char *, const char *);
void	fspinbutton_destroy(void *);
void	fspinbutton_prescale(struct fspinbutton *, const char *);
void	fspinbutton_scale(void *, int, int);
void	fspinbutton_draw(void *);

void	fspinbutton_set_value(struct fspinbutton *, double);
void	fspinbutton_add_value(struct fspinbutton *, double);
void	fspinbutton_set_min(struct fspinbutton *, double);
void	fspinbutton_set_max(struct fspinbutton *, double);
void	fspinbutton_set_range(struct fspinbutton *, double, double);
void	fspinbutton_set_increment(struct fspinbutton *, double);
void	fspinbutton_select_unit(struct fspinbutton *, const char *);
void	fspinbutton_set_precision(struct fspinbutton *, const char *, int);
void	fspinbutton_set_writeable(struct fspinbutton *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_FSPINBUTTON_H_ */

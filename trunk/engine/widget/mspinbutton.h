/*	$Csoft: mspinbutton.h,v 1.2 2004/03/25 09:00:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_MSPINBUTTON_H_
#define _AGAR_WIDGET_MSPINBUTTON_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>

#include "begin_code.h"

struct mspinbutton {
	struct widget wid;
	const char *sep;			/* x/y value separator */
	int xvalue, yvalue;			/* Default x/y bindings */
	int min, max;				/* Default range bindings */
	pthread_mutex_t	lock;
	int inc;				/* Increment for buttons */
	int writeable;				/* 0 = read-only */
	struct textbox *input;
	struct button *xincbu, *xdecbu;
	struct button *yincbu, *ydecbu;
};

__BEGIN_DECLS
struct mspinbutton *mspinbutton_new(void *, const char *, const char *, ...)
		        FORMAT_ATTRIBUTE(printf, 3, 4)
		        NONNULL_ATTRIBUTE(3);

void	mspinbutton_init(struct mspinbutton *, const char *, const char *);
void	mspinbutton_destroy(void *);
void	mspinbutton_scale(void *, int, int);
void	mspinbutton_draw(void *);

void	mspinbutton_add_value(struct mspinbutton *, const char *, int);
void	mspinbutton_set_value(struct mspinbutton *, const char *, ...);
void	mspinbutton_set_min(struct mspinbutton *, int);
void	mspinbutton_set_max(struct mspinbutton *, int);
void	mspinbutton_set_range(struct mspinbutton *, int, int);
void	mspinbutton_set_increment(struct mspinbutton *, int);
void	mspinbutton_set_writeable(struct mspinbutton *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_MSPINBUTTON_H_ */

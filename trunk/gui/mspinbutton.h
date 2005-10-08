/*	$Csoft: mspinbutton.h,v 1.3 2004/03/26 04:57:43 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_MSPINBUTTON_H_
#define _AGAR_WIDGET_MSPINBUTTON_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>

#include "begin_code.h"

typedef struct ag_mspinbutton {
	struct ag_widget wid;
	const char *sep;			/* x/y value separator */
	int xvalue, yvalue;			/* Default x/y bindings */
	int min, max;				/* Default range bindings */
	pthread_mutex_t	lock;
	int inc;				/* Increment for buttons */
	int writeable;				/* 0 = read-only */
	AG_Textbox *input;
	AG_Button *xincbu, *xdecbu;
	AG_Button *yincbu, *ydecbu;
} AG_MSpinbutton;

__BEGIN_DECLS
AG_MSpinbutton *AG_MSpinbuttonNew(void *, const char *, const char *, ...)
		                  FORMAT_ATTRIBUTE(printf, 3, 4)
		                  NONNULL_ATTRIBUTE(3);

void	AG_MSpinbuttonInit(AG_MSpinbutton *, const char *, const char *);
void	AG_MSpinbuttonDestroy(void *);
void	AG_MSpinbuttonScale(void *, int, int);
void	AG_MSpinbuttonDraw(void *);

void	AG_MSpinbuttonAddValue(AG_MSpinbutton *, const char *, int);
void	AG_MSpinbuttonSetValue(AG_MSpinbutton *, const char *, ...);
void	AG_MSpinbuttonSetMin(AG_MSpinbutton *, int);
void	AG_MSpinbuttonSetMax(AG_MSpinbutton *, int);
void	AG_MSpinbuttonSetRange(AG_MSpinbutton *, int, int);
void	AG_MSpinbuttonSetIncrement(AG_MSpinbutton *, int);
void	AG_MSpinbuttonSetWriteable(AG_MSpinbutton *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_MSPINBUTTON_H_ */

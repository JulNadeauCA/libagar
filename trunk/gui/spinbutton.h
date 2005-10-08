/*	$Csoft: spinbutton.h,v 1.9 2004/03/26 04:57:43 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_SPINBUTTON_H_
#define _AGAR_WIDGET_SPINBUTTON_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>

#include "begin_code.h"

typedef struct ag_spinbutton {
	struct ag_widget wid;
	int value;			/* Default value binding */
	int min, max;			/* Default range bindings */
	pthread_mutex_t lock;
	int incr;			/* Increment for buttons */
	int writeable;			/* 0 = read-only */
	AG_Textbox *input;
	AG_Button *incbu;
	AG_Button *decbu;
} AG_Spinbutton;

__BEGIN_DECLS
AG_Spinbutton *AG_SpinbuttonNew(void *, const char *, ...)
		 	        FORMAT_ATTRIBUTE(printf, 2, 3)
			        NONNULL_ATTRIBUTE(2);

void	AG_SpinbuttonInit(AG_Spinbutton *, const char *);
void	AG_SpinbuttonDestroy(void *);
void	AG_SpinbuttonScale(void *, int, int);
void	AG_SpinbuttonDraw(void *);

void	AG_SpinbuttonAddValue(AG_Spinbutton *, int);
void	AG_SpinbuttonSetValue(AG_Spinbutton *, ...);
void	AG_SpinbuttonSetMin(AG_Spinbutton *, int);
void	AG_SpinbuttonSetMax(AG_Spinbutton *, int);
void	AG_SpinbuttonSetRange(AG_Spinbutton *, int, int);
void	AG_SpinbuttonSetIncrement(AG_Spinbutton *, int);
void	AG_SpinbuttonSetWriteable(AG_Spinbutton *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_SPINBUTTON_H_ */

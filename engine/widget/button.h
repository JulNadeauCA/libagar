/*	$Csoft: button.h,v 1.31 2005/01/30 05:39:11 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BUTTON_H_
#define _AGAR_WIDGET_BUTTON_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

enum ag_button_justify {
	AG_BUTTON_LEFT,
	AG_BUTTON_CENTER,
	AG_BUTTON_RIGHT
};

typedef struct ag_button {
	struct ag_widget wid;
	int state;			/* Default state binding */
	enum ag_button_justify justify;	/* Label justification */
	int flags;
#define AG_BUTTON_INSENSITIVE	0x01	/* Not responsive */
#define AG_BUTTON_STICKY	0x02	/* Toggle state */
#define AG_BUTTON_MOUSEOVER	0x04	/* Mouse overlaps */
#define AG_BUTTON_REPEAT	0x08	/* Send multiple button-pushed events */
	int padding;			/* Padding in pixels */
	AG_Timeout delay_to;		/* Delay for triggering repeat mode */
	AG_Timeout repeat_to;		/* Timeout for repeat mode */
} AG_Button;

__BEGIN_DECLS
AG_Button	*AG_ButtonNew(void *, const char *);

void	 AG_ButtonInit(AG_Button *, const char *);
void	 AG_ButtonDestroy(void *);
void	 AG_ButtonDraw(void *);
void	 AG_ButtonScale(void *, int, int);

void	 AG_ButtonEnable(AG_Button *);
void	 AG_ButtonDisable(AG_Button *);
void	 AG_ButtonSetPadding(AG_Button *, int);
void	 AG_ButtonSetFocusable(AG_Button *, int);
void	 AG_ButtonSetSticky(AG_Button *, int);
void	 AG_ButtonSetJustification(AG_Button *, enum ag_button_justify);
void	 AG_ButtonSetSurface(AG_Button *, SDL_Surface *);
void	 AG_ButtonSetRepeatMode(AG_Button *, int);
void	 AG_ButtonPrintf(AG_Button *, const char *, ...)
	     FORMAT_ATTRIBUTE(printf, 2, 3)
	     NONNULL_ATTRIBUTE(2);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BUTTON_H_ */

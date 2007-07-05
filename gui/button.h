/*	Public domain	*/

#ifndef _AGAR_WIDGET_BUTTON_H_
#define _AGAR_WIDGET_BUTTON_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

enum ag_button_justify {
	AG_BUTTON_LEFT,
	AG_BUTTON_CENTER,
	AG_BUTTON_RIGHT
};

typedef struct ag_button {
	struct ag_widget wid;
	int state;			/* Default state binding */
	char *text;			/* Label text */
	int surface;			/* Label surface handle */
	enum ag_button_justify justify;	/* Label justification */
	Uint flags;
#define AG_BUTTON_STICKY	0x002	/* Toggle state */
#define AG_BUTTON_MOUSEOVER	0x004	/* Mouse overlaps */
#define AG_BUTTON_REPEAT	0x008	/* Repeat button-pushed event */
#define AG_BUTTON_HFILL		0x010	/* Fill available width */
#define AG_BUTTON_VFILL		0x020	/* Fill available height */
#define AG_BUTTON_FOCUS		0x040	/* Focus button automatically */
#define AG_BUTTON_REGEN		0x080	/* Label surface need updating */
#define AG_BUTTON_TEXT_NODUP	0x100	/* Label text string is static */
#define AG_BUTTON_EXPAND	(AG_BUTTON_HFILL|AG_BUTTON_VFILL)

	int lPad, rPad, tPad, bPad;	/* Padding in pixels */
	AG_Timeout delay_to;		/* Delay for triggering repeat mode */
	AG_Timeout repeat_to;		/* Timeout for repeat mode */
} AG_Button;

__BEGIN_DECLS
AG_Button *AG_ButtonNew(void *, Uint, const char *);
AG_Button *AG_ButtonAct(void *, Uint, const char *,
			void (*)(AG_Event *), const char *, ...);
void	   AG_ButtonInit(AG_Button *, Uint, const char *);
void	   AG_ButtonDestroy(void *);
void	   AG_ButtonDraw(void *);
void	   AG_ButtonScale(void *, int, int);

void	   AG_ButtonSetPadding(AG_Button *, int, int, int, int);
#define	AG_ButtonSetPaddingLeft(lbl,v)   AG_ButtonSetPadding(lbl,(v),-1,-1,-1)
#define	AG_ButtonSetPaddingRight(lbl,v)  AG_ButtonSetPadding(lbl,-1,(v),-1,-1)
#define AG_ButtonSetPaddingTop(lbl,v)    AG_ButtonSetPadding(lbl,-1,-1,(v),-1)
#define	AG_ButtonSetPaddingBottom(lbl,v) AG_ButtonSetPadding(lbl,-1,-1,-1,(v))
void	   AG_ButtonSetFocusable(AG_Button *, int);
void	   AG_ButtonSetSticky(AG_Button *, int);
void	   AG_ButtonSetJustification(AG_Button *, enum ag_button_justify);
void	   AG_ButtonSurface(AG_Button *, SDL_Surface *);
void	   AG_ButtonSurfaceNODUP(AG_Button *, SDL_Surface *);
void	   AG_ButtonSetRepeatMode(AG_Button *, int);

void	   AG_ButtonText(AG_Button *, const char *, ...)
	     FORMAT_ATTRIBUTE(printf, 2, 3)
	     NONNULL_ATTRIBUTE(2);
void	   AG_ButtonTextNODUP(AG_Button *, char *)
	     NONNULL_ATTRIBUTE(2);

/* Legacy routines */
#define AG_ButtonSetSurface(bu,su) AG_ButtonSurface((bu),(su))
#define	AG_ButtonPrintf AG_ButtonText
#define	AG_ButtonEnable AG_WidgetEnable
#define	AG_ButtonDisable AG_WidgetDisable
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BUTTON_H_ */

/*	Public domain	*/

#ifndef _AGAR_WIDGET_BUTTON_H_
#define _AGAR_WIDGET_BUTTON_H_

#include <agar/gui/widget.h>
#include <agar/gui/label.h>

#include <agar/gui/begin.h>

typedef struct ag_button {
	struct ag_widget wid;
	int state;			/* Default state binding */
	AG_Label *lbl;			/* Text label */
	int surface;			/* Icon surface handle */
	enum ag_text_justify justify;	/* Label justification */
	enum ag_text_valign valign;	/* Vertical alignment */
	Uint flags;
#define AG_BUTTON_STICKY	0x0002	/* Toggle state */
#define AG_BUTTON_REPEAT	0x0008	/* Repeat button-pushed event */
#define AG_BUTTON_HFILL		0x0010	/* Fill available width */
#define AG_BUTTON_VFILL		0x0020	/* Fill available height */
#define AG_BUTTON_INVSTATE	0x0400	/* Invert value of "state" binding */
#define AG_BUTTON_KEYDOWN	0x0800	/* Got `key-down' before `key-up' */
#define AG_BUTTON_EXCL		0x1000
#define AG_BUTTON_NOEXCL	0x2000	/* For AG_ButtonNewFn() */
#define AG_BUTTON_EXPAND	(AG_BUTTON_HFILL|AG_BUTTON_VFILL)

	int lPad, rPad, tPad, bPad;	/* Padding in pixels */
	AG_Timer delayTo, repeatTo;	/* For AG_BUTTON_REPEAT */
} AG_Button;

__BEGIN_DECLS
extern AG_WidgetClass agButtonClass;

AG_Button *AG_ButtonNew(void *, Uint, const char *, ...)
                        FORMAT_ATTRIBUTE(printf, 3, 4);
AG_Button *AG_ButtonNewS(void *, Uint, const char *);
AG_Button *AG_ButtonNewFn(void *, Uint, const char *, AG_EventFn,
			  const char *, ...);
AG_Button *AG_ButtonNewInt(void *, Uint, const char *, int *);
AG_Button *AG_ButtonNewUint8(void *, Uint, const char *, Uint8 *);
AG_Button *AG_ButtonNewUint16(void *, Uint, const char *, Uint16 *);
AG_Button *AG_ButtonNewUint32(void *, Uint, const char *, Uint32 *);

AG_Button *AG_ButtonNewFlag(void *, Uint, const char *, Uint *, Uint);
AG_Button *AG_ButtonNewFlag8(void *, Uint, const char *, Uint8 *, Uint8);
AG_Button *AG_ButtonNewFlag16(void *, Uint, const char *, Uint16 *, Uint16);
AG_Button *AG_ButtonNewFlag32(void *, Uint, const char *, Uint32 *, Uint32);

void	   AG_ButtonSetPadding(AG_Button *, int, int, int, int);
#define	AG_ButtonSetPaddingLeft(b,v)   AG_ButtonSetPadding((b),(v),-1,-1,-1)
#define	AG_ButtonSetPaddingRight(b,v)  AG_ButtonSetPadding((b),-1,(v),-1,-1)
#define AG_ButtonSetPaddingTop(b,v)    AG_ButtonSetPadding((b),-1,-1,(v),-1)
#define	AG_ButtonSetPaddingBottom(b,v) AG_ButtonSetPadding((b),-1,-1,-1,(v))
void	   AG_ButtonSetFocusable(AG_Button *, int);
void	   AG_ButtonSetSticky(AG_Button *, int);
void	   AG_ButtonInvertState(AG_Button *, int);
void	   AG_ButtonJustify(AG_Button *, enum ag_text_justify);
void	   AG_ButtonValign(AG_Button *, enum ag_text_valign);
void	   AG_ButtonSurface(AG_Button *, const AG_Surface *);
void	   AG_ButtonSurfaceNODUP(AG_Button *, AG_Surface *);
void	   AG_ButtonSetRepeatMode(AG_Button *, int);

void	   AG_ButtonText(AG_Button *, const char *, ...)
	                 FORMAT_ATTRIBUTE(printf, 2, 3)
	                 NONNULL_ATTRIBUTE(2);
void	   AG_ButtonTextS(AG_Button *, const char *);

#ifdef AG_LEGACY
# define AG_ButtonAct AG_ButtonNewFn
# define AG_ButtonSetSurface(bu,su) AG_ButtonSurface((bu),(su))
# define AG_ButtonPrintf AG_ButtonText
# define AG_ButtonEnable AG_WidgetEnable
# define AG_ButtonDisable AG_WidgetDisable
# define AG_ButtonSetJustification AG_ButtonJustify
#endif /* AG_LEGACY */

__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_BUTTON_H_ */

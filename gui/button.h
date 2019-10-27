/*	Public domain	*/

#ifndef _AGAR_WIDGET_BUTTON_H_
#define _AGAR_WIDGET_BUTTON_H_

#include <agar/gui/widget.h>
#include <agar/gui/label.h>

#include <agar/gui/begin.h>

typedef struct ag_button {
	struct ag_widget wid;		/* AG_Widget -> AG_Button */

	int state;			/* Default state binding */
	int surface;			/* Icon surface handle */
	AG_Label *_Nullable lbl;	/* Text label */
	enum ag_text_justify justify;	/* Label justification */
	enum ag_text_valign valign;	/* Vertical alignment */
	Uint flags;
#define AG_BUTTON_STICKY	0x0002	/* Toggle state */
#define AG_BUTTON_REPEAT	0x0008	/* Repeat button-pushed (needs TIMERS) */
#define AG_BUTTON_HFILL		0x0010	/* Fill available width */
#define AG_BUTTON_VFILL		0x0020	/* Fill available height */
#define AG_BUTTON_INVSTATE	0x0400	/* Invert value of "state" binding */
#define AG_BUTTON_KEYDOWN	0x0800	/* Got `key-down' before `key-up' */
#define AG_BUTTON_EXCL		0x1000
#define AG_BUTTON_NOEXCL	0x2000	/* For AG_ButtonNewFn() */
#define AG_BUTTON_SET		0x4000	/* Initialize "state" to 1 */
#define AG_BUTTON_EXPAND	(AG_BUTTON_HFILL|AG_BUTTON_VFILL)

	int lPad, rPad, tPad, bPad;	/* Padding in pixels */
	Uint32 _pad;
	AG_Timer delayTo, repeatTo;	/* For AG_BUTTON_REPEAT */
} AG_Button;

#define AGBUTTON(obj)            ((AG_Button *)(obj))
#define AGCBUTTON(obj)           ((const AG_Button *)(obj))
#define AG_BUTTON_SELF()          AGBUTTON( AG_OBJECT(0,"AG_Widget:AG_Button:*") )
#define AG_BUTTON_PTR(n)          AGBUTTON( AG_OBJECT((n),"AG_Widget:AG_Button:*") )
#define AG_BUTTON_NAMED(n)        AGBUTTON( AG_OBJECT_NAMED((n),"AG_Widget:AG_Button:*") )
#define AG_CONST_BUTTON_SELF()   AGCBUTTON( AG_CONST_OBJECT(0,"AG_Widget:AG_Button:*") )
#define AG_CONST_BUTTON_PTR(n)   AGCBUTTON( AG_CONST_OBJECT((n),"AG_Widget:AG_Button:*") )
#define AG_CONST_BUTTON_NAMED(n) AGCBUTTON( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Button:*") )

__BEGIN_DECLS
extern AG_WidgetClass agButtonClass;

AG_Button *_Nonnull AG_ButtonNewInt(void *_Nullable, Uint,
                                    const char *_Nullable, int  *_Nonnull);
AG_Button *_Nonnull AG_ButtonNewFlag(void *_Nullable, Uint,
                                     const char *_Nullable, Uint *_Nonnull,
				     Uint);
AG_Button *_Nonnull AG_ButtonNew(void *_Nullable, Uint, const char *_Nullable, ...)
                                FORMAT_ATTRIBUTE(printf,3,4);
AG_Button *_Nonnull AG_ButtonNewFn(void *_Nullable, Uint, const char *_Nullable,
                                  _Nonnull AG_EventFn, const char *_Nullable, ...);
AG_Button *_Nonnull AG_ButtonNewS(void *_Nullable, Uint, const char *_Nullable);

void    AG_ButtonSetPadding(AG_Button *_Nonnull, int,int,int,int);
#define	AG_ButtonSetPaddingLeft(b,v)   AG_ButtonSetPadding((b),(v),-1,-1,-1)
#define	AG_ButtonSetPaddingRight(b,v)  AG_ButtonSetPadding((b),-1,(v),-1,-1)
#define AG_ButtonSetPaddingTop(b,v)    AG_ButtonSetPadding((b),-1,-1,(v),-1)
#define	AG_ButtonSetPaddingBottom(b,v) AG_ButtonSetPadding((b),-1,-1,-1,(v))

void AG_ButtonSetFocusable(AG_Button *_Nonnull, int);
void AG_ButtonSetSticky(AG_Button *_Nonnull, int);
void AG_ButtonInvertState(AG_Button *_Nonnull, int);

void AG_ButtonJustify(AG_Button *_Nonnull, enum ag_text_justify);
void AG_ButtonValign(AG_Button *_Nonnull, enum ag_text_valign);
void AG_ButtonSurface(AG_Button *_Nonnull, const AG_Surface *_Nullable);
void AG_ButtonSurfaceNODUP(AG_Button *_Nonnull, AG_Surface *_Nullable);
void AG_ButtonSetRepeatMode(AG_Button *_Nonnull, int);
void AG_ButtonText(AG_Button *_Nonnull, const char *_Nonnull, ...)
                  FORMAT_ATTRIBUTE(printf,2,3);
void AG_ButtonTextS(AG_Button *_Nonnull, const char *_Nullable);

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

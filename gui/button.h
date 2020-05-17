/*	Public domain	*/

#ifndef _AGAR_WIDGET_BUTTON_H_
#define _AGAR_WIDGET_BUTTON_H_

#include <agar/gui/widget.h>
#include <agar/gui/label.h>

#include <agar/gui/begin.h>

/* Context for REPEAT feature */
typedef struct ag_button_repeat {
	AG_Timer delayTo;               /* Delay until key repeat */
	AG_Timer ivalTo;                /* Interval between repetitions */
} AG_ButtonRepeat;

typedef struct ag_button {
	struct ag_widget wid;           /* AG_Widget -> AG_Button */

	Uint flags;
#define AG_BUTTON_NO_FOCUS 0x0001       /* Make non-focusable */
#define AG_BUTTON_STICKY   0x0002       /* Toggle state */
#define AG_BUTTON_PRESSING 0x0004       /* Button press in progress */
#define AG_BUTTON_REPEAT   0x0008       /* Trigger "button-pushed" repeatedly */
#define AG_BUTTON_HFILL    0x0010       /* Fill available width */
#define AG_BUTTON_VFILL    0x0020       /* Fill available height */
#define AG_BUTTON_INVERTED 0x0400       /* Invert value of "state" binding */
#define AG_BUTTON_KEYDOWN  0x0800       /* Got `key-down' before `key-up' */
#define AG_BUTTON_EXCL     0x1000	/* Hint exclusive access to "state" */
#define AG_BUTTON_NOEXCL   0x2000       /* For AG_ButtonNewFn() */
#define AG_BUTTON_SET      0x4000       /* Initialize "state" to 1 */
#define AG_BUTTON_SLOW     0x8000
#define AG_BUTTON_EXPAND  (AG_BUTTON_HFILL | AG_BUTTON_VFILL)

	int state;                      /* Default state binding */
	int surfaceLbl;                 /* Rendered text label (or -1) */
	int surfaceSrc;			/* Specified graphical surface (or -1) */
	enum ag_text_justify justify;   /* Horizontal alignment mode */
	enum ag_text_valign valign;     /* Vertical alignment mode */
	int wReq, hReq;                 /* Size requisition */
	char *_Nullable label;          /* Optional text label */
	AG_ButtonRepeat *_Nullable repeat; /* Context for REPEAT mode */
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
#define             AG_ButtonNewUint(bu,fl,s,p) \
                    AG_ButtonNewInt((bu),(fl),(s),(int *)(p))

AG_Button *_Nonnull AG_ButtonNewFlag(void *_Nullable, Uint, const char *_Nullable,
                                     Uint *_Nonnull, Uint);

AG_Button *_Nonnull AG_ButtonNew(void *_Nullable, Uint, const char *_Nullable, ...)
                                FORMAT_ATTRIBUTE(printf,3,4);

AG_Button *_Nonnull AG_ButtonNewFn(void *_Nullable, Uint, const char *_Nullable,
                                  _Nonnull AG_EventFn, const char *_Nullable, ...);

AG_Button *_Nonnull AG_ButtonNewS(void *_Nullable, Uint, const char *_Nullable);

void AG_ButtonSetFocusable(AG_Button *_Nonnull, int);
void AG_ButtonSetSticky(AG_Button *_Nonnull, int);
void AG_ButtonSetInverted(AG_Button *_Nonnull, int);

int  AG_ButtonGetState(AG_Button *_Nonnull);
int  AG_ButtonSetState(AG_Button *_Nonnull, int);
int  AG_ButtonToggle(AG_Button *_Nonnull);

void AG_ButtonJustify(AG_Button *_Nonnull, enum ag_text_justify);
void AG_ButtonValign(AG_Button *_Nonnull, enum ag_text_valign);
void AG_ButtonSurface(AG_Button *_Nonnull, const AG_Surface *_Nullable);
void AG_ButtonSurfaceNODUP(AG_Button *_Nonnull, AG_Surface *_Nullable);
void AG_ButtonSetRepeatMode(AG_Button *_Nonnull, int);
void AG_ButtonText(AG_Button *_Nonnull, const char *_Nonnull, ...)
                  FORMAT_ATTRIBUTE(printf,2,3);
void AG_ButtonTextS(AG_Button *_Nonnull, const char *_Nullable);

#ifdef AG_LEGACY
# define AG_BUTTON_INVSTATE               AG_BUTTON_INVERTED
# define AG_ButtonAct                     AG_ButtonNewFn
# define AG_ButtonDisable                 AG_WidgetDisable
# define AG_ButtonEnable                  AG_WidgetEnable
# define AG_ButtonInvertState             AG_ButtonSetInverted
# define AG_ButtonPrintf                  AG_ButtonText
# define AG_ButtonSetJustification        AG_ButtonJustify
# define AG_ButtonSetPadding(bu,l,r,t,b)  AG_SetStyleF((bu), "padding", "%d %d %d %d", (t),(r),(b),(l))
# define AG_ButtonSetPaddingLeft(bu,v)    AG_SetStyleF((bu), "padding", "%d 0 0 0", (v))
# define AG_ButtonSetPaddingRight(bu,v)   AG_SetStyleF((bu), "padding", "0 %d 0 0", (v))
# define AG_ButtonSetPaddingTop(bu,v)     AG_SetStyleF((bu), "padding", "0 0 %d 0", (v))
# define AG_ButtonSetPaddingBottom(bu,v)  AG_SetStyleF((bu), "padding", "0 0 0 %d", (v))
# define AG_ButtonSetSurface(bu,su)       AG_ButtonSurface((bu),(su))
#endif /* AG_LEGACY */
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_BUTTON_H_ */

/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXTBOX_H_
#define _AGAR_WIDGET_TEXTBOX_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/scrollbar.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/scrollbar.h>
#endif

#include "begin_code.h"

#define AG_TEXTBOX_STRING_MAX 1024

typedef struct ag_textbox {
	struct ag_widget wid;
	
	char string[AG_TEXTBOX_STRING_MAX];	/* Default string binding */

	AG_Mutex     lock;
	char	    *labelText;			/* label text */
	int	     label;			/* Label surface mapping */

	Uint flags;
#define AG_TEXTBOX_MULTILINE	 0x0001
#define AG_TEXTBOX_BLINK_ON	 0x0002	/* Cursor blink state (internal) */
#define AG_TEXTBOX_PASSWORD	 0x0004	/* Password (hidden) input */
#define AG_TEXTBOX_ABANDON_FOCUS 0x0008	/* Abandon focus on return */
#define AG_TEXTBOX_COMBO	 0x0010	/* Used by AG_Combo */
#define AG_TEXTBOX_HFILL	 0x0020
#define AG_TEXTBOX_VFILL	 0x0040
#define AG_TEXTBOX_EXPAND	 (AG_TEXTBOX_HFILL|AG_TEXTBOX_VFILL)
#define AG_TEXTBOX_FOCUS	 0x0080
#define AG_TEXTBOX_READONLY	 0x0100	/* Equivalent to WidgetDisable() */
#define AG_TEXTBOX_INT_ONLY	 0x0200	/* Accepts only valid strtol() input */
#define AG_TEXTBOX_FLT_ONLY	 0x0400	/* Accepts only valid strtof() input */
#define AG_TEXTBOX_CATCH_TAB	 0x0800	/* Enter literal tabs into text
					   instead of cycling focus */
#define AG_TEXTBOX_CURSOR_MOVING 0x1000	/* Cursor is being moved */

	int wPre, hPre;			/* Prescale */
	int boxPadX, boxPadY;		/* Padding around textbox */
	int lblPadL, lblPadR;		/* Padding around label */
	int wLbl;			/* Label width to display */
	int pos;			/* Cursor position */
	int offs;			/* Display offset */
	int compose;			/* Key for input composition */

	int sel_x1, sel_x2;		/* Selection points */
	int sel_edit;			/* Point being edited */

	AG_Timeout delay_to;		/* Pre-repeat delay timer */
	AG_Timeout repeat_to;		/* Repeat timer */
	AG_Timeout cblink_to;		/* Cursor blink timer */
	AG_Scrollbar *hBar, *vBar;	/* Scrollbars for MULTILINE */
	int xMin, x, xMax;		/* Horizontal scrollbar range */
	int yMin, y, yMax, yVis;	/* Vertical scrollbar range */

	struct {
		SDLKey key;		/* For key repeat */
		SDLMod mod;
		Uint32 unicode;
	} repeat;
} AG_Textbox;

__BEGIN_DECLS
extern const AG_WidgetOps agTextboxOps;

AG_Textbox *AG_TextboxNew(void *, Uint, const char *);
void	    AG_TextboxInit(AG_Textbox *, Uint, const char *);
void	    AG_TextboxPrescale(AG_Textbox *, const char *);

void	 AG_TextboxSetPassword(AG_Textbox *, int);
void	 AG_TextboxSetLabel(AG_Textbox *, const char *, ...);

void	 AG_TextboxPrintf(AG_Textbox *, const char *, ...);
char	*AG_TextboxDupString(AG_Textbox *);
size_t	 AG_TextboxCopyString(AG_Textbox *, char *, size_t)
	                      BOUNDED_ATTRIBUTE(__string__, 2, 3);
int	 AG_TextboxInt(AG_Textbox *);

/* Legacy */
#define AG_TextboxSetWriteable(tb,flag)	do {	\
	if (flag) {				\
	 	AG_WidgetEnable(tb);		\
	} else {				\
		AG_WidgetDisable(tb);		\
	}					\
} while (0)

__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TEXTBOX_H_ */

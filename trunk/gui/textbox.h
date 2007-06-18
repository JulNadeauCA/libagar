/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXTBOX_H_
#define _AGAR_WIDGET_TEXTBOX_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

#define AG_TEXTBOX_STRING_MAX 1024

typedef struct ag_textbox {
	struct ag_widget wid;
	
	char string[AG_TEXTBOX_STRING_MAX];	/* Default string binding */

	SDL_Surface *label_su;			/* Label surface */
	int	     label_id;			/* Label surface mapping */

	Uint flags;
#define AG_TEXTBOX_WRITEABLE	 0x001	/* Allow focus/text input */
#define AG_TEXTBOX_BLINK_ON	 0x002	/* Cursor blink state (internal) */
#define AG_TEXTBOX_PASSWORD	 0x004	/* Password (hidden) input */
#define AG_TEXTBOX_ABANDON_FOCUS 0x008	/* Abandon focus on return */
#define AG_TEXTBOX_COMBO	 0x010	/* Used by AG_Combo */
#define AG_TEXTBOX_HFILL	 0x020
#define AG_TEXTBOX_VFILL	 0x040
#define AG_TEXTBOX_EXPAND	 (AG_TEXTBOX_HFILL|AG_TEXTBOX_VFILL)
#define AG_TEXTBOX_FOCUS	 0x080
#define AG_TEXTBOX_READONLY	 0x100	/* Text is read-only */
#define AG_TEXTBOX_INT_ONLY	 0x200	/* Accepts only valid strtol() input */
#define AG_TEXTBOX_FLT_ONLY	 0x400	/* Accepts only valid strtof() input */

	int prew, preh;			/* Prescale */
	int xpadding, ypadding;		/* Text padding */
	int pos;			/* Cursor position */
	int offs;			/* Display offset */
	int compose;			/* Key for input composition */

	int sel_x1, sel_x2;		/* Selection points */
	int sel_edit;			/* Point being edited */

	AG_Timeout delay_to;		/* Pre-repeat delay timer */
	AG_Timeout repeat_to;		/* Repeat timer */
	AG_Timeout cblink_to;		/* Cursor blink timer */

	struct {
		SDLKey key;
		SDLMod mod;
		Uint32 unicode;
	} repeat;
} AG_Textbox;

__BEGIN_DECLS
AG_Textbox *AG_TextboxNew(void *, Uint, const char *);
void	    AG_TextboxInit(AG_Textbox *, Uint, const char *);
void	    AG_TextboxDestroy(void *);
void	    AG_TextboxDraw(void *);
void	    AG_TextboxPrescale(AG_Textbox *, const char *);
void	    AG_TextboxScale(void *, int, int);

void	 AG_TextboxPrintf(AG_Textbox *, const char *, ...);
char	*AG_TextboxDupString(AG_Textbox *);
size_t	 AG_TextboxCopyString(AG_Textbox *, char *, size_t)
	                      BOUNDED_ATTRIBUTE(__string__, 2, 3);
int	 AG_TextboxInt(AG_Textbox *);
void	 AG_TextboxSetWriteable(AG_Textbox *, int);
void	 AG_TextboxSetPassword(AG_Textbox *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TEXTBOX_H_ */

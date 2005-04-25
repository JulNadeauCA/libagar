/*	$Csoft: textbox.h,v 1.35 2005/02/08 15:45:38 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXTBOX_H_
#define _AGAR_WIDGET_TEXTBOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define TEXTBOX_STRING_MAX 1024

struct textbox {
	struct widget wid;
	
	char string[TEXTBOX_STRING_MAX]; /* Default string binding */
	SDL_Surface *label;		 /* Label to display */
	u_int flags;
#define TEXTBOX_WRITEABLE	0x01	/* Writeable */
#define TEXTBOX_COMPOSE		0x02	/* Input composition */
#define TEXTBOX_BLINK_ON	0x04	/* Cursor blink state */
#define TEXTBOX_PASSWORD	0x08	/* Password (hidden) input */

	int prew, preh;			/* Prescale */
	int xpadding, ypadding;		/* Text padding */
	int pos;			/* Cursor position */
	int offs;			/* Display offset */

	int sel_x1, sel_x2;		/* Selection points */
	int sel_edit;			/* Point being edited */

	struct timeout delay_to;	/* Pre-repeat delay timer */
	struct timeout repeat_to;	/* Repeat timer */
	struct timeout cblink_to;	/* Cursor blink timer */
	struct {
		SDLKey key;
		SDLMod mod;
		Uint32 unicode;
	} repeat;
};

__BEGIN_DECLS
struct textbox	*textbox_new(void *, const char *);

void	 textbox_init(struct textbox *, const char *);
void	 textbox_destroy(void *);
void	 textbox_draw(void *);
void	 textbox_prescale(struct textbox *, const char *);
void	 textbox_scale(void *, int, int);

void	 textbox_shown(int, union evarg *);
void	 textbox_hidden(int, union evarg *);
void	 textbox_printf(struct textbox *, const char *, ...)
	     FORMAT_ATTRIBUTE(printf, 2, 3);
char	*textbox_string(struct textbox *);
size_t	 textbox_copy_string(struct textbox *, char *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 3);
int	 textbox_int(struct textbox *);
void	 textbox_set_writeable(struct textbox *, int);
void	 textbox_set_password(struct textbox *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TEXTBOX_H_ */

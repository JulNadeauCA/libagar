/*	$Csoft: textbox.h,v 1.32 2004/03/12 02:48:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXTBOX_H_
#define _AGAR_WIDGET_TEXTBOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define TEXTBOX_STRING_MAX 1024

struct textbox {
	struct widget wid;
	
	char	string[TEXTBOX_STRING_MAX];	/* UTF-8 text buffer */
	Uint32	compose;			/* For input composition */
	SDL_Surface *label;			/* Label to display */

	int	 prew, preh;			/* Prescale */
	int	 writeable;			/* Read/write? */
	int	 xpadding, ypadding;		/* Text padding */
	int	 newx;				/* Mouse seek */
	int	 pos;				/* Position in text */
	int	 offs;				/* Text display offset */
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
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TEXTBOX_H_ */

/*	$Csoft: textbox.h,v 1.21 2003/06/15 05:08:43 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXTBOX_H_
#define _AGAR_WIDGET_TEXTBOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define TEXTBOX_STRING_MAX	128

struct textbox {
	struct widget wid;
	
	SDL_Surface	*label;			/* Label (left) */

	char	 string[TEXTBOX_STRING_MAX];	/* Default string binding */
	int	 writeable;			/* Read/write? */
	int	 xpadding, ypadding;		/* Text padding */
	int	 newx;				/* Mouse seek */
	
	int	 pos;				/* Position in text */
	int	 offs;				/* Text display offset */
};

__BEGIN_DECLS
extern DECLSPEC struct textbox	*textbox_new(void *, const char *);

extern DECLSPEC void	 textbox_init(struct textbox *, const char *);
extern DECLSPEC void	 textbox_destroy(void *);
extern DECLSPEC void	 textbox_draw(void *);
extern DECLSPEC void	 textbox_scale(void *, int, int);

extern DECLSPEC void	 textbox_shown(int, union evarg *);
extern DECLSPEC void	 textbox_hidden(int, union evarg *);
extern DECLSPEC void	 textbox_printf(struct textbox *, const char *, ...);
extern DECLSPEC char	*textbox_string(struct textbox *);
extern DECLSPEC Uint16	*textbox_unicode(struct textbox *);
extern DECLSPEC size_t	 textbox_copy_string(struct textbox *, char *, size_t);
extern DECLSPEC size_t	 textbox_copy_unicode(struct textbox *, Uint16 *,
			                      size_t);
extern DECLSPEC int	 textbox_int(struct textbox *);
extern DECLSPEC void	 textbox_set_writeable(struct textbox *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TEXTBOX_H_ */

/*	$Csoft: textbox.h,v 1.19 2003/06/06 03:18:14 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXTBOX_H_
#define _AGAR_WIDGET_TEXTBOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define TEXTBOX_DEFAULT_MAX	128

struct textbox {
	struct widget wid;
	
	SDL_Surface	*label;			/* Label (left) */

	char	 string[TEXTBOX_DEFAULT_MAX];	/* Def string binding */
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
extern DECLSPEC size_t	 textbox_copy_string(struct textbox *, char *, size_t);
extern DECLSPEC int	 textbox_int(struct textbox *);
extern DECLSPEC void	 textbox_set_writeable(struct textbox *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TEXTBOX_H_ */

/*	$Csoft: textbox.h,v 1.17 2003/05/24 00:29:00 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXTBOX_H_
#define _AGAR_WIDGET_TEXTBOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define TEXTBOX_DEFAULT_MAX	128

struct textbox {
	struct widget	 wid;
	int		 writeable;			/* Read/write? */
	int		 xpadding, ypadding;
	int		 newx;
	SDL_Surface	*label;
	struct {
		int	 pos;
		int	 offs;
		pthread_mutex_t lock;
	} text;
	struct {
		char	 string[TEXTBOX_DEFAULT_MAX];
	} def;
};

__BEGIN_DECLS
extern DECLSPEC struct textbox	*textbox_new(struct region *, const char *);
extern DECLSPEC void		 textbox_init(struct textbox *, const char *);
extern DECLSPEC void		 textbox_destroy(void *);

extern DECLSPEC void	 textbox_shown(int, union evarg *);
extern DECLSPEC void	 textbox_hidden(int, union evarg *);
extern DECLSPEC void	 textbox_draw(void *);
extern DECLSPEC void	 textbox_printf(struct textbox *, const char *, ...);
extern DECLSPEC char	*textbox_string(struct textbox *);
extern DECLSPEC size_t	 textbox_copy_string(struct textbox *, char *, size_t);
extern DECLSPEC int	 textbox_int(struct textbox *);
extern DECLSPEC void	 textbox_set_writeable(struct textbox *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TEXTBOX_H_ */

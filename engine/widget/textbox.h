/*	$Csoft: textbox.h,v 1.15 2003/03/22 04:22:45 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXTBOX_H_
#define _AGAR_WIDGET_TEXTBOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

struct textbox {
	struct widget	 wid;
	int		 flags;
#define TEXTBOX_READONLY	0x01	/* Text cannot be modified */
	int		 xmargin, ymargin;
	int		 newx;
	SDL_Surface	*label;
	struct {
		char	*s;
		int	 pos;
		int	 offs;
		pthread_mutex_t lock;
	} text;
};

__BEGIN_DECLS
extern DECLSPEC struct textbox	*textbox_new(struct region *, const char *, int,
				             int, int);
extern DECLSPEC void		 textbox_init(struct textbox *, const char *,
				              int, int, int);
extern DECLSPEC void		 textbox_destroy(void *);

extern DECLSPEC void	 textbox_shown(int, union evarg *);
extern DECLSPEC void	 textbox_hidden(int, union evarg *);
extern DECLSPEC void	 textbox_draw(void *);
extern DECLSPEC void	 textbox_printf(struct textbox *, const char *, ...);
extern DECLSPEC char	*textbox_string(struct textbox *);
extern DECLSPEC size_t	 textbox_copy_string(struct textbox *, char *, size_t);
extern DECLSPEC int	 textbox_int(struct textbox *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TEXTBOX_H_ */

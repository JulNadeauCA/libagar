/*	$Csoft: textbox.h,v 1.13 2003/01/23 01:53:37 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXTBOX_H_
#define _AGAR_WIDGET_TEXTBOX_H_

#include <engine/widget/widget.h>

struct textbox {
	struct widget	wid;

	int	 flags;
#define TEXTBOX_READONLY	0x01	/* Text cannot be modified */

	int	 xmargin, ymargin;
	int	 newx;

	SDL_Surface	*label;
	
	struct {
		char	*s;
		int	 pos;
		int	 offs;
		pthread_mutex_t lock;
	} text;
};

struct textbox	*textbox_new(struct region *, const char *, int, int, int);
void		 textbox_init(struct textbox *, const char *, int, int, int);
void		 textbox_destroy(void *);

void	 textbox_shown(int, union evarg *);
void	 textbox_hidden(int, union evarg *);
void	 textbox_draw(void *);
void	 textbox_printf(struct textbox *, const char *, ...);
char	*textbox_string(struct textbox *);
int	 textbox_int(struct textbox *);

#endif /* _AGAR_WIDGET_TEXTBOX_H_ */

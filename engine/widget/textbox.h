/*	$Csoft: textbox.h,v 1.11 2002/09/07 04:34:14 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXTBOX_H_
#define _AGAR_WIDGET_TEXTBOX_H_

struct textbox {
	struct	 widget wid;

	int	 flags;
#define TEXTBOX_READONLY	0x01	/* Read-only */
	
	int	 xmargin;
	int	 ymargin;
	int	 newx;

	char	*label;
	SDL_Surface *label_s;
	
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

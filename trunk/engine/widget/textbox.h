/*	$Csoft: textbox.h,v 1.9 2002/08/21 23:52:03 vedge Exp $	*/
/*	Public domain	*/

struct textbox {
	struct	 widget wid;

	int	 flags;
#define TEXTBOX_READONLY	0x01	/* Read-only */

	char	*label;
	char	*text;
	int	 textpos;
	int	 textoffs;
	int	 xmargin, ymargin;
	int	 newx;

	SDL_Surface	*label_s;
};

struct textbox	*textbox_new(struct region *, const char *, int, int, int);
void		 textbox_init(struct textbox *, const char *, int, int, int);
void		 textbox_destroy(void *);

void		 textbox_shown(int argc, union evarg *argv);
void		 textbox_hidden(int argc, union evarg *argv);
void		 textbox_draw(void *);
void		 textbox_printf(struct textbox *te, const char *fmt, ...);


/*	$Csoft: textbox.h,v 1.7 2002/06/09 10:08:08 vedge Exp $	*/
/*	Public domain	*/

struct textbox {
	struct	 widget wid;

	int	 flags;
#define TEXTBOX_READONLY	0x01	/* Read-only */
#define TEXTBOX_CURSOR		0x02	/* Cursor visible */

	char	*label;
	char	*text;
	int	 textpos;
	int	 textoffs;
	int	 xmargin, ymargin;
	
	SDL_Surface	*label_s;
};

struct textbox	*textbox_new(struct region *, const char *, int, int, int);
void		 textbox_init(struct textbox *, const char *, int, int, int);
void		 textbox_destroy(void *);

void		 textbox_shown(int argc, union evarg *argv);
void		 textbox_hidden(int argc, union evarg *argv);
void		 textbox_draw(void *);
void		 textbox_printf(struct textbox *te, char *fmt, ...);


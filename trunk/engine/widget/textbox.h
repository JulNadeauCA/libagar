/*	$Csoft: textbox.h,v 1.3 2002/05/28 06:01:18 vedge Exp $	*/

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

	void	(*typed)(struct textbox *, char);
};

struct textbox	*textbox_new(struct region *, const char *, int, int);
void		 textbox_init(struct textbox *, const char *, int, int);
void		 textbox_destroy(void *);
void		 textbox_onattach(void *, void *);
void		 textbox_ondetach(void *, void *);

void	 textbox_draw(void *);
void	 textbox_event(void *, SDL_Event *, int);


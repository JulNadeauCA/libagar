/*	$Csoft: textbox.h,v 1.2 2002/05/25 08:48:36 vedge Exp $	*/

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

	void	(*typed)(struct textbox *, char);
};

struct textbox	*textbox_new(struct region *, char *, int, int);
void		 textbox_init(struct textbox *, char *, int, int);
void		 textbox_destroy(void *);
void		 textbox_onattach(void *, void *);
void		 textbox_ondetach(void *, void *);

void	 textbox_draw(void *);
void	 textbox_event(void *, SDL_Event *, int);


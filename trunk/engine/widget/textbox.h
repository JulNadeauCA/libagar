/*	$Csoft$	*/

struct textbox {
	struct	 widget wid;

	int	 flags;
#define TEXTBOX_READONLY	0x01	/* Read-only */
#define TEXTBOX_CURSOR		0x02	/* Cursor visible */

	char	*text;
	int	 textpos;
	int	 xmargin, ymargin;

	void	(*typed)(struct textbox *, char);
};

struct textbox	*textbox_new(struct region *, int, int, int);
void		 textbox_init(struct textbox *, int, int, int);
void		 textbox_destroy(void *);

void	 textbox_draw(void *);
void	 textbox_event(void *, SDL_Event *, int);


/*	$Csoft: text.h,v 1.8 2002/04/09 02:34:54 vedge Exp $	*/

#ifndef _AGAR_TEXT_TEXT_H_
#define _AGAR_TEXT_TEXT_H_

#include <SDL_ttf.h>

struct text {
	struct	 object obj;

	Uint32	 flags;
#define TEXT_WAITKEY		0x01	/* Destroy on key press */
#define TEXT_SLEEP		0x02	/* Destroy after timer has expired */
#define TEXT_TRANSPARENT	0x04	/* Don't draw a background */
#define TEXT_DEBUG		0x08	/* Show only in debug mode */

	Sint32	 sleepms;	/* Sleep delay in ms */
	Uint32	 w, h;		/* Window geometry */
	Uint32	 x, y;		/* Window position */
	Uint32	 tx, ty;	/* Current text offset */
	Uint32	 nlines;	/* Lines in text window */
	SDL_Rect mvmask;	/* Map view mask */

	struct viewport *view;
	SDL_Color *fgcolor;
	Uint32	 bgcolor;

	SDL_Surface *surface;
	SDL_Surface *v;

	TAILQ_ENTRY(text) texts;	/* Texts to display */
};

int		 text_init(void);
void		 text_quit(void);
struct text	*text_create(Uint32, Uint32, Uint32, Uint32, Uint32, Uint32);
int		 text_destroy(void *);
int		 text_link(void *);
int		 text_unlink(void *);

void		 text_clear(struct text *);
void		 text_render(struct text *, char *);
void		 text_drawall(void);
void		 text_destroyall(void);

void		 text_printf(struct text *, char *, ...);
void		 text_msg(Uint32, Uint32, char *fmt, ...);

#endif	/* _AGAR_TEXT_TEXT_H_ */

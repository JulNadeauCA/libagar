/*	$Csoft: text.h,v 1.4 2002/04/30 08:18:34 vedge Exp $	*/

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

	Uint16	 sleepms;	/* Sleep delay in ms */
	Uint32	 w, h;		/* Window geometry */
	Uint32	 x, y;		/* Window position */
	Uint32	 tx, ty;	/* Current text offset */
	Uint32	 nlines;	/* Lines in text window */
	SDL_Rect mvmask;	/* Map view mask */

	struct viewport *view;
	SDL_Color *fgcolor;
	Uint32	 bgcolor;

	SDL_Surface *surface;		/* TTF surface */
	SDL_Surface *v;			/* XXX destination view */

	TAILQ_ENTRY(text) texts;	/* Texts to display */
};

extern TTF_Font *font;

int	 text_engine_init(void);
void	 text_engine_destroy(void);
void	 text_init(struct text *, Sint16, Sint16, Uint16, Uint16, Uint32,
	     Uint8);
void	 text_destroy(void *);
void	 text_onattach(void *, void *);
void	 text_ondetach(void *, void *);

void	 text_clear(struct text *);
void	 text_render(struct text *, char *);
void	 text_drawall(void);
void	 text_destroyall(void);

void	 text_printf(struct text *, char *, ...);
void	 text_msg(Uint8, Uint32, char *, ...);


#endif	/* _AGAR_TEXT_TEXT_H_ */

/*	$Csoft: text.h,v 1.14 2003/01/05 08:41:43 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXT_H_
#define _AGAR_WIDGET_TEXT_H_

#include <engine/media/ttf.h>

extern ttf_font *font;

int		 text_init(void);
void		 text_destroy(void);
void		 text_set_default_font(char *, int, int);

SDL_Surface	*text_render(char *, int, Uint32, char *);
void		 text_msg(const char *, const char *, ...);

#endif	/* _AGAR_WIDGET_TEXT_H_ */

/*	$Csoft: text.h,v 1.15 2003/03/02 04:13:15 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXT_H_
#define _AGAR_WIDGET_TEXT_H_

#include <engine/media/ttf.h>

extern ttf_font *font;

int		 text_init(void);
void		 text_destroy(void);
void		 text_set_default_font(char *, int, int);
int		 text_font_height(ttf_font *);
int		 text_font_ascent(ttf_font *);
int		 text_font_descent(ttf_font *);
int		 text_font_line_skip(ttf_font *);

SDL_Surface	*text_render(char *, int, Uint32, char *);
void		 text_msg(const char *, const char *, ...);

#endif	/* _AGAR_WIDGET_TEXT_H_ */

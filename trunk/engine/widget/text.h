/*	$Csoft: text.h,v 1.13 2002/12/21 10:26:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXT_H_
#define _AGAR_WIDGET_TEXT_H_

#include <engine/ttf.h>

extern TTF_Font *font;
extern int font_h;

int		 text_engine_init(void);
void		 text_engine_destroy(void);
SDL_Surface	*text_render(char *, int, Uint32, char *);
void		 text_msg(const char *, const char *, ...);

#endif	/* _AGAR_WIDGET_TEXT_H_ */

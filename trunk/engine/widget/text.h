/*	$Csoft: text.h,v 1.11 2002/08/22 23:04:36 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_TEXT_TEXT_H_
#define _AGAR_TEXT_TEXT_H_

#include <engine/ttf.h>

extern TTF_Font *font;
extern int font_h;

int		 text_engine_init(void);
void		 text_engine_destroy(void);
SDL_Surface	*text_render(char *, int, Uint32, char *);
void		 text_msg(char *, char *, ...);

#endif	/* _AGAR_TEXT_TEXT_H_ */

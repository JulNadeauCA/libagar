/*	$Csoft: text.h,v 1.18 2003/06/06 03:18:14 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXT_H_
#define _AGAR_WIDGET_TEXT_H_

#include <engine/media/ttf.h>

#include "begin_code.h"

extern ttf_font *font;

__BEGIN_DECLS
extern DECLSPEC int		 text_init(void);
extern DECLSPEC void		 text_destroy(void);
extern DECLSPEC void		 text_set_default_font(char *, int, int);
extern DECLSPEC int		 text_font_height(ttf_font *);
extern DECLSPEC int		 text_font_ascent(ttf_font *);
extern DECLSPEC int		 text_font_descent(ttf_font *);
extern DECLSPEC int		 text_font_line_skip(ttf_font *);
extern DECLSPEC SDL_Surface	*text_render(const char *, int, Uint32,
				             const char *);
extern DECLSPEC SDL_Surface	*text_render_glyph(const char *, int, Uint32,
				                   char);
extern DECLSPEC void		 text_prescale(const char *, int *, int *);
extern DECLSPEC void		 text_msg(const char *, const char *, ...);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_TEXT_H_ */

/*	$Csoft: text.h,v 1.25 2003/08/31 11:58:10 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXT_H_
#define _AGAR_WIDGET_TEXT_H_

#include <engine/loader/ttf.h>

#include "begin_code.h"

extern ttf_font *font;

enum text_msg_title {
	MSG_ERROR,
	MSG_WARNING,
	MSG_INFO
};

enum text_engine {
	TEXT_ENGINE_FREETYPE,	/* Use FreeType */
	TEXT_ENGINE_BITMAP	/* Use bitmap fonts */
};

__BEGIN_DECLS
int	 text_init(void);
void	 text_destroy(void);
void	 text_set_default_font(char *, int, int);
int	 text_font_height(ttf_font *);
int	 text_font_ascent(ttf_font *);
int	 text_font_descent(ttf_font *);
int	 text_font_line_skip(ttf_font *);

__inline__ SDL_Surface	*text_render(const char *, int, Uint32, const char *);
SDL_Surface		*text_render_unicode(const char *, int, Uint32,
			                     const Uint32 *);
__inline__ SDL_Surface	*text_render_glyph(const char *, int, Uint32, Uint32);
void			 text_prescale_unicode(const Uint32 *, int *, int *);
__inline__ void		 text_prescale(const char *, int *, int *);
void			 text_msg(enum text_msg_title, const char *, ...)
			     FORMAT_ATTRIBUTE(printf, 2, 3)
			     NONNULL_ATTRIBUTE(2);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_TEXT_H_ */

/*	$Csoft: text.h,v 1.19 2003/06/13 01:45:14 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXT_H_
#define _AGAR_WIDGET_TEXT_H_

#include <engine/media/ttf.h>

#include "begin_code.h"

extern ttf_font *font;

enum text_engine {
	TEXT_ENGINE_FREETYPE,	/* Use FreeType */
	TEXT_ENGINE_BITMAP	/* Use bitmap fonts */
};

__BEGIN_DECLS
extern DECLSPEC int		 text_init(void);
extern DECLSPEC void		 text_destroy(void);
extern DECLSPEC void		 text_set_default_font(char *, int, int);
extern DECLSPEC int		 text_font_height(ttf_font *);
extern DECLSPEC int		 text_font_ascent(ttf_font *);
extern DECLSPEC int		 text_font_descent(ttf_font *);
extern DECLSPEC int		 text_font_line_skip(ttf_font *);
extern __inline__ SDL_Surface	*text_render(const char *, int, Uint32,
				             const char *);
extern __inline__ SDL_Surface	*text_render_utf8(const char *, int, Uint32,
				                  const char *);
extern DECLSPEC SDL_Surface	*text_render_unicode(const char *, int, Uint32,
				                     const Uint16 *);
extern __inline__ SDL_Surface	*text_render_glyph(const char *, int, Uint32,
				                   Uint16);
extern DECLSPEC void	 text_prescale_unicode(const Uint16 *, int *, int *);
extern __inline__ void	 text_prescale(const char *, int *, int *);

extern DECLSPEC void	 text_msg(const char *, const char *, ...);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_TEXT_H_ */

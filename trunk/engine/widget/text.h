/*	$Csoft: text.h,v 1.30 2004/04/26 03:21:19 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXT_H_
#define _AGAR_WIDGET_TEXT_H_

#include <engine/loader/ttf.h>

#include "begin_code.h"

enum text_msg_title {
	MSG_ERROR,
	MSG_WARNING,
	MSG_INFO
};

enum text_engine {
	TEXT_ENGINE_TTF,		/* FreeType */
	TEXT_ENGINE_BITMAP,		/* Fixed-size bitmap */
	TEXT_ENGINE_PIXMAP		/* Fixed-size pixmap */
};

/* Cached text surface. */
struct text {
	char fontname[32];		/* Font name */
	int fontsize;			/* Font size in points */
	Uint32 color;			/* Text color */
	char *s;			/* Text string */
	Uint32 nrefs;			/* Reference count */
	SDL_Surface *su;		/* Rendered surface */
	SLIST_ENTRY(text) texts;
};

/* Font engine independent representation of a font style. */
struct text_font {
	char	 name[32];
	int	 size;
	int	 style;
	void	*p;
	SLIST_ENTRY(text_font) fonts;
};

__BEGIN_DECLS
int	 text_init(enum text_engine);
void	 text_parse_fontspec(char *);
void	 text_destroy(void);
int	 text_font_height(struct text_font *);
int	 text_font_ascent(struct text_font *);
int	 text_font_descent(struct text_font *);
int	 text_font_line_skip(struct text_font *);

__inline__ SDL_Surface	*text_render(const char *, int, Uint32, const char *);
SDL_Surface		*text_render_unicode(const char *, int, Uint32,
			                     const Uint32 *);
void			 text_prescale_unicode(const Uint32 *, int *, int *);
__inline__ void		 text_prescale(const char *, int *, int *);
void			 text_msg(enum text_msg_title, const char *, ...)
			     FORMAT_ATTRIBUTE(printf, 2, 3)
			     NONNULL_ATTRIBUTE(2);

struct text	*text_render2(const char *, int, Uint32, const char *);
void		 text_unused2(struct text *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_TEXT_H_ */

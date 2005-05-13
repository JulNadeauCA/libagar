/*	$Csoft: text.h,v 1.40 2005/05/10 12:25:54 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXT_H_
#define _AGAR_WIDGET_TEXT_H_

#include "begin_code.h"

enum text_msg_title {
	MSG_ERROR,
	MSG_WARNING,
	MSG_INFO
};

/* Cached glyph surface/texture information. */
struct text_glyph {
	char fontname[32];		/* Font name */
	int fontsize;			/* Font size in points */
	Uint32 color;			/* Glyph color */
	Uint32 ch;			/* Unicode character */
	Uint32 nrefs;			/* Reference count */
	SDL_Surface *su;		/* Rendered surface */
#ifdef HAVE_OPENGL
	GLuint texture;			/* Rendered texture */
	GLfloat texcoord[4];
#endif
	SLIST_ENTRY(text_glyph) glyphs;
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
extern int text_font_height, text_font_ascent, text_font_descent,
	   text_font_line_skip, text_tab_width, text_blink_rate;

int	 text_init(void);
void	 text_parse_fontspec(char *);
void	 text_destroy(void);

struct text_font	*text_fetch_font(const char *, int, int);
__inline__ SDL_Surface	*text_render(const char *, int, Uint32, const char *);
SDL_Surface		*text_render_unicode(const char *, int, SDL_Color,
			                     const Uint32 *);
void			 text_prescale_unicode(const Uint32 *, int *, int *);
__inline__ void		 text_prescale(const char *, int *, int *);

void text_msg(enum text_msg_title, const char *, ...)
	          FORMAT_ATTRIBUTE(printf, 2, 3)
	          NONNULL_ATTRIBUTE(2);
void text_tmsg(enum text_msg_title, Uint32, const char *, ...)
	          FORMAT_ATTRIBUTE(printf, 3, 4)
	          NONNULL_ATTRIBUTE(3);

void text_prompt_float(double *, double, double, const char *,
		       const char *, ...)
		         FORMAT_ATTRIBUTE(printf, 5, 6)
		         NONNULL_ATTRIBUTE(5);
void text_prompt_string(char **, size_t, const char *, ...)
		         FORMAT_ATTRIBUTE(printf, 3, 4)
		         NONNULL_ATTRIBUTE(3);

struct text_glyph *text_render_glyph(const char *, int, Uint32, Uint32);
void		   text_unused_glyph(struct text_glyph *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_TEXT_H_ */

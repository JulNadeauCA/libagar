/*	$Csoft: text.h,v 1.43 2005/09/20 10:22:13 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXT_H_
#define _AGAR_WIDGET_TEXT_H_

#include <engine/widget/button.h>
#include <engine/widget/window.h>

#include "begin_code.h"

enum ag_text_msg_title {
	AG_MSG_ERROR,
	AG_MSG_WARNING,
	AG_MSG_INFO
};

/* Cached glyph surface/texture information. */
typedef struct ag_glyph {
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
	SLIST_ENTRY(ag_glyph) glyphs;
} AG_Glyph;

/* Font engine independent representation of a font style. */
typedef struct ag_font {
	char name[32];
	int size;
	int style;
	void *p;
	SLIST_ENTRY(ag_font) fonts;
} AG_Font;

__BEGIN_DECLS
extern int agTextFontHeight, agTextFontAscent, agTextFontDescent,
	   agTextFontLineSkip, agTextTabWidth, agTextBlinkRate;

int	 AG_TextInit(void);
void	 AG_TextParseFontSpec(char *);
void	 AG_TextDestroy(void);

AG_Font		*AG_FetchFont(const char *, int, int);
__inline__ SDL_Surface	*AG_TextRender(const char *, int, Uint32, const char *);
SDL_Surface		*AG_TextRenderUnicode(const char *, int, SDL_Color,
			                      const Uint32 *);
void			 AG_TextPrescaleUnicode(const Uint32 *, int *, int *);
__inline__ void		 AG_TextPrescale(const char *, int *, int *);

void AG_TextMsg(enum ag_text_msg_title, const char *, ...)
	      FORMAT_ATTRIBUTE(printf, 2, 3)
	      NONNULL_ATTRIBUTE(2);
void AG_TextTmsg(enum ag_text_msg_title, Uint32, const char *, ...)
	       FORMAT_ATTRIBUTE(printf, 3, 4)
	       NONNULL_ATTRIBUTE(3);

AG_Window *AG_TextPromptOptions(AG_Button **, u_int, const char *, ...)
		         	FORMAT_ATTRIBUTE(printf, 3, 4)
		          	NONNULL_ATTRIBUTE(3);
void AG_TextPromptFloat(double *, double, double, const char *,
		        const char *, ...)
		        FORMAT_ATTRIBUTE(printf, 5, 6)
		        NONNULL_ATTRIBUTE(5);
void AG_TextEditString(char **, size_t, const char *, ...)
		       FORMAT_ATTRIBUTE(printf, 3, 4)
		       NONNULL_ATTRIBUTE(3);
void AG_TextPromptString(const char *, void (*)(int, union evarg *),
		         const char *, ...);

AG_Glyph *AG_TextRenderGlyph(const char *, int, Uint32, Uint32);
void	  AG_TextUnusedGlyph(AG_Glyph *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_TEXT_H_ */

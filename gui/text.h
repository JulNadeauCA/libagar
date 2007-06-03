/*	$Csoft: text.h,v 1.44 2005/09/27 00:25:23 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXT_H_
#define _AGAR_WIDGET_TEXT_H_

#include <agar/gui/button.h>
#include <agar/gui/window.h>

#include "begin_code.h"

enum ag_text_msg_title {
	AG_MSG_ERROR,
	AG_MSG_WARNING,
	AG_MSG_INFO
};

/* Cached glyph surface/texture information. */
typedef struct ag_glyph {
	char fontname[AG_OBJECT_NAME_MAX];
	int fontsize;			/* Font size in points */
	Uint32 color;			/* Glyph color */
	Uint32 ch;			/* Unicode character */
	Uint32 nrefs;			/* Reference count */
	SDL_Surface *su;		/* Rendered surface */
#ifdef HAVE_OPENGL
	Uint texture;			/* Rendered texture */
	float texcoord[4];
#endif
	SLIST_ENTRY(ag_glyph) glyphs;
} AG_Glyph;

/* Cached font */
typedef struct ag_font {
	struct ag_object obj;
	enum {
		AG_FONT_VECTOR,
		AG_FONT_BITMAP
	} type;
	int size;			/* Size in points */
	Uint flags;
#define AG_FONT_BOLD		0x01	/* Bold font */
#define AG_FONT_ITALIC		0x02	/* Italic font */
#define AG_FONT_UNDERLINE	0x04	/* Underlined */
#define AG_FONT_UPPERCASE	0x08	/* Force uppercase display */
	int height;			/* Body size in pixels */
	int ascent;			/* Ascent (relative to baseline) */
	int descent;			/* Descent (relative to baseline) */
	int lineskip;			/* Multiline y-increment */

	void *ttf;			/* TTF object */
	char bspec[32];			/* Bitmap font specification */
	SDL_Surface **bglyphs;		/* Bitmap glyphs */
	Uint nglyphs;			/* Bitmap glyph count */
	Uint32 c0, c1;			/* Bitmap glyph range */

	SLIST_ENTRY(ag_font) fonts;
} AG_Font;

__BEGIN_DECLS
extern int agTextFontHeight, agTextFontAscent, agTextFontDescent,
	   agTextFontLineSkip, agTextTabWidth, agTextBlinkRate;

int	 AG_TextInit(void);
void	 AG_TextParseFontSpec(const char *);
void	 AG_TextDestroy(void);

AG_Font			*AG_FetchFont(const char *, int, int);
void			 AG_FontDestroy(void *);

__inline__ SDL_Surface	*AG_TextRender(const char *, int, Uint32, const char *);
__inline__ SDL_Surface	*AG_TextFormat(const char *, int, Uint32, const char *,
			               ...);
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

void AG_TextEditFloat(double *, double, double, const char *,
		      const char *, ...)
		      FORMAT_ATTRIBUTE(printf, 5, 6)
		      NONNULL_ATTRIBUTE(5);
void AG_TextEditString(char **, size_t, const char *, ...)
		       FORMAT_ATTRIBUTE(printf, 3, 4)
		       NONNULL_ATTRIBUTE(3);

AG_Window *AG_TextPromptOptions(AG_Button **, Uint, const char *, ...)
		         	FORMAT_ATTRIBUTE(printf, 3, 4)
		          	NONNULL_ATTRIBUTE(3);
void AG_TextPromptString(const char *, void (*)(AG_Event *),
		         const char *, ...);
void AG_TextPromptDouble(const char *, const char *, double, double,
			 void (*)(AG_Event *), const char *, ...);

AG_Glyph *AG_TextRenderGlyph(const char *, int, Uint32, Uint32);
void	  AG_TextUnusedGlyph(AG_Glyph *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_TEXT_H_ */

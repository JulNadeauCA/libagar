/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXT_H_
#define _AGAR_WIDGET_TEXT_H_

#include <agar/config/have_opengl.h>
#include <agar/config/utf8.h>

#include "begin_code.h"

#define AG_TEXT_STATES_MAX 32

struct ag_window;
struct ag_button;

enum ag_font_type {
	AG_FONT_VECTOR,		/* Vectorial font */
	AG_FONT_BITMAP		/* Series of pixmaps */
};

enum ag_text_justify {
	AG_TEXT_LEFT,
	AG_TEXT_CENTER,
	AG_TEXT_RIGHT
};
enum ag_text_valign {
	AG_TEXT_TOP,
	AG_TEXT_MIDDLE,
	AG_TEXT_BOTTOM
};
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
	Uint32 lastRef;			/* Ticks since last reference */
	AG_Surface *su;			/* Rendered surface */
	int advance;			/* Pixel advance */

	/* For OpenGL */
	Uint texture;			/* Rendered texture */
	float texcoord[4];

	AG_SLIST_ENTRY(ag_glyph) glyphs;
} AG_Glyph;

/* Cached font */
typedef struct ag_font {
	struct ag_object obj;
	enum ag_font_type type;		/* Class of font */
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
	AG_Surface **bglyphs;		/* Bitmap glyphs */
	Uint nglyphs;			/* Bitmap glyph count */
	Uint32 c0, c1;			/* Bitmap glyph range */

	AG_SLIST_ENTRY(ag_font) fonts;
} AG_Font;

/* State variables for text rendering. */
typedef struct ag_text_state {
	AG_Font *font;			/* Font face */
	Uint32 color;			/* FG color (surfaceFmt) */
	Uint32 colorBG;			/* BG color (surfaceFmt) */
	enum ag_text_justify justify;	/* Justification mode */
	enum ag_text_valign valign;	/* Vertical alignment mode */
} AG_TextState;

/* Description of font stored in data segment. */
typedef struct ag_static_font {
	const char *name;		/* Identifier */
	enum ag_font_type type;		/* Type of font */
	Uint32 size;			/* Size in bytes */
	const Uint8 *data;		/* Font data */
	AG_Font *font;			/* Initialized font structure */
} AG_StaticFont;

/* Measures of rendered text. */
typedef struct ag_text_metrics {
	int w, h;			/* Dimensions in pixels */
	Uint *wLines;			/* Width of each line */
	Uint  nLines;			/* Total line count */
} AG_TextMetrics;

__BEGIN_DECLS
extern AG_ObjectClass agFontClass;
extern AG_Font *agDefaultFont;
extern int agTextFontHeight;
extern int agTextFontAscent;
extern int agTextFontDescent;
extern int agTextFontLineSkip;
extern int agFreetype;
extern int agGlyphGC;
extern AG_TextState *agTextState;
extern AG_Mutex agTextLock;

int	 AG_TextInit(void);
void	 AG_TextParseFontSpec(const char *);
void	 AG_TextDestroy(void);
AG_Font	*AG_FetchFont(const char *, int, int);
void	 AG_PushTextState(void);
void	 AG_PopTextState(void);
int	 AG_TextFontLookup(const char *, int, Uint);

void	 AG_TextSize(const char *, int *, int *);
void	 AG_TextSizeMulti(const char *, int *, int *, Uint **, Uint *);
void	 AG_TextSizeUCS4(const Uint32 *, int *, int *);
void	 AG_TextSizeMultiUCS4(const Uint32 *, int *, int *, Uint **, Uint *);

AG_Surface *AG_TextRenderf(const char *, ...);
AG_Surface *AG_TextRenderUCS4(const Uint32 *);


void AG_TextMsg(enum ag_text_msg_title, const char *, ...)
                FORMAT_ATTRIBUTE(printf,2,3)
		NONNULL_ATTRIBUTE(2);
void AG_TextTmsg(enum ag_text_msg_title, Uint32, const char *, ...)
	         FORMAT_ATTRIBUTE(printf,3,4)
		 NONNULL_ATTRIBUTE(3);
void AG_TextInfo(const char *, const char *, ...)
	         FORMAT_ATTRIBUTE(printf,2,3)
		 NONNULL_ATTRIBUTE(2);
void AG_TextWarning(const char *, const char *, ...)
	            FORMAT_ATTRIBUTE(printf,2,3)
	            NONNULL_ATTRIBUTE(2);
void AG_TextError(const char *, ...)
	          FORMAT_ATTRIBUTE(printf,1,2)
	          NONNULL_ATTRIBUTE(1);

void AG_TextEditFloat(double *, double, double, const char *,
		      const char *, ...)
		      FORMAT_ATTRIBUTE(printf,5,6)
		      NONNULL_ATTRIBUTE(5);
void AG_TextEditString(char *, size_t, const char *, ...)
		       FORMAT_ATTRIBUTE(printf,3,4)
		       NONNULL_ATTRIBUTE(3);

struct ag_window *AG_TextPromptOptions(struct ag_button **, Uint,
		                       const char *, ...)
		         	FORMAT_ATTRIBUTE(printf,3,4)
		          	NONNULL_ATTRIBUTE(3);
void AG_TextPromptString(const char *, void (*)(AG_Event *),
		         const char *, ...);

AG_Glyph *AG_TextRenderGlyph(Uint32);
void	  AG_TextUnusedGlyph(AG_Glyph *);
void	  AG_ClearGlyphCache(void);

void AG_TextAlign(int *, int *, int, int, int, int, int, int, int,
                  int, enum ag_text_justify, enum ag_text_valign);

#define     AG_TextFormat AG_TextRenderf
#define AG_TextMsgFromError() \
	AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError())

#define	AG_TextColor(name) AG_TextColorVideo32(AG_COLOR(name))
#define	AG_TextBGColor(name) AG_TextBGColorVideo32(AG_COLOR(name))


/* Compare two text states. */
static __inline__ int
AG_TextStateCompare(const AG_TextState *s1, const AG_TextState *s2)
{
	if (s1->font == s2->font &&
	    s1->color == s2->color &&
	    s1->colorBG == s2->colorBG &&
	    s1->justify == s2->justify) {
		return (0);
	}
	return (1);
}

/*
 * Return the offset in pixels needed to align text based on the current
 * justification mode.
 */
static __inline__ int
AG_TextJustifyOffset(int w, int wLine)
{
	switch (agTextState->justify) {
	case AG_TEXT_LEFT:	return (0);
	case AG_TEXT_CENTER:	return (w/2 - wLine/2);
	case AG_TEXT_RIGHT:	return (w - wLine);
	}
	return (0);
}

/*
 * Return the offset in pixels needed to align text based on the current
 * vertical alignment mode.
 */
static __inline__ int
AG_TextValignOffset(int h, int hLine)
{
	switch (agTextState->valign) {
	case AG_TEXT_TOP:	return (0);
	case AG_TEXT_MIDDLE:	return (h/2 - hLine/2);
	case AG_TEXT_BOTTOM:	return (h - hLine);
	}
	return (0);
}

/*
 * Allocate a transparent surface and render text from a standard C string
 * (possibly with UTF-8 sequences), onto it.
 */
static __inline__ AG_Surface *
AG_TextRender(const char *text)
{
	Uint32 *ucs;
	AG_Surface *su;
	
#ifdef UTF8
	ucs = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, text, 0);
#else
	ucs = AG_ImportUnicode(AG_UNICODE_FROM_USASCII, text, 0);
#endif
	su = AG_TextRenderUCS4(ucs);
	free(ucs);
	return (su);
}

/* Set text color from a 32-bit pixel value (agDisplayFormat). */
static __inline__ void
AG_TextColorVideo32(Uint32 pixel)
{
	AG_MutexLock(&agTextLock);
	agTextState->color = AG_SurfacePixel(pixel);
	AG_MutexUnlock(&agTextLock);
}

/* Set text color from a 32-bit pixel value (agSurfaceFormat). */
static __inline__ void
AG_TextColor32(Uint32 pixel)
{
	AG_MutexLock(&agTextLock);
	agTextState->color = pixel;
	AG_MutexUnlock(&agTextLock);
}

/* Set an opaque text color from RGB components. */
static __inline__ void
AG_TextColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_MutexLock(&agTextLock);
	agTextState->color = AG_MapRGB(agSurfaceFmt, r,g,b);
	AG_MutexUnlock(&agTextLock);
}

/* Set text color from RGBA components. */
static __inline__ void
AG_TextColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_MutexLock(&agTextLock);
	agTextState->color = AG_MapRGBA(agSurfaceFmt, r, g, b, a);
	AG_MutexUnlock(&agTextLock);
}

/* Set text color from 0xRRGGBBAA format. */
static __inline__ void
AG_TextColorHex(Uint32 c)
{
	AG_MutexLock(&agTextLock);
	agTextState->color = AG_MapRGBA(agSurfaceFmt,
	    (c&0xff000000) >> 24,
	    (c&0x00ff0000) >> 16,
	    (c&0x0000ff00) >> 8,
	    (c&0x000000ff));
	AG_MutexUnlock(&agTextLock);
}

/* Set BG color from a 32-bit pixel value (agDisplayFormat). */
static __inline__ void
AG_TextBGColorVideo32(Uint32 pixel)
{
	AG_MutexLock(&agTextLock);
	agTextState->colorBG = AG_SurfacePixel(pixel);
	AG_MutexUnlock(&agTextLock);
}

/* Set BG color from a 32-bit pixel value (agSurfaceFormat). */
static __inline__ void
AG_TextBGColor32(Uint32 pixel)
{
	AG_MutexLock(&agTextLock);
	agTextState->colorBG = pixel;
	AG_MutexUnlock(&agTextLock);
}

/* Set text BG color from RGB components. */
static __inline__ void
AG_TextBGColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_MutexLock(&agTextLock);
	agTextState->colorBG = AG_MapRGB(agSurfaceFmt, r,g,b);
	AG_MutexUnlock(&agTextLock);
}

/* Set text BG color from RGBA components. */
static __inline__ void
AG_TextBGColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_MutexLock(&agTextLock);
	agTextState->colorBG = AG_MapRGBA(agSurfaceFmt, r,g,b,a);
	AG_MutexUnlock(&agTextLock);
}

/* Set text BG color from 0xRRGGBBAA format. */
static __inline__ void
AG_TextBGColorHex(Uint32 c)
{
	AG_MutexLock(&agTextLock);
	agTextState->colorBG = AG_MapRGBA(agSurfaceFmt,
	    (c&0xff000000) >> 24,
	    (c&0x00ff0000) >> 16,
	    (c&0x0000ff00) >> 8,
	    (c&0x000000ff));
	AG_MutexUnlock(&agTextLock);
}

/* Select the font face to use in rendering text. */
static __inline__ void
AG_TextFont(AG_Font *font)
{
	AG_MutexLock(&agTextLock);
	agTextState->font = font;
	AG_MutexUnlock(&agTextLock);
}

/* Select the justification mode to use in rendering text. */
static __inline__ void
AG_TextJustify(enum ag_text_justify mode)
{
	AG_MutexLock(&agTextLock);
	agTextState->justify = mode;
	AG_MutexUnlock(&agTextLock);
}

/* Select the vertical alignment mode to use in rendering text. */
static __inline__ void
AG_TextValign(enum ag_text_valign mode)
{
	AG_MutexLock(&agTextLock);
	agTextState->valign = mode;
	AG_MutexUnlock(&agTextLock);
}
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_TEXT_H_ */

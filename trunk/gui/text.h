/*	Public domain	*/

#ifndef _AGAR_GUI_TEXT_H_
#define _AGAR_GUI_TEXT_H_
#include <agar/gui/begin.h>

#define AG_GLYPH_NBUCKETS 1024	/* Buckets for glyph cache table */
#define AG_TEXT_STATES_MAX 32	/* Maximum number of saved text states */

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

struct ag_font;

/* Cached glyph surface/texture information. */
typedef struct ag_glyph {
	struct ag_font *font;		/* Font face */
	AG_Color        color;		/* Glyph color */
	Uint32          ch;		/* Unicode character */
	AG_Surface     *su;		/* Rendered surface */
	int             advance;	/* Pixel advance */
	Uint            texture;	/* Cached texture (driver-specific) */
	AG_TexCoord     texcoords;	/* Texture coordinates */
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
	AG_Color color;			/* Foreground text color */
	AG_Color colorBG;		/* Background color */
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

typedef struct ag_glyph_cache {
	AG_SLIST_HEAD_(ag_glyph) glyphs;
} AG_GlyphCache;

__BEGIN_DECLS
extern AG_ObjectClass agFontClass;
extern AG_Font *agDefaultFont;
extern int agTextFontHeight;
extern int agTextFontAscent;
extern int agTextFontDescent;
extern int agTextFontLineSkip;
extern int agFreetypeInited;
extern int agRTL;
extern int agGlyphGC;
extern AG_TextState *agTextState;
extern AG_Mutex agTextLock;
extern AG_StaticFont *agBuiltinFonts[];
extern const int agBuiltinFontCount;

int	 AG_TextInit(void);
void	 AG_TextDestroy(void);

void	 AG_TextParseFontSpec(const char *);
AG_Font	*AG_FetchFont(const char *, int, int);
void     AG_DestroyFont(AG_Font *);
void	 AG_SetDefaultFont(AG_Font *);
void	 AG_SetRTL(int);

void	 AG_PushTextState(void);
void	 AG_PopTextState(void);
AG_Font *AG_TextFontLookup(const char *, int, Uint);
AG_Font *AG_TextFontPts(int);
AG_Font *AG_TextFontPct(int);

void	 AG_TextSize(const char *, int *, int *);
void	 AG_TextSizeMulti(const char *, int *, int *, Uint **, Uint *);
void	 AG_TextSizeUCS4(const Uint32 *, int *, int *);
void	 AG_TextSizeMultiUCS4(const Uint32 *, int *, int *, Uint **, Uint *);

AG_Surface *AG_TextRenderf(const char *, ...);
AG_Surface *AG_TextRenderUCS4(const Uint32 *);

void AG_TextMsgS(enum ag_text_msg_title, const char *);
void AG_TextMsg(enum ag_text_msg_title, const char *, ...)
                FORMAT_ATTRIBUTE(printf,2,3)
		NONNULL_ATTRIBUTE(2);

void AG_TextTmsgS(enum ag_text_msg_title, Uint32, const char *);
void AG_TextTmsg(enum ag_text_msg_title, Uint32, const char *, ...)
	         FORMAT_ATTRIBUTE(printf,3,4)
		 NONNULL_ATTRIBUTE(3);

void AG_TextInfoS(const char *, const char *);
void AG_TextInfo(const char *, const char *, ...)
	         FORMAT_ATTRIBUTE(printf,2,3)
		 NONNULL_ATTRIBUTE(2);

void AG_TextWarningS(const char *, const char *);
void AG_TextWarning(const char *, const char *, ...)
	            FORMAT_ATTRIBUTE(printf,2,3)
	            NONNULL_ATTRIBUTE(2);

void AG_TextErrorS(const char *);
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

void      AG_TextInitGlyphCache(AG_Driver *);
void      AG_TextClearGlyphCache(AG_Driver *);
AG_Glyph *AG_TextRenderGlyphMiss(AG_Driver *, Uint32);

void AG_TextAlign(int *, int *, int, int, int, int, int, int, int,
                  int, enum ag_text_justify, enum ag_text_valign);

#define AG_TextMsgFromError() \
	AG_TextMsgS(AG_MSG_ERROR, AG_GetError())

/* Compare two text states. */
static __inline__ int
AG_TextStateCompare(const AG_TextState *s1, const AG_TextState *s2)
{
	if (s1->font == s2->font &&
	    AG_ColorCompare(s1->color,s2->color) == 0 &&
	    AG_ColorCompare(s1->colorBG,s2->colorBG) == 0 &&
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
	
	ucs = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, text, 0);
	su = AG_TextRenderUCS4(ucs);
	AG_Free(ucs);
	return (su);
}

/*
 * Lookup/insert a glyph in the glyph cache.
 * Must be called from GUI rendering context.
 */
static __inline__ AG_Glyph *
AG_TextRenderGlyph(AG_Driver *drv, Uint32 ch)
{
	AG_Glyph *gl;
	Uint h = (Uint)(ch % AG_GLYPH_NBUCKETS);

	AG_SLIST_FOREACH(gl, &drv->glyphCache[h].glyphs, glyphs) {
		if (ch == gl->ch &&
		    agTextState->font == gl->font &&
		    AG_ColorCompare(agTextState->color,gl->color) == 0)
			break;
	}
	if (gl == NULL) {
		gl = AG_TextRenderGlyphMiss(drv, ch);
		AG_SLIST_INSERT_HEAD(&drv->glyphCache[h].glyphs, gl, glyphs);
	}
	return (gl);
}


/* Set active text color. */
static __inline__ void
AG_TextColor(AG_Color C)
{
	AG_MutexLock(&agTextLock);
	agTextState->color = C;
	AG_MutexUnlock(&agTextLock);
}
static __inline__ void
AG_TextColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_MutexLock(&agTextLock);
	agTextState->color = AG_ColorRGB(r,g,b);
	AG_MutexUnlock(&agTextLock);
}
static __inline__ void
AG_TextColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_MutexLock(&agTextLock);
	agTextState->color = AG_ColorRGBA(r,g,b,a);
	AG_MutexUnlock(&agTextLock);
}

/* Set text color from 0xRRGGBBAA format. */
static __inline__ void
AG_TextColorHex(Uint32 c)
{
	AG_MutexLock(&agTextLock);
	agTextState->color = AG_ColorHex(c);
	AG_MutexUnlock(&agTextLock);
}

/* Set active text background color. */
static __inline__ void
AG_TextBGColor(AG_Color C)
{
	AG_MutexLock(&agTextLock);
	agTextState->colorBG = C;
	AG_MutexUnlock(&agTextLock);
}
static __inline__ void
AG_TextBGColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_MutexLock(&agTextLock);
	agTextState->colorBG = AG_ColorRGB(r,g,b);
	AG_MutexUnlock(&agTextLock);
}
static __inline__ void
AG_TextBGColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_MutexLock(&agTextLock);
	agTextState->colorBG = AG_ColorRGBA(r,g,b,a);
	AG_MutexUnlock(&agTextLock);
}

/* Set text BG color from 0xRRGGBBAA format. */
static __inline__ void
AG_TextBGColorHex(Uint32 c)
{
	AG_MutexLock(&agTextLock);
	agTextState->colorBG = AG_ColorHex(c);
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

#ifdef AG_LEGACY
#define AG_TextFormat AG_TextRenderf

static __inline__ void
AG_TextColor32(Uint32 px)
{
	AG_Color C;

	C = AG_GetColorRGB(px, agSurfaceFmt);
	AG_MutexLock(&agTextLock);
	agTextState->color = C;
	AG_MutexUnlock(&agTextLock);
}
#endif /* AG_LEGACY */

__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_GUI_TEXT_H_ */

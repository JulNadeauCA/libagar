/*	Public domain	*/

#ifndef _AGAR_GUI_TEXT_H_
#define _AGAR_GUI_TEXT_H_

#include <agar/gui/surface.h>
#include <agar/gui/drv.h>
#include <agar/gui/begin.h>

#if AG_MODEL == AG_SMALL
# define AG_GLYPH_NBUCKETS  256
# define AG_TEXT_STATES_MAX 32
#else
# define AG_GLYPH_NBUCKETS  512
# define AG_TEXT_STATES_MAX 64
#endif

struct ag_window;
struct ag_button;

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

/* Font specification (may be obtained from fontconfig) */
enum ag_font_type {
	AG_FONT_VECTOR,				/* Vectorial font */
	AG_FONT_BITMAP				/* Raw glyph pixmaps */
};
enum ag_font_spec_source {
	AG_FONT_SOURCE_FILE,			/* Load from file */
	AG_FONT_SOURCE_MEMORY			/* Read from memory */
};

#ifdef AG_HAVE_FLOAT
typedef double AG_FontPts;
#else
typedef int AG_FontPts;
#endif

#ifndef AG_FONT_PTS_EPSILON
#define AG_FONT_PTS_EPSILON 0.01
#endif

typedef struct ag_font_spec {
	enum ag_font_type type;
	enum ag_font_spec_source sourceType;
	union {
		char file[AG_PATHNAME_MAX];	/* Font file */
		struct {
			const Uint8 *_Nonnull data;	/* Memory region */
			AG_Size size;
		} mem;
	} source;
	int index;				/* Font index */
	AG_FontPts size;			/* Font size */
#ifdef AG_HAVE_FLOAT
	struct {				/* Transform matrix */
		double xx, xy;
		double yx, yy;
	} matrix;
#endif
} AG_FontSpec;

/* Cached glyph surface/texture information. */
typedef struct ag_glyph {
	struct ag_font *_Nonnull font;	/* Font face */
	AG_Color color;			/* Base color */
	Uint32 ch;			/* Unicode character */
	AG_Surface *_Nonnull su;	/* Rendered surface */
	int advance;			/* Pixel advance */
	Uint texture;			/* Cached texture (driver-specific) */
	AG_TexCoord texcoords;		/* Texture coordinates */
	AG_SLIST_ENTRY(ag_glyph) glyphs;
} AG_Glyph;

/* Loaded font */
typedef struct ag_font {
	struct ag_object obj;
	AG_FontSpec spec;		/* Input font specification */
	Uint flags;
#define AG_FONT_BOLD		0x01	/* Render as bold */
#define AG_FONT_ITALIC		0x02	/* Render as italic */
#define AG_FONT_UNDERLINE	0x04	/* Render with underline */
#define AG_FONT_UPPERCASE	0x08	/* Force uppercase display */
	int height;			/* Body size in pixels */
	int ascent;			/* Ascent (relative to baseline) */
	int descent;			/* Descent (relative to baseline) */
	int lineskip;			/* Multiline y-increment */
	void *_Nonnull ttf;		/* AG_TTFFont object */
	char bspec[32];			/* Bitmap font specification */

	AG_Surface *_Nullable *_Nullable bglyphs; /* Glyph surfaces */
	Uint                             nglyphs; /* Glyph count */

	Uint32 c0, c1;			/* Glyph range */
	Uint nRefs;			/* Reference count */
	AG_TAILQ_ENTRY(ag_font) fonts;
} AG_Font;

/*
 * State variables for text rendering.
 * SYNC: AG_TextStateCompare()
 */
typedef struct ag_text_state {
	AG_Font *_Nonnull font;		/* Font face */
	AG_Color color;			/* Foreground text color */
	AG_Color colorBG;		/* Background color */
	enum ag_text_justify justify;	/* Justification mode */
	enum ag_text_valign valign;	/* Vertical alignment mode */
	int tabWd;			/* Width of \t in pixels */
} AG_TextState;

/* Description of font stored in data segment. */
typedef struct ag_static_font {
	const char *_Nonnull name;	/* Identifier */
	enum ag_font_type type;		/* Type of font */
	Uint32 size;			/* Size in bytes */
	const Uint8 *_Nonnull data;	/* Font data */
	AG_Font *_Nullable font;	/* Initialized font */
} AG_StaticFont;

/* Measures of rendered text. */
typedef struct ag_text_metrics {
	int w, h;			/* Dimensions in pixels */
	Uint *_Nullable wLines;		/* Width of each line */
	Uint            nLines;		/* Total line count */
} AG_TextMetrics;

typedef struct ag_glyph_cache {
	AG_SLIST_HEAD_(ag_glyph) glyphs;
} AG_GlyphCache;

__BEGIN_DECLS
extern AG_ObjectClass agFontClass;
extern AG_Font *_Nullable agDefaultFont;
extern int agTextFontHeight;
extern int agTextFontAscent;
extern int agTextFontDescent;
extern int agTextFontLineSkip;
extern int agFreetypeInited;
extern int agRTL;

extern AG_TextState *_Nonnull  agTextState;
extern _Nonnull_Mutex AG_Mutex agTextLock;

extern AG_StaticFont *_Nonnull agBuiltinFonts[];

int  AG_InitTextSubsystem(void);
void AG_DestroyTextSubsystem(void);
void AG_SetDefaultFont(AG_Font *_Nonnull);
void AG_TextParseFontSpec(const char *_Nonnull);
void AG_SetRTL(int);
void AG_PushTextState(void);
void AG_PopTextState(void);

AG_Font	*_Nullable AG_FetchFont(const char *_Nullable,
                                const AG_FontPts *_Nullable, Uint)
                               _Warn_Unused_Result;
void               AG_UnusedFont(AG_Font *_Nonnull);

AG_Font *_Nullable AG_TextFontLookup(const char *_Nullable,
                                     const AG_FontPts *_Nullable, Uint);

AG_Font *_Nullable AG_TextFontPts(const AG_FontPts *_Nullable);
AG_Font *_Nullable AG_TextFontPct(int);

void AG_TextSize(const char *_Nullable, int *_Nullable, int *_Nullable);
void AG_TextSizeMulti(const char *_Nonnull, int *_Nonnull, int *_Nonnull,
                      Uint *_Nullable *_Nonnull, Uint *_Nullable);
void AG_TextSizeUCS4(const Uint32 *_Nonnull, int *_Nullable, int *_Nullable);
void AG_TextSizeMultiUCS4(const Uint32 *_Nonnull, int *_Nullable,
                          int *_Nullable, Uint *_Nullable *_Nonnull,
			  Uint *_Nonnull);

AG_Surface *_Nonnull AG_TextRenderf(const char *_Nonnull, ...) _Warn_Unused_Result;
AG_Surface *_Nonnull AG_TextRenderUCS4(const Uint32 *_Nonnull) _Warn_Unused_Result;

void AG_TextMsgS(enum ag_text_msg_title, const char *_Nonnull);
void AG_TextMsg(enum ag_text_msg_title, const char *_Nonnull, ...)
               FORMAT_ATTRIBUTE(printf,2,3);
void AG_TextMsgFromError(void);

void AG_TextTmsgS(enum ag_text_msg_title, Uint32, const char *_Nonnull);
void AG_TextTmsg(enum ag_text_msg_title, Uint32, const char *_Nonnull, ...)
                FORMAT_ATTRIBUTE(printf,3,4);

void AG_TextInfoS(const char *_Nonnull, const char *_Nonnull);
void AG_TextInfo(const char *_Nonnull, const char *_Nonnull, ...)
                FORMAT_ATTRIBUTE(printf,2,3);

void AG_TextWarningS(const char *_Nonnull, const char *_Nonnull);
void AG_TextWarning(const char *_Nonnull, const char *_Nonnull, ...)
                   FORMAT_ATTRIBUTE(printf,2,3);
void AG_TextErrorS(const char *_Nonnull);
void AG_TextError(const char *_Nonnull, ...)
                 FORMAT_ATTRIBUTE(printf,1,2);

#ifdef AG_HAVE_FLOAT
void AG_TextEditFloat(double *_Nonnull, double, double, const char *_Nonnull,
                      const char *_Nonnull, ...)
                     FORMAT_ATTRIBUTE(printf,5,6);
#endif

void AG_TextEditString(char *_Nonnull, AG_Size, const char *_Nonnull, ...)
                      FORMAT_ATTRIBUTE(printf,3,4);

struct ag_window *_Nonnull AG_TextPromptOptions(struct ag_button *_Nonnull *_Nonnull ,
                                                Uint, const char *_Nonnull, ...);

void AG_TextInitGlyphCache(AG_Driver *_Nonnull);
void AG_TextClearGlyphCache(AG_Driver *_Nonnull);
void AG_TextDestroyGlyphCache(AG_Driver *_Nonnull);

AG_Glyph *_Nonnull AG_TextRenderGlyphMiss(AG_Driver *_Nonnull, Uint32);

void AG_TextAlign(int *_Nonnull, int *_Nonnull, int,int, int,int,
                  int,int,int, int, enum ag_text_justify, enum ag_text_valign);

int AG_TextJustifyOffset(int, int) _Pure_Attribute;
int AG_TextValignOffset(int, int) _Pure_Attribute;

AG_Surface *_Nonnull AG_TextRender(const char *_Nonnull)
                                  _Warn_Unused_Result;

AG_Glyph *_Nonnull AG_TextRenderGlyph(AG_Driver *_Nonnull, Uint32)
                                     _Warn_Unused_Result;

void AG_TextColor(const AG_Color *_Nonnull);
void AG_TextColorRGB(Uint8, Uint8, Uint8);
void AG_TextColorRGBA(Uint8, Uint8, Uint8, Uint8);
void AG_TextColorHex(Uint32);
void AG_TextBGColor(const AG_Color *_Nonnull);
void AG_TextBGColorRGB(Uint8, Uint8, Uint8);
void AG_TextBGColorRGBA(Uint8, Uint8, Uint8, Uint8);
void AG_TextBGColorHex(Uint32);
void AG_TextFont(AG_Font *_Nonnull);
void AG_TextJustify(enum ag_text_justify);
void AG_TextValign(enum ag_text_valign);
void AG_TextTabWidth(int);
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_GUI_TEXT_H_ */

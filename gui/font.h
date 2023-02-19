/*	Public domain	*/

#ifndef _AGAR_GUI_FONT_H_
#define _AGAR_GUI_FONT_H_

#include <agar/gui/begin.h>

struct ag_text_metrics;

/* Font type / font engine class. */
enum ag_font_type {
	AG_FONT_FREETYPE,			/* FreeType 2 font */
	AG_FONT_BITMAP,				/* Raw glyph pixmaps */
	AG_FONT_DUMMY,				/* No font engine */
	AG_FONT_TYPE_LAST
};
#define AG_FONT_VECTOR AG_FONT_FREETYPE

/* Font loading mechanism. */
enum ag_font_spec_source {
	AG_FONT_SOURCE_FILE,			/* Load font from file */
	AG_FONT_SOURCE_MEMORY			/* Read font from memory */
};

/* Epsilon for font point size comparisons. */
#ifndef AG_FONT_PTS_EPSILON
#define AG_FONT_PTS_EPSILON 0.01
#endif

/* Bucket size for the AG_TextRenderGlyph() cache. */
#ifndef AG_GLYPH_NBUCKETS
#define AG_GLYPH_NBUCKETS (AG_MODEL * 8)
#endif

/* Generic font specification. */
typedef struct ag_font_spec {
	float size;				/* Font size in points */
	int index;				/* Font index (FC_INDEX) */
	enum ag_font_type type;			/* Font engine type */
	enum ag_font_spec_source sourceType;	/* Source type */
	struct {				/* Transformation matrix */
		double xx, xy;
		double yx, yy;
	} matrix;
	union {
		struct {
			const Uint8 *_Nonnull data;  /* Source memory region */
			AG_Size size;                /* Size in bytes */
			AG_SIZE_PADDING(_pad);
		} mem;
	} source;
} AG_FontSpec;

/* Scaling and metric adjustments to a font. */
typedef struct ag_font_adjustment {
	const char *face;       /* Font family */
	float size_factor;      /* Scaling factor */
	int ascent_offset[6];   /* Ascent tweak (size range specific) */
	Uint regFlags;          /* Which styles to consider Regular or Normal */
	Uint stateFlags;        /* Set these stateFlags when font is loaded */
} AG_FontAdjustment;

/* Map alternate names to font names. */
typedef struct ag_font_alias {
	const char *from;     /* Originally named */
	const char *to;       /* Renamed to */
} AG_FontAlias;

/* Map individual AG_Font flag bits to fontconfig FC_STYLE names. */
typedef struct ag_font_style_name {
	const char *name;     /* Fontconfig FC_STYLE name */
	Uint flag;            /* Individual AG_Font(3) style bit */
} AG_FontStyleName;

/* Indicate sorting order for a given style, weight and width variant. */
typedef struct ag_font_style_sort {
	Uint16 flags;         /* AG_Font(3) style flags */
	Sint16 key;           /* Sort key (-1 = last) */
} AG_FontStyleSort;

/*
 * Cached rendering of a glyph of a given font and BG/FG color. This cache is
 * used by widgets which require per-glyph metrics such as AG_Editable(3).
 * For drivers which support hardware textures, the rendered glyph may be
 * associated with a generated texture and texture coordinates.
 */
typedef struct ag_glyph {
	struct ag_font *_Nonnull font;   /* Font face */
	AG_Color colorBG;                /* Background color */
	AG_Color color;                  /* Foreground color */
	AG_Surface *_Nonnull su;         /* Rendered surface */
	AG_Char ch;                      /* Native character */
	int advance;                     /* Advance (px) */
	Uint texture;                    /* Mapped texture (driver-specific) */
	AG_TexCoord texcoords;           /* Mapped texture coordinates */
	Uint32 _pad1;
	AG_SLIST_ENTRY(ag_glyph) glyphs; /* Entry in glyph cache */
} AG_Glyph;

/* Cache of rendered glyphs. Typically managed by an AG_Driver(3). */
typedef struct ag_glyph_cache {
	AG_SLIST_HEAD_(ag_glyph) glyphs; /* Cached glyph list */
} AG_GlyphCache;

/* Loaded font */
typedef struct ag_font {
	struct ag_object obj;           /* AG_Object -> AG_Font */

	char name[AG_OBJECT_NAME_MAX];  /* Base font name (without any suffix) */
	AG_FontSpec spec;               /* Generic font specification */

	Uint flags;                     /* Weight / Style / Width Variant */
#define AG_FONT_THIN           0x0001   /* Wt#100 - Thin */
#define AG_FONT_EXTRALIGHT     0x0002   /* Wt#200 - Extra Light ("Ultra Light") */
#define AG_FONT_LIGHT          0x0004   /* Wt#300 - Light */
                                        /* Wt#400 - Regular (the default) */
#define AG_FONT_SEMIBOLD       0x0008   /* Wt#600 - Semi Bold ("Demi Bold") */
#define AG_FONT_BOLD           0x0010   /* Wt#700 - Bold */
#define AG_FONT_EXTRABOLD      0x0020   /* Wt#800 - Extra Bold */
#define AG_FONT_BLACK          0x0040   /* Wt#900 - Black ("Heavy") */
#define AG_FONT_OBLIQUE        0x0080   /* Style - Oblique */
#define AG_FONT_ITALIC         0x0100   /* Style - Italic */
                            /* 0x0200      (Unused) */
#define AG_FONT_ULTRACONDENSED 0x0400   /* Wd(50%) - Ultra Condensed */
#define AG_FONT_CONDENSED      0x0800   /* Wd(75%) - Condensed */
#define AG_FONT_SEMICONDENSED  0x1000   /* Wd(87.5%) - Semi Condensed ("Demi Condensed") */
#define AG_FONT_SEMIEXPANDED   0x2000   /* Wd(112.5%) - Semi Expanded ("Demi Expanded") */
#define AG_FONT_EXPANDED       0x4000   /* Wd(125%) - Expanded */
#define AG_FONT_ULTRAEXPANDED  0x8000   /* Wd(200%) - Ultra Expanded */

#define AG_FONT_WEIGHTS (AG_FONT_THIN | AG_FONT_EXTRALIGHT | AG_FONT_LIGHT | \
                         AG_FONT_SEMIBOLD | AG_FONT_BOLD | AG_FONT_EXTRABOLD | \
                         AG_FONT_BLACK)

#define AG_FONT_STYLES (AG_FONT_OBLIQUE | AG_FONT_ITALIC)

#define AG_FONT_WD_VARIANTS (AG_FONT_ULTRACONDENSED | AG_FONT_CONDENSED | \
                             AG_FONT_SEMICONDENSED | AG_FONT_SEMIEXPANDED | \
                             AG_FONT_EXPANDED | AG_FONT_ULTRAEXPANDED)

	Uint           nFamilyStyles;
	Uint *_Nullable familyStyles;   /* Styles available in this font family */

	Uint stateFlags;
#define AG_FONT_FONTCONFIGED      0x01  /* Discovered via fontconfig */
#define AG_FONT_FAMILY_FLAGS      0x02  /* Family flags are specified */

	int height;                     /* Height (px) */
	int ascent;                     /* Ascent (px) */
	int descent;                    /* Descent (px) */
	int lineskip;                   /* Multiline y-increment (px) */
	int underlinePos;               /* Underline position */
	int underlineThk;               /* Underline thickness */
	Uint32 tAccess;                 /* Access time (debug mode only) */
	AG_TAILQ_ENTRY(ag_font) fonts;  /* Entry in global fonts list */
} AG_Font;

typedef AG_TAILQ_HEAD(ag_fontq, ag_font) AG_FontQ;

/* Description of a font stored in data segment. */
typedef struct ag_static_font {
	const char *_Nonnull name;	/* Identifier */
	enum ag_font_type type;		/* Type of font */
	Uint32 size;			/* Size in bytes */
	const Uint8 *_Nonnull data;	/* Font data */
	AG_Font *_Nullable font;	/* Initialized font */
} AG_StaticFont;

/* Font class description */
typedef struct ag_font_class {
	struct ag_object_class _inherit;     /* [AG_Object] -> [AG_Font] */

	int             (*_Nonnull open)(void *_Nonnull, const char *_Nonnull);
	void            (*_Nonnull flush_cache)(void *_Nonnull);
	void            (*_Nonnull close)(void *_Nonnull);
	void *_Nullable (*_Nonnull get_glyph)(void *_Nonnull, AG_Char, Uint);
	void            (*_Nonnull get_glyph_metrics)(void *_Nonnull,
	                                              AG_Glyph *_Nonnull);

	void            (*_Nonnull render)(const AG_Char *_Nonnull,
	                                   AG_Surface *_Nonnull,
	                                   const struct ag_text_metrics *_Nonnull,
	                                   AG_Font *_Nonnull,
	                                   const AG_Color *_Nonnull,
	                                   const AG_Color *_Nonnull);

	void            (*_Nonnull size)(const AG_Font *_Nonnull,
	                                 const AG_Char *_Nonnull,
	                                 struct ag_text_metrics *_Nonnull, int);
} AG_FontClass;

#define AGFONT(font)     ((AG_Font *)(font))
#define AGFONT_OPS(font) ((AG_FontClass *)AGOBJECT(font)->cls)

__BEGIN_DECLS
extern AG_FontClass agFontClass;
extern const AG_FontAdjustment agFontAdjustments[];
extern const AG_FontAlias agFontAliases[];
extern const AG_FontStyleName agFontStyleNames[];
extern const AG_FontStyleSort agFontStyleSort[];
extern const char *agFontFileExts[];
extern AG_StaticFont *_Nonnull agBuiltinFonts[];

AG_Font	*_Nullable AG_FetchFont(const char *_Nullable, float, Uint)
                               _Warn_Unused_Result;

int     AG_FontGetFamilyStyles(AG_Font *_Nonnull);
AG_Size AG_FontGetStyleName(char *_Nonnull, AG_Size, Uint);
Uint    AG_FontGetStyleByName(const char *_Nonnull);

#ifdef AG_LEGACY
#define AG_UnusedFont(font) /* unused */
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_GUI_FONT_H_ */

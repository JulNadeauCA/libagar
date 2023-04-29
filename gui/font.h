/*	Public domain	*/

#ifndef _AGAR_GUI_FONT_H_
#define _AGAR_GUI_FONT_H_

#include <agar/gui/begin.h>

struct ag_text_metrics;

/* Font type / font engine class. */
enum ag_font_type {
	AG_FONT_FREETYPE,             /* FreeType 2 font */
	AG_FONT_BITMAP,               /* Raw glyph pixmaps */
	AG_FONT_DUMMY,                /* No font engine */
	AG_FONT_TYPE_LAST
};
#define AG_FONT_VECTOR AG_FONT_FREETYPE

/* Font loading mechanism. */
enum ag_font_spec_source {
	AG_FONT_SOURCE_FILE,          /* Load font from file */
	AG_FONT_SOURCE_MEMORY         /* Read font from memory */
};

#ifndef AG_FONT_NAME_MAX              /* Max length of font name */
#define AG_FONT_NAME_MAX 48
#endif
#ifndef AG_FONT_PTS_EPSILON           /* Epsilon for point size comparisons */
#define AG_FONT_PTS_EPSILON 0.01
#endif
#ifndef AG_GLYPH_NBUCKETS             /* Bucket count for glyph cache */
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
	const char *face;         /* Font family */
	float size_factor;        /* Scaling factor */
	Sint8 ascent_offset[32];  /* Ascent offsets for standard point sizes */
	Uint regFlags;            /* Which styles to consider Regular or Normal */
	Uint stateFlags;          /* Set these stateFlags when font is loaded */
} AG_FontAdjustment;

/* Map alternate names to font names. */
typedef struct ag_font_alias {
	const char *from;         /* Originally named */
	const char *to;           /* Renamed to */
} AG_FontAlias;

/* Map individual AG_Font flag bits to fontconfig FC_STYLE names. */
typedef struct ag_font_style_name {
	const char *name;         /* Fontconfig FC_STYLE name */
	Uint flag;                /* Individual AG_Font(3) style bit */
} AG_FontStyleName;

/* Indicate sorting order for a given style, weight and width variant. */
typedef struct ag_font_style_sort {
	Uint16 flags;             /* AG_Font(3) style flags */
	Sint16 key;               /* Sort key (-1 = last) */
} AG_FontStyleSort;

/*
 * Cached rendering of a glyph of a given font and BG/FG color. This cache is
 * used by widgets which require per-glyph metrics such as AG_Editable(3).
 * For drivers which support hardware textures, the rendered glyph may be
 * associated with a generated texture and texture coordinates.
 */
typedef struct ag_glyph {
	struct ag_font *_Nonnull font;       /* Font face */
	AG_Color colorBG;                    /* Background color */
	AG_Color color;                      /* Foreground color */
	AG_Surface *_Nonnull su;             /* Rendered surface */
	AG_Char ch;                          /* Native character */
	int advance;                         /* Advance (px) */
	Uint texture;                        /* Driver-specific texture ID */
	AG_TexCoord texcoords;               /* Mapped texture coordinates */
	Uint32 _pad1;
	AG_SLIST_ENTRY(ag_glyph) glyphs;     /* Entry in glyph cache */
} AG_Glyph;

/* Cache of rendered glyphs (usually driver-managed). */
typedef struct ag_glyph_cache {
	AG_SLIST_HEAD_(ag_glyph) glyphs;      /* Cached glyph list */
} AG_GlyphCache;

/* Font object instance */
typedef struct ag_font {
	struct ag_object obj;           /* AG_Object -> AG_Font */
	char name[AG_FONT_NAME_MAX];    /* Base font name (without any suffix) */
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
#define AG_FONT_ULTRACONDENSED 0x0400   /* Wd(50%) - Ultra Condensed ("Compressed") */
#define AG_FONT_CONDENSED      0x0800   /* Wd(75%) - Condensed */
#define AG_FONT_SEMICONDENSED  0x1000   /* Wd(87.5%) - Semi Condensed ("Demi Condensed") */
#define AG_FONT_SEMIEXPANDED   0x2000   /* Wd(112.5%) - Semi Expanded ("Demi Expanded") */
#define AG_FONT_EXPANDED       0x4000   /* Wd(125%) - Expanded */
#define AG_FONT_ULTRAEXPANDED  0x8000   /* Wd(200%) - Ultra Expanded */
#define AG_FONT_WEIGHTS     (AG_FONT_THIN | AG_FONT_EXTRALIGHT | AG_FONT_LIGHT | \
                             AG_FONT_SEMIBOLD | AG_FONT_BOLD | AG_FONT_EXTRABOLD | \
                             AG_FONT_BLACK)
#define AG_FONT_STYLES      (AG_FONT_OBLIQUE | AG_FONT_ITALIC)
#define AG_FONT_WD_VARIANTS (AG_FONT_ULTRACONDENSED | AG_FONT_CONDENSED | \
                             AG_FONT_SEMICONDENSED | AG_FONT_SEMIEXPANDED | \
                             AG_FONT_EXPANDED | AG_FONT_ULTRAEXPANDED)

	Uint32 uniRanges[4];             /* Unicode coverage (OS/2 Charsets) */

	Uint           nFamilyStyles;
	Uint *_Nullable familyStyles;    /* Styles available in this font family */

	Uint stateFlags;
#define AG_FONT_FONTCONFIGED 0x01        /* Discovered via fontconfig */
#define AG_FONT_FAMILY_FLAGS 0x02        /* Family flags were found */

	int height;                      /* Height (px) */
	int ascent;                      /* Ascent (px) */
	int descent;                     /* Descent (px) */
	int lineskip;                    /* Multiline y-increment (px) */
	int underlinePos;                /* Underline position */
	int underlineThk;                /* Underline thickness */
	int typoAscender;
	int typoDescender;
	int typoLineGap;
	int usWinAscent;
	int usWinDescent;
	Uint32 tAccess;                  /* Access time (debug mode only) */
	AG_TAILQ_ENTRY(ag_font) fonts;   /* Entry in global fonts list */
} AG_Font;

/*
 * Macros to test the unicode coverage of a font.
 */
#define AG_FONT_HAS_LATIN1_SUPPL(f)         (((f)->uniRanges[0] >>  1) & 1)
#define AG_FONT_HAS_LATIN_EXT_A(f)          (((f)->uniRanges[0] >>  2) & 1)
#define AG_FONT_HAS_LATIN_EXT_B(f)          (((f)->uniRanges[0] >>  3) & 1)
#define AG_FONT_HAS_IPA_EXTENSIONS(f)       (((f)->uniRanges[0] >>  4) & 1)
#define AG_FONT_HAS_SP_MOD_LETTERS(f)       (((f)->uniRanges[0] >>  5) & 1)
#define AG_FONT_HAS_COMB_DIACRIT_MARKS(f)   (((f)->uniRanges[0] >>  6) & 1)
#define AG_FONT_HAS_GREEK(f)                (((f)->uniRanges[0] >>  7) & 1)
#define AG_FONT_HAS_COPTIC(f)               (((f)->uniRanges[0] >>  8) & 1)
#define AG_FONT_HAS_CYRILLIC(f)             (((f)->uniRanges[0] >>  9) & 1)
#define AG_FONT_HAS_ARMENIAN(f)             (((f)->uniRanges[0] >> 10) & 1)
#define AG_FONT_HAS_HEBREW(f)               (((f)->uniRanges[0] >> 11) & 1)
#define AG_FONT_HAS_VAI(f)                  (((f)->uniRanges[0] >> 12) & 1)
#define AG_FONT_HAS_ARABIC(f)               (((f)->uniRanges[0] >> 13) & 1)
#define AG_FONT_HAS_NKO(f)                  (((f)->uniRanges[0] >> 14) & 1)
#define AG_FONT_HAS_DEVANAGARI(f)           (((f)->uniRanges[0] >> 15) & 1)
#define AG_FONT_HAS_BENGALI(f)              (((f)->uniRanges[0] >> 16) & 1)
#define AG_FONT_HAS_GURMUKHI(f)             (((f)->uniRanges[0] >> 17) & 1)
#define AG_FONT_HAS_GUJARATI(f)             (((f)->uniRanges[0] >> 18) & 1)
#define AG_FONT_HAS_ORIYA(f)                (((f)->uniRanges[0] >> 19) & 1)
#define AG_FONT_HAS_TAMIL(f)                (((f)->uniRanges[0] >> 20) & 1)
#define AG_FONT_HAS_TELUGU(f)               (((f)->uniRanges[0] >> 21) & 1)
#define AG_FONT_HAS_KANNADA(f)              (((f)->uniRanges[0] >> 22) & 1)
#define AG_FONT_HAS_MALAYALAM(f)            (((f)->uniRanges[0] >> 23) & 1)
#define AG_FONT_HAS_THAI(f)                 (((f)->uniRanges[0] >> 24) & 1)
#define AG_FONT_HAS_LAO(f)                  (((f)->uniRanges[0] >> 25) & 1)
#define AG_FONT_HAS_GEORGIAN(f)             (((f)->uniRanges[0] >> 26) & 1)
#define AG_FONT_HAS_BALINESE(f)             (((f)->uniRanges[0] >> 27) & 1)
#define AG_FONT_HAS_HANGUL_JAMO(f)          (((f)->uniRanges[0] >> 28) & 1)
#define AG_FONT_HAS_LATIN_EXT_ADD_CD(f)     (((f)->uniRanges[0] >> 29) & 1)
#define AG_FONT_HAS_GREEK_EXTENDED(f)       (((f)->uniRanges[0] >> 30) & 1)
#define AG_FONT_HAS_GEN_PUNCT(f)            (((f)->uniRanges[0] >> 31) & 1)
#define AG_FONT_HAS_SUPER_SUB_SCRIPTS(f)     ((f)->uniRanges[1]        &1) /* 32 */
#define AG_FONT_HAS_CURRENCY_SYMBOLS(f)     (((f)->uniRanges[1] >>  1) &1) /* 33 */
#define AG_FONT_HAS_COMB_DIACRIT_SYM(f)     (((f)->uniRanges[1] >>  2) &1) /* 34 */
#define AG_FONT_HAS_LETTERLIKE_SYMBOLS(f)   (((f)->uniRanges[1] >>  3) &1) /* 35 */
#define AG_FONT_HAS_NUMBER_FORMS(f)         (((f)->uniRanges[1] >>  4) &1) /* 36 */
#define AG_FONT_HAS_ARROWS(f)               (((f)->uniRanges[1] >>  5) &1) /* 37 */
#define AG_FONT_HAS_MATH_OPS(f)             (((f)->uniRanges[1] >>  6) &1) /* 38 */
#define AG_FONT_HAS_MISC_TECH(f)            (((f)->uniRanges[1] >>  7) &1) /* 39 */
#define AG_FONT_HAS_CONTROL_PICTURES(f)     (((f)->uniRanges[1] >>  8) &1) /* 40 */
#define AG_FONT_HAS_OCR(f)                  (((f)->uniRanges[1] >>  9) &1) /* 41 */
#define AG_FONT_HAS_ENCL_ALPHANUMERICS(f)   (((f)->uniRanges[1] >> 10) &1) /* 42 */
#define AG_FONT_HAS_BOX_DRAWING(f)          (((f)->uniRanges[1] >> 11) &1) /* 43 */
#define AG_FONT_HAS_BLOCK_ELEMENTS(f)       (((f)->uniRanges[1] >> 12) &1) /* 44 */
#define AG_FONT_HAS_GEOMETRIC_SHAPES(f)     (((f)->uniRanges[1] >> 13) &1) /* 45 */
#define AG_FONT_HAS_MISC_SYMBOLS(f)         (((f)->uniRanges[1] >> 14) &1) /* 46 */
#define AG_FONT_HAS_DINGBATS(f)             (((f)->uniRanges[1] >> 15) &1) /* 47 */
#define AG_FONT_HAS_CJK_SYM_PUNCT(f)        (((f)->uniRanges[1] >> 16) &1) /* 48 */
#define AG_FONT_HAS_HIRAGANA(f)             (((f)->uniRanges[1] >> 17) &1) /* 49 */
#define AG_FONT_HAS_KATAKANA(f)             (((f)->uniRanges[1] >> 18) &1) /* 50 */
#define AG_FONT_HAS_BOPOMOFO(f)             (((f)->uniRanges[1] >> 19) &1) /* 51 */
#define AG_FONT_HAS_HANGUL_COMPAT_JAMO(f)   (((f)->uniRanges[1] >> 20) &1) /* 52 */
#define AG_FONT_HAS_PHAGS_PA(f)             (((f)->uniRanges[1] >> 21) &1) /* 53 */
#define AG_FONT_HAS_ENCL_CJK_LTRMONTHS(f)   (((f)->uniRanges[1] >> 22) &1) /* 54 */
#define AG_FONT_HAS_CJK_COMPAT(f)           (((f)->uniRanges[1] >> 23) &1) /* 55 */
#define AG_FONT_HAS_HANGUL_SYLLABLES(f)     (((f)->uniRanges[1] >> 24) &1) /* 56 */
#define AG_FONT_HAS_PHOENICIAN(f)           (((f)->uniRanges[1] >> 26) &1) /* 58 */
#define AG_FONT_HAS_CJK(f)                  (((f)->uniRanges[1] >> 27) &1) /* 59 */
#define AG_FONT_HAS_PRIVATE_USE_AREA(f)     (((f)->uniRanges[1] >> 28) &1) /* 60 */
#define AG_FONT_HAS_CJK_STROKES(f)          (((f)->uniRanges[1] >> 29) &1) /* 61 */
#define AG_FONT_HAS_ALPHA_PRES_FORMS(f)     (((f)->uniRanges[1] >> 30) &1) /* 62 */
#define AG_FONT_HAS_ARABIC_PRES_FORMS(f)    (((f)->uniRanges[1] >> 31) &1) /* 63 */
#define AG_FONT_HAS_COMB_HALF_MARKS(f)       ((f)->uniRanges[2]        &1) /* 64 */
#define AG_FONT_HAS_V_CJK_COMPAT_FORMS(f)   (((f)->uniRanges[2] >>  1) &1) /* 65 */
#define AG_FONT_HAS_SMALL_FORM_VARIANTS(f)  (((f)->uniRanges[2] >>  2) &1) /* 66 */
#define AG_FONT_HAS_ARABIC_PRES_FORMS_B(f)  (((f)->uniRanges[2] >>  3) &1) /* 67 */
#define AG_FONT_HAS_HALFWIDTH_FULLWIDTH(f)  (((f)->uniRanges[2] >>  4) &1) /* 68 */
#define AG_FONT_HAS_SPECIALS(f)             (((f)->uniRanges[2] >>  5) &1) /* 69 */
#define AG_FONT_HAS_TIBETAN(f)              (((f)->uniRanges[2] >>  6) &1) /* 70 */
#define AG_FONT_HAS_SYRIAC(f)               (((f)->uniRanges[2] >>  7) &1) /* 71 */
#define AG_FONT_HAS_THAANA(f)               (((f)->uniRanges[2] >>  8) &1) /* 72 */
#define AG_FONT_HAS_SINHALA(f)              (((f)->uniRanges[2] >>  9) &1) /* 73 */
#define AG_FONT_HAS_MYANMAR(f)              (((f)->uniRanges[2] >> 10) &1) /* 74 */
#define AG_FONT_HAS_ETHIOPIC(f)             (((f)->uniRanges[2] >> 11) &1) /* 75 */
#define AG_FONT_HAS_CHEROKEE(f)             (((f)->uniRanges[2] >> 12) &1) /* 76 */
#define AG_FONT_HAS_UNI_CA_ABORIG_SYLL(f)   (((f)->uniRanges[2] >> 13) &1) /* 77 */
#define AG_FONT_HAS_OGHAM(f)                (((f)->uniRanges[2] >> 14) &1) /* 78 */
#define AG_FONT_HAS_RUNIC(f)                (((f)->uniRanges[2] >> 15) &1) /* 79 */
#define AG_FONT_HAS_KHMER(f)                (((f)->uniRanges[2] >> 16) &1) /* 80 */
#define AG_FONT_HAS_MONGOLIAN(f)            (((f)->uniRanges[2] >> 17) &1) /* 81 */
#define AG_FONT_HAS_BRAILLE_PATTERNS(f)     (((f)->uniRanges[2] >> 18) &1) /* 82 */
#define AG_FONT_HAS_YI_SYLLABLES(f)         (((f)->uniRanges[2] >> 19) &1) /* 83 */
#define AG_FONT_HAS_TL_HNN_BKU_TAGB(f)      (((f)->uniRanges[2] >> 20) &1) /* 84 */
#define AG_FONT_HAS_OLD_ITALIC(f)           (((f)->uniRanges[2] >> 21) &1) /* 85 */
#define AG_FONT_HAS_GOTHIC(f)               (((f)->uniRanges[2] >> 22) &1) /* 86 */
#define AG_FONT_HAS_DESERET(f)              (((f)->uniRanges[2] >> 23) &1) /* 87 */
#define AG_FONT_HAS_MUSICAL_SYMBOLS(f)      (((f)->uniRanges[2] >> 24) &1) /* 88 */
#define AG_FONT_HAS_MATH_ALPHANUM_SYM(f)    (((f)->uniRanges[2] >> 25) &1) /* 89 */
#define AG_FONT_HAS_SUPPL_PRIVATE_USE(f)    (((f)->uniRanges[2] >> 26) &1) /* 90 */
#define AG_FONT_HAS_VARIATION_SELECTORS(f)  (((f)->uniRanges[2] >> 27) &1) /* 91 */
#define AG_FONT_HAS_TAGS(f)                 (((f)->uniRanges[2] >> 28) &1) /* 92 */
#define AG_FONT_HAS_LIMBU(f)                (((f)->uniRanges[2] >> 29) &1) /* 93 */
#define AG_FONT_HAS_TAI_LE(f)               (((f)->uniRanges[2] >> 30) &1) /* 94 */
#define AG_FONT_HAS_NEW_TAI_LUE(f)          (((f)->uniRanges[2] >> 31) &1) /* 95 */
#define AG_FONT_HAS_BUGINESE(f)              ((f)->uniRanges[3]        &1) /* 96 */
#define AG_FONT_HAS_GLAGOLITIC(f)           (((f)->uniRanges[3] >>  1) &1) /* 97 */
#define AG_FONT_HAS_TIFINAGH(f)             (((f)->uniRanges[3] >>  2) &1) /* 98 */
#define AG_FONT_HAS_YIJING_HEXAGRAM_SYM(f)  (((f)->uniRanges[3] >>  3) &1) /* 99 */
#define AG_FONT_HAS_SYLOTI_NAGRI(f)         (((f)->uniRanges[3] >>  4) &1) /* 100 */
#define AG_FONT_HAS_LINEAR_B_SYLLABARY(f)   (((f)->uniRanges[3] >>  5) &1) /* 101 */
#define AG_FONT_HAS_ANCIENT_GREEK_NUM(f)    (((f)->uniRanges[3] >>  6) &1) /* 102 */
#define AG_FONT_HAS_UGARITIC(f)             (((f)->uniRanges[3] >>  7) &1) /* 103 */
#define AG_FONT_HAS_OLD_PERSIAN(f)          (((f)->uniRanges[3] >>  8) &1) /* 104 */
#define AG_FONT_HAS_SHAVIAN(f)              (((f)->uniRanges[3] >>  9) &1) /* 105 */
#define AG_FONT_HAS_OSMANYA(f)              (((f)->uniRanges[3] >> 10) &1) /* 106 */
#define AG_FONT_HAS_CYPRIOT_SYLLABARY(f)    (((f)->uniRanges[3] >> 11) &1) /* 107 */
#define AG_FONT_HAS_KHAROSHTHI(f)           (((f)->uniRanges[3] >> 12) &1) /* 108 */
#define AG_FONT_HAS_TAI_XUAN_JING_SYM(f)    (((f)->uniRanges[3] >> 13) &1) /* 109 */
#define AG_FONT_HAS_CUNEIFORM(f)            (((f)->uniRanges[3] >> 14) &1) /* 110 */
#define AG_FONT_HAS_COUNTING_ROD_NUM(f)     (((f)->uniRanges[3] >> 15) &1) /* 111 */
#define AG_FONT_HAS_SUNDANESE(f)            (((f)->uniRanges[3] >> 16) &1) /* 112 */
#define AG_FONT_HAS_LEPCHA(f)               (((f)->uniRanges[3] >> 17) &1) /* 113 */
#define AG_FONT_HAS_OL_CHIKI(f)             (((f)->uniRanges[3] >> 18) &1) /* 114 */
#define AG_FONT_HAS_SAURASHTRA(f)           (((f)->uniRanges[3] >> 19) &1) /* 115 */
#define AG_FONT_HAS_KAYAH_LI(f)             (((f)->uniRanges[3] >> 20) &1) /* 116 */
#define AG_FONT_HAS_REJANG(f)               (((f)->uniRanges[3] >> 21) &1) /* 117 */
#define AG_FONT_HAS_CHAM(f)                 (((f)->uniRanges[3] >> 22) &1) /* 118 */
#define AG_FONT_HAS_ANCIENT_SYMBOLS(f)      (((f)->uniRanges[3] >> 23) &1) /* 119 */
#define AG_FONT_HAS_PHAISTOS_DISC(f)        (((f)->uniRanges[3] >> 24) &1) /* 120 */
#define AG_FONT_HAS_CARIAN_LYCIAN_LYDIAN(f) (((f)->uniRanges[3] >> 25) &1) /* 121 */
#define AG_FONT_HAS_DOMINO_MAHJONG_TILES(f) (((f)->uniRanges[3] >> 26) &1) /* 122 */

#define AG_FONT_HAS_LATIN1(f) (((f)->uniRanges[0] & 0xf) == 0xf)
#define AG_FONT_HAS_LATIN(f)   (AG_FONT_HAS_LATIN1(f) && \
                                AG_FONT_HAS_LATIN_EXT_ADD_CD(f))

typedef AG_TAILQ_HEAD(ag_fontq, ag_font) AG_FontQ;

/* Information about a range of Unicode characters. */
typedef struct ag_unicode_range {
	const char *_Nonnull name;     /* Range name */
	AG_Char from, to;              /* First and last characters in range */
	const char *_Nullable fonts;   /* Known fonts with Unicode coverage */
} AG_UnicodeRange;

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

#define   AGFONT(o)        ((AG_Font *)(o))
#define  AGcFONT(o)        ((const AG_Font *)(o))
#define   AGFONT_OPS(o)    ((AG_FontClass *)AGOBJECT(o)->cls)
#define  AG_FONT_ISA(o)    (((AGOBJECT(o)->cid & 0xff000000) >> 24) == 0x07)
#define  AG_FONT_SELF()    AGFONT(  AG_OBJECT(0,         "AG_Font:*") )
#define  AG_FONT_PTR(n)    AGFONT(  AG_OBJECT((n),       "AG_Font:*") )
#define  AG_FONT_NAMED(n)  AGFONT(  AG_OBJECT_NAMED((n), "AG_Font:*") )
#define AG_cFONT_SELF()   AGcFONT( AG_cOBJECT(0,         "AG_Font:*") )
#define AG_cFONT_PTR(n)   AGcFONT( AG_cOBJECT((n),       "AG_Font:*") )
#define AG_cFONT_NAMED(n) AGcFONT( AG_cOBJECT_NAMED((n), "AG_Font:*") )

__BEGIN_DECLS
extern AG_FontClass agFontClass;

extern const AG_FontAdjustment agFontAdjustments[];
extern const AG_FontAlias      agFontAliases[];
extern const AG_FontStyleName  agFontStyleNames[];
extern const AG_FontStyleSort  agFontStyleSort[];
extern const char             *agFontFileExts[];
extern const float             agFontStdSizes[32];
extern AG_StaticFont *_Nonnull agBuiltinFonts[];
extern const AG_UnicodeRange   agUnicodeRanges[];
extern const int               agUnicodeRangeCount;
extern const int               agUnicodeRangeFromOS2[][8];

AG_Font	*_Nullable AG_FetchFont(const char *_Nullable, float, Uint)
                               _Warn_Unused_Result;
AG_Font	*_Nonnull  AG_FetchFontFromList(const char *_Nullable, float, Uint)
                                        _Warn_Unused_Result;

int     AG_FontGetFamilyStyles(AG_Font *_Nonnull);
AG_Size AG_FontGetStyleName(char *_Nonnull, AG_Size, Uint);
Uint    AG_FontGetStyleByName(const char *_Nonnull);
float   AG_FontGetStandardSize(float);
int     AG_FontGetStandardSizeIndex(float);

#ifdef AG_LEGACY
#define AG_UnusedFont(font) /* unused */
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_GUI_FONT_H_ */

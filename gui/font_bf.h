/*	Public domain	*/

#ifndef _AGAR_GUI_FONT_BF_H_
#define _AGAR_GUI_FONT_BF_H_

#include <agar/gui/begin.h>

/* Agar Bitmap font glyph. */
typedef struct ag_glyph_bf {
	AG_Char ch;                /* Unicode character mapping */
	AG_CHAR_PADDING(_pad1);
	Uint flags;
#define AG_GLYPH_VALID 0x8000      /* Glyph is defined and valid */
	int yOffset;               /* Baseline offset from reference (px) */
	AG_Rect rs;                /* Bounding box in source image (S). */
} AG_GlyphBf;

/* 
 * Mode of colorization for applying the requested foreground color.
 * "Grays" are pixels with a saturation of 0 (where r = g = b exactly).
 */
enum ag_font_bf_colorize_mode {
	AG_FONT_BF_COLORIZE_NONE,  /* Don't colorize anything */
	AG_FONT_BF_COLORIZE_GRAYS, /* Colorize grays and copy non-grays as-is */
	AG_FONT_BF_COLORIZE_ALL,   /* Colorize grays and blend non-grays */
	AG_FONT_BF_COLORIZE_LAST
};

/* Font in Agar Bitmap Font (.agbf) format. */
typedef struct ag_font_bf {
	AG_Font _inherit;                       /* AG_Font -> AG_FontBf */
	Uint flags;
#define AG_FONT_BF_VALID 0x01                   /* Font is open */
	enum ag_font_bf_colorize_mode colorize; /* Colorization mode */
	char *_Nullable name;                   /* Display name */
	AG_Char *_Nullable unicode;             /* Unicode mappings */
	Uint              nUnicode;             /* Unicode mapping count */
	Uint                 nGlyphs;           /* Glyph count */
	AG_GlyphBf *_Nullable glyphs;           /* Glyph array */
	AG_Surface *S;                          /* Source image surface */
	int height;                             /* Reference bbox height (px) */
	int wdRef;                              /* Reference bbox width (px) */
	AG_Rect *_Nullable rects;               /* Source image rectangles */
	Uint              nRects;
	float sizeMin, sizeMax;                 /* Range of point sizes covered */
	int advance;                            /* Global advance */
} AG_FontBf;

#define   AGFONTBF(o)      ((AG_FontBf *)(o))
#define  AGcFONTBF(o)      ((const AG_FontBf *)(o))
#define  AG_FONTBF_ISA(o) (((AGOBJECT(o)->cid & 0xffff0000) >> 16) == 0x0701)
#define  AG_FONTBF_SELF()    AGFONTBF(  AG_OBJECT(0,         "AG_Font:AG_FontBf:*") )
#define  AG_FONTBF_PTR(n)    AGFONTBF(  AG_OBJECT((n),       "AG_Font:AG_FontBf:*") )
#define  AG_FONTBF_NAMED(n)  AGFONTBF(  AG_OBJECT_NAMED((n), "AG_Font:AG_FontBf:*") )
#define AG_cFONTBF_SELF()   AGcFONTBF( AG_cOBJECT(0,         "AG_Font:AG_FontBf:*") )
#define AG_cFONTBF_PTR(n)   AGcFONTBF( AG_cOBJECT((n),       "AG_Font:AG_FontBf:*") )
#define AG_cFONTBF_NAMED(n) AGcFONTBF( AG_cOBJECT_NAMED((n), "AG_Font:AG_FontBf:*") )

__BEGIN_DECLS
extern AG_FontClass agFontBfClass;

AG_FontBf *AG_FontBfNew(const char *, const char *, const AG_FontSpec *,
                        const char *, Uint);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_FONT_BMP */

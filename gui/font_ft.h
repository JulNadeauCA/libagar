/*	Public domain	*/

#ifndef _AGAR_GUI_FONT_FT_H_
#define _AGAR_GUI_FONT_FT_H_

#include <agar/config/have_freetype.h>
#ifdef HAVE_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include <agar/gui/begin.h>

/* Cached glyph */
typedef struct ag_glyph_ft {
	Uint stored;                   /* Which resources are stored */
#define AG_GLYPH_FT_BITMAP  0x01       /* Bitmap rendering */
#define AG_GLYPH_FT_PIXMAP  0x02       /* Pixmap rendering */
#define AG_GLYPH_FT_METRICS 0x10       /* Glyph metrics */
	FT_UInt index;                 /* Glyph index for this character */
	FT_Bitmap bitmap;              /* Cached bitmap */
	FT_Bitmap pixmap;              /* Cached pixmap */
	int xMin, xMax;                /* Bounding box X */
	int yMin, yMax;                /* Bounding box Y */
	int yOffset;                   /* (ascent - yMax) */
	int advance;                   /* Horizontal advance */
	AG_Char cached;                /* Cached character */
	AG_CHAR_PADDING(_pad);
} AG_GlyphFt;

/* FreeType font */
typedef struct ag_font_ft {
	AG_Font _inherit;              /* AG_Font -> AG_FontFt */
	_Nonnull FT_Face face;         /* Typographical font face handle */
	AG_GlyphFt *_Nonnull current;
	AG_GlyphFt cache[256];         /* Transform cache */
	AG_GlyphFt scratch;
	int fixedSize;                 /* For non-scalable formats */
	Uint32 _pad;
} AG_FontFt;

#define   AGFONTFT(o)      ((AG_FontFt *)(o))
#define  AGcFONTFT(o)      ((const AG_FontFt *)(o))
#define  AG_FONTFT_ISA(o) (((AGOBJECT(o)->cid & 0xffff0000) >> 16) == 0x0702)
#define  AG_FONTFT_SELF()    AGFONTFT(  AG_OBJECT(0,         "AG_Font:AG_FontFt:*") )
#define  AG_FONTFT_PTR(n)    AGFONTFT(  AG_OBJECT((n),       "AG_Font:AG_FontFt:*") )
#define  AG_FONTFT_NAMED(n)  AGFONTFT(  AG_OBJECT_NAMED((n), "AG_Font:AG_FontFt:*") )
#define AG_cFONTFT_SELF()   AGcFONTFT( AG_cOBJECT(0,         "AG_Font:AG_FontFt:*") )
#define AG_cFONTFT_PTR(n)   AGcFONTFT( AG_cOBJECT((n),       "AG_Font:AG_FontFt:*") )
#define AG_cFONTFT_NAMED(n) AGcFONTFT( AG_cOBJECT_NAMED((n), "AG_Font:AG_FontFt:*") )

__BEGIN_DECLS
extern AG_FontClass agFontFtClass;

AG_FontFt *AG_FontFtNew(const char *, const char *, const AG_FontSpec *,
                        const char *, Uint);
__END_DECLS

#include <agar/gui/close.h>
#endif /* HAVE_FREETYPE */
#endif /* _AGAR_GUI_FONT_FT */

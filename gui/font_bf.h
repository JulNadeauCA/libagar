/*	Public domain	*/

#ifndef _AGAR_GUI_FONT_BF_H_
#define _AGAR_GUI_FONT_BF_H_

#include <agar/gui/begin.h>

/* Agar Bitmap font glyph. */
typedef struct ag_glyph_bf {
	AG_Char ch;                /* Unicode character mapping */
	AG_CHAR_PADDING(_pad1);
	int x,y;                   /* Coordinates in source image (px) */
	int w,h;                   /* Bounding box dimensions (px) */
	int yOffset;               /* Baseline offset from reference (px) */
	Uint32 _pad2;
} AG_GlyphBf;

/* 
 * Mode of colorization for applying the requested foreground color.
 * "Grays" are pixels with a saturation of 0 (where r = g = b exactly).
 */
enum ag_font_bmp_colorize_mode {
	AG_FONT_BF_COLORIZE_NONE,  /* Don't colorize anything */
	AG_FONT_BF_COLORIZE_GRAYS, /* Colorize grays and copy non-grays as-is */
	AG_FONT_BF_COLORIZE_ALL    /* Colorize grays and blend non-grays */
};

/* Font in Agar Bitmap Font (.agbf) format. */
typedef struct ag_font_bf {
	AG_Font _inherit;                         /* AG_Font -> AG_FontBf */
	Uint flags;
#define AG_FONT_BF_VALID 0x01                     /* Font is open */
	enum ag_font_bmp_colorize_mode colorize;  /* Colorization mode */
	char *_Nullable name;                     /* Display name */
	int underlinePos;                         /* Underline position (px) */
	int underlineThick;                       /* Underline thickness (px) */
	AG_Char *_Nullable unicode;               /* Unicode mappings */
	Uint nUnicode;                            /* Unicode mapping count */
	Uint nGlyphs;                             /* Glyph count */
	AG_GlyphBf *_Nullable glyphs;             /* Glyph array */
	AG_Surface *S;                            /* Source image surface */
} AG_FontBf;

#define AGFONTBF(font) ((AG_FontBf *)(font))

__BEGIN_DECLS
extern AG_FontClass agFontBfClass;

AG_FontBf *AG_FontBfNew(const char *, const char *, const AG_FontSpec *,
                        const char *, Uint);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_FONT_BMP */

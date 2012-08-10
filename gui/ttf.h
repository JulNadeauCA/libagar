/*	Public domain	*/

#ifndef _AGAR_LOADER_TTF_H_
#define _AGAR_LOADER_TTF_H_

#include <agar/config/have_freetype.h>

#ifdef HAVE_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include <agar/gui/begin.h>

typedef struct ag_ttf_glyph {
	int stored;
#define TTF_CACHED_METRICS	0x10
#define TTF_CACHED_BITMAP	0x01
#define TTF_CACHED_PIXMAP	0x02
	FT_UInt index;
	FT_Bitmap bitmap;
	FT_Bitmap pixmap;
	int minx, maxx;
	int miny, maxy;
	int yoffset;
	int advance;
	Uint32 cached;
} AG_TTFGlyph;

typedef struct ag_ttf_font {
	FT_Face	face;
	int height;
	int ascent;
	int descent;
	int lineskip;
	int style;
	int glyph_overhang;
	float glyph_italics;
	int underline_offset;
	int underline_height;

	AG_TTFGlyph *current;
	AG_TTFGlyph cache[256];	/* Transform cache */
	AG_TTFGlyph scratch;
	
	int font_size_family;		/* For non-scalable formats */
} AG_TTFFont;

#define AG_TTF_STYLE_NORMAL	0x00
#define AG_TTF_STYLE_BOLD	0x01
#define AG_TTF_STYLE_ITALIC	0x02
#define AG_TTF_STYLE_UNDERLINE	0x04

__BEGIN_DECLS
struct ag_font;

int  AG_TTFInit(void);
void AG_TTFDestroy(void);
int  AG_TTFOpenFont(struct ag_font *);
void AG_TTFCloseFont(struct ag_font *);
int  AG_TTFFindGlyph(AG_TTFFont *, Uint32, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* HAVE_FREETYPE */
#endif /* _AGAR_LOADER_TTF_H_ */

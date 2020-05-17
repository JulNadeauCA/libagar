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
	AG_Char cached;
#ifdef AG_UNICODE
	Uint8 _pad[4];
#else
	Uint8 _pad[7];
#endif
} AG_TTFGlyph;

typedef struct ag_ttf_font {
	_Nonnull FT_Face face;
	int height;
	int ascent;
	int descent;
	int lineskip;
	Uint style;			/* Font flags (AG_FONT_BOLD, ...) */
	int glyph_overhang;
	double glyph_italics;
	int underline_offset;
	int underline_height;

	AG_TTFGlyph *_Nonnull current;
	AG_TTFGlyph cache[256];		/* Transform cache */
	AG_TTFGlyph scratch;
	
	int font_size_family;		/* For non-scalable formats */
	Uint32 _pad;
} AG_TTFFont;

__BEGIN_DECLS
struct ag_font;

int  AG_TTFInit(void);
void AG_TTFDestroy(void);
int  AG_TTFOpenFont(struct ag_font *_Nonnull, const char *_Nonnull);
void AG_TTFCloseFont(struct ag_font *_Nonnull);
int  AG_TTFFindGlyph(AG_TTFFont *_Nonnull, AG_Char, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* HAVE_FREETYPE */
#endif /* _AGAR_LOADER_TTF_H_ */

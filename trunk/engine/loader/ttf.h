/*	$Csoft: ttf.h,v 1.4 2004/11/26 06:10:55 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_LOADER_TTF_H_
#define _AGAR_LOADER_TTF_H_

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include "begin_code.h"

struct ttf_glyph {
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
};

struct ttf_font {
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

	struct ttf_glyph *current;
	struct ttf_glyph cache[256];	/* Transform cache */
	struct ttf_glyph scratch;
	
	int font_size_family;		/* For non-scalable formats */
};

__BEGIN_DECLS
int		 ttf_init(void);
void		 ttf_destroy(void);
struct ttf_font	*ttf_open_font(const char *, int);
void		 ttf_close_font(struct ttf_font *);
int		 ttf_find_glyph(struct ttf_font *, Uint32, int);

__inline__ int	 ttf_get_font_style(struct ttf_font *);
__inline__ void	 ttf_set_font_style(struct ttf_font *, int);
#define TTF_STYLE_NORMAL	0x00
#define TTF_STYLE_BOLD		0x01
#define TTF_STYLE_ITALIC	0x02
#define TTF_STYLE_UNDERLINE	0x04

__inline__ int	 ttf_font_height(struct ttf_font *);
__inline__ int	 ttf_font_ascent(struct ttf_font *);
__inline__ int	 ttf_font_descent(struct ttf_font *);
__inline__ int	 ttf_font_line_skip(struct ttf_font *);
__inline__ int	 ttf_font_face_fixed_width(struct ttf_font *);
__inline__ char	*ttf_font_face_family(struct ttf_font *);
__inline__ char	*ttf_font_face_style(struct ttf_font *);

int ttf_glyph_metrics(struct ttf_font *, Uint32 , int *, int *, int *, int *,
		      int *);
int ttf_size_text(struct ttf_font *, const char *, int *, int *);
int ttf_size_unicode(struct ttf_font *, const Uint32 *, int *, int *);

SDL_Surface *ttf_render_text_solid(struct ttf_font *, const char *, SDL_Color);
SDL_Surface *ttf_render_unicode_solid(struct ttf_font *, const Uint32 *,
		                      SDL_Color);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_LOADER_TTF_H_ */

/*	$Csoft: ttf.h,v 1.6 2003/06/14 07:15:43 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MEDIA_TTF_H_
#define _AGAR_MEDIA_TTF_H_

typedef struct _ttf_font ttf_font;

int		 ttf_init(void);
void		 ttf_destroy(void);
ttf_font	*ttf_open_font_index(const char *, int, long);
ttf_font	*ttf_open_font(const char *, int);
void		 ttf_close_font(ttf_font *);

int		 ttf_get_font_style(ttf_font *);
void		 ttf_set_font_style(ttf_font *, int);
#define TTF_STYLE_NORMAL	0x00
#define TTF_STYLE_BOLD		0x01
#define TTF_STYLE_ITALIC	0x02
#define TTF_STYLE_UNDERLINE	0x04

int		 ttf_font_height(ttf_font *);
int		 ttf_font_ascent(ttf_font *);
int		 ttf_font_descent(ttf_font *);
int		 ttf_font_line_skip(ttf_font *);
long		 ttf_font_faces(ttf_font *);
int		 ttf_font_face_fixed_width(ttf_font *);
char		*ttf_font_face_family(ttf_font *);
char		*ttf_font_face_style(ttf_font *);

int	 ttf_glyph_metrics(ttf_font *, Uint16, int *, int *, int *,
	     int *, int *);
int	 ttf_size_text(ttf_font *, const char *, int *, int *);
int	 ttf_size_utf8(ttf_font *, const char *, int *, int *);
int	 ttf_size_unicode(ttf_font *, const Uint16 *, int *, int *);

SDL_Surface *ttf_render_glyph_solid(ttf_font *, Uint16, SDL_Color);
SDL_Surface *ttf_render_text_solid(ttf_font *, const char *, SDL_Color);
SDL_Surface *ttf_render_utf8_solid(ttf_font *, const char *, SDL_Color);
SDL_Surface *ttf_render_unicode_solid(ttf_font *, const Uint16 *, SDL_Color);

#endif /* _AGAR_MEDIA_TTF_H_ */

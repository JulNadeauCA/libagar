/*	$Csoft: ttf.h,v 1.3 2004/05/24 03:28:09 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MEDIA_TTF_H_
#define _AGAR_MEDIA_TTF_H_

typedef struct _ttf_font ttf_font;

int		 ttf_init(void);
void		 ttf_destroy(void);
ttf_font	*ttf_open_font(const char *, int);
void		 ttf_close_font(ttf_font *);

__inline__ int	 ttf_get_font_style(ttf_font *);
__inline__ void	 ttf_set_font_style(ttf_font *, int);
#define TTF_STYLE_NORMAL	0x00
#define TTF_STYLE_BOLD		0x01
#define TTF_STYLE_ITALIC	0x02
#define TTF_STYLE_UNDERLINE	0x04

__inline__ int	 ttf_font_height(ttf_font *);
__inline__ int	 ttf_font_ascent(ttf_font *);
__inline__ int	 ttf_font_descent(ttf_font *);
__inline__ int	 ttf_font_line_skip(ttf_font *);
__inline__ int	 ttf_font_face_fixed_width(ttf_font *);
__inline__ char	*ttf_font_face_family(ttf_font *);
__inline__ char	*ttf_font_face_style(ttf_font *);

int  ttf_glyph_metrics(ttf_font *, Uint32 , int *, int *, int *, int *, int *);
int  ttf_size_text(ttf_font *, const char *, int *, int *);
int  ttf_size_unicode(ttf_font *, const Uint32 *, int *, int *);

SDL_Surface *ttf_render_text_solid(ttf_font *, const char *, SDL_Color);
SDL_Surface *ttf_render_unicode_solid(ttf_font *, const Uint32 *, SDL_Color);

#endif /* _AGAR_MEDIA_TTF_H_ */

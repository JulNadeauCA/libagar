/*	$Csoft: ttf.h,v 1.2 2003/03/02 04:13:13 vedge Exp $	*/
/*	Public domain	*/

typedef struct _ttf_font ttf_font;

int	 ttf_init(void);
void	 ttf_destroy(void);

ttf_font	*ttf_open_font(const char *, int);
ttf_font	*ttf_open_font_index(const char *, int, long);
void		 ttf_close_font(ttf_font *);

int	 ttf_get_font_style(ttf_font *);
void	 ttf_set_font_style(ttf_font *, int);
#define TTF_STYLE_NORMAL	0x00
#define TTF_STYLE_BOLD		0x01
#define TTF_STYLE_ITALIC	0x02
#define TTF_STYLE_UNDERLINE	0x04

int	 ttf_font_height(ttf_font *);
int	 ttf_font_ascent(ttf_font *);
int	 ttf_font_descent(ttf_font *);
int	 ttf_font_line_skip(ttf_font *);
long	 ttf_font_faces(ttf_font *);

int	 ttf_font_face_fixed_width(ttf_font *);
char	*ttf_font_face_family_name(ttf_font *);
char	*ttf_font_face_style_name(ttf_font *);

int	 ttf_glyph_metrics(ttf_font *, Uint16, int *, int *, int *,
	     int *, int *);
int	 ttf_size_text(ttf_font *, char *, int *, int *);
int	 ttf_size_utf8(ttf_font *, char *, int *, int *);
int	 ttf_size_unicode(ttf_font *, Uint16 *, int *, int *);

SDL_Surface *ttf_render_text_solid(ttf_font *, char *, SDL_Color);
SDL_Surface *ttf_render_utf8_solid(ttf_font *, char *, SDL_Color);
SDL_Surface *ttf_render_unicode_solid(ttf_font *, Uint16 *, SDL_Color);
SDL_Surface *ttf_render_glyph_solid(ttf_font *, Uint16, SDL_Color);


/*	$Csoft: ttf.h,v 1.4 2002/11/04 08:33:14 vedge Exp $	*/
/*	Public domain	*/

typedef struct _TTF_Font TTF_Font;

int	  TTF_Init(void);
TTF_Font *TTF_OpenFont(const char *, int);
TTF_Font *TTF_OpenFontIndex(const char *, int, long);

int	 TTF_GetFontStyle(TTF_Font *);
void	 TTF_SetFontStyle(TTF_Font *, int);
#define TTF_STYLE_NORMAL	0x00
#define TTF_STYLE_BOLD		0x01
#define TTF_STYLE_ITALIC	0x02
#define TTF_STYLE_UNDERLINE	0x04

int	 TTF_FontHeight(TTF_Font *);
int	 TTF_FontAscent(TTF_Font *);
int	 TTF_FontDescent(TTF_Font *);
int	 TTF_FontLineSkip(TTF_Font *);
long	 TTF_FontFaces(TTF_Font *);

int	 TTF_FontFaceIsFixedWidth(TTF_Font *);
char	*TTF_FontFaceFamilyName(TTF_Font *);
char	*TTF_FontFaceStyleName(TTF_Font *);

int	 TTF_GlyphMetrics(TTF_Font *, unsigned int, int *, int *, int *, int *,
	     int *);
int	 TTF_SizeText(TTF_Font *, char *, int *, int *);
int	 TTF_SizeUTF8(TTF_Font *, char *, int *, int *);
int	 TTF_SizeUNICODE(TTF_Font *, unsigned int *, int *, int *);

SDL_Surface	*TTF_RenderText_Solid(TTF_Font *, char *, SDL_Color);
SDL_Surface	*TTF_RenderUTF8_Solid(TTF_Font *, char *, SDL_Color);
SDL_Surface	*TTF_RenderUNICODE_Solid(TTF_Font *, unsigned int *, SDL_Color);
SDL_Surface	*TTF_RenderGlyph_Solid(TTF_Font *, unsigned int, SDL_Color);

void	 TTF_CloseFont(TTF_Font *);
void	 TTF_Quit(void);


/*	$Csoft$	*/
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

int	 TTF_GlyphMetrics(TTF_Font *, Uint16, int *, int *, int *, int *,\
	     int *);
int	 TTF_SizeText(TTF_Font *, const char *, int *, int *);
int	 TTF_SizeUTF8(TTF_Font *, const char *, int *, int *);
int	 TTF_SizeUNICODE(TTF_Font *, const Uint16 *, int *, int *);

SDL_Surface	*TTF_RenderText_Solid(TTF_Font *, const char *, SDL_Color);
SDL_Surface	*TTF_RenderUTF8_Solid(TTF_Font *, const char *, SDL_Color);
SDL_Surface	*TTF_RenderUNICODE_Solid(TTF_Font *, const Uint16 *, SDL_Color);
SDL_Surface	*TTF_RenderGlyph_Solid(TTF_Font *, Uint16, SDL_Color);
SDL_Surface	*TTF_RenderText_Shaded(TTF_Font *, const char *, SDL_Color,
		    SDL_Color);
SDL_Surface	*TTF_RenderUTF8_Shaded(TTF_Font *, const char *, SDL_Color,
		    SDL_Color);
SDL_Surface	*TTF_RenderUNICODE_Shaded(TTF_Font *, const Uint16 *,
		    SDL_Color, SDL_Color);
SDL_Surface	*TTF_RenderGlyph_Shaded(TTF_Font *, Uint16, SDL_Color,
		    SDL_Color);
SDL_Surface	*TTF_RenderText_Blended(TTF_Font *, const char *, SDL_Color);
SDL_Surface	*TTF_RenderUTF8_Blended(TTF_Font *, const char *, SDL_Color);
SDL_Surface	*TTF_RenderUNICODE_Blended(TTF_Font *, const Uint16 *,
		    SDL_Color);
SDL_Surface	*TTF_RenderGlyph_Blended(TTF_Font *, Uint16, SDL_Color);

void	 TTF_CloseFont(TTF_Font *);
void	 TTF_Quit(void);


/*	$Csoft: rootmap.h,v 1.6 2002/11/28 01:06:50 vedge Exp $	*/
/*	Public domain	*/

struct viewmap;
struct viewport;

void	 rootmap_init(struct viewmap *, int, int);
void	 rootmap_animate(void);
void	 rootmap_draw(void);
void	 rootmap_focus(struct map *);
void	 rootmap_center(struct map *, int, int);
void	 rootmap_scroll(struct map *, int, int);

SDL_Rect	**rootmap_alloc_maprects(int, int);
void		  rootmap_free_maprects(struct viewport *);


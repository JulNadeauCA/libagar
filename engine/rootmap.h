/*	$Csoft: rootmap.h,v 1.2 2002/08/24 23:56:41 vedge Exp $	*/

void	 rootmap_animate(void);
void	 rootmap_draw(void);
void	 rootmap_focus(struct map *);
void	 rootmap_center(struct map *, int, int);
void	 rootmap_scroll(struct map *, int);

SDL_Rect	**rootmap_alloc_maprects(int, int);
SDL_Rect	 *rootmap_alloc_rects(int, int);
void		  rootmap_free_maprects(struct viewport *);


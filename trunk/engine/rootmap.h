/*	$Csoft: rootmap.h,v 1.1 2002/08/24 04:05:23 vedge Exp $	*/

void	 rootmap_animate(void);
void	 rootmap_draw(void);
void	 rootmap_focus(struct map *);
void	 rootmap_center(struct map *, int, int);
void	 rootmap_scroll(struct map *, int);
void	 rootmap_maskfill(SDL_Rect *, int);

int		**rootmap_alloc_mask(int, int);
SDL_Rect	**rootmap_alloc_maprects(int, int);
SDL_Rect	 *rootmap_alloc_rects(int, int);
void		  rootmap_free_mask(struct viewport *);
void		  rootmap_free_maprects(struct viewport *);


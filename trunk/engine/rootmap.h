/*	$Csoft: rootmap.h,v 1.5 2002/11/22 08:56:49 vedge Exp $	*/
/*	Public domain	*/

void	 rootmap_animate(void);
void	 rootmap_draw(void);
void	 rootmap_focus(struct map *);
void	 rootmap_center(struct map *, int, int);
void	 rootmap_scroll(struct map *, int, int);

struct viewport;

SDL_Rect	**rootmap_alloc_maprects(int, int);
void		  rootmap_free_maprects(struct viewport *);


/*	$Csoft: rootmap.h,v 1.8 2003/04/25 09:47:05 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ROOTMAP_H_
#define _AGAR_ROOTMAP_H_
#include "begin_code.h"

struct viewmap;
struct viewport;

__BEGIN_DECLS
extern DECLSPEC void	 rootmap_init(struct viewmap *, int, int);
extern DECLSPEC void	 rootmap_animate(void);
extern DECLSPEC void	 rootmap_draw(void);
extern DECLSPEC void	 rootmap_focus(struct map *);
extern DECLSPEC void	 rootmap_center(struct map *, int, int);
extern DECLSPEC void	 rootmap_scroll(struct map *, int, int);
extern __inline__ void	 rootmap_redraw(void);

extern DECLSPEC SDL_Rect **rootmap_alloc_maprects(int, int);
extern DECLSPEC void	   rootmap_free_maprects(struct viewport *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_ROOTMAP_H_ */

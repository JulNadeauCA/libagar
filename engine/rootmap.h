/*	$Csoft: rootmap.h,v 1.9 2003/06/06 02:50:19 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ROOTMAP_H_
#define _AGAR_ROOTMAP_H_
#include "begin_code.h"

struct viewmap;
struct viewport;

__BEGIN_DECLS
void		  rootmap_init(struct viewmap *, int, int);
void		  rootmap_animate(void);
void		  rootmap_draw(void);
void		  rootmap_focus(struct map *);
void		  rootmap_center(struct map *, int, int);
void		  rootmap_scroll(struct map *, int, int);
__inline__ void	  rootmap_redraw(void);
SDL_Rect	**rootmap_alloc_maprects(int, int);
void		  rootmap_free_maprects(struct viewport *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_ROOTMAP_H_ */

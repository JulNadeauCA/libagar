/*	$Csoft: rootmap.h,v 1.11 2004/02/25 18:12:18 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ROOTMAP_H_
#define _AGAR_ROOTMAP_H_
#include "begin_code.h"

struct viewmap;
struct viewport;

enum {
	ROOTMAP_N,
	ROOTMAP_S,
	ROOTMAP_W,
	ROOTMAP_E
};

__BEGIN_DECLS
void		  rootmap_init(struct viewmap *, int, int);
void		  rootmap_animate(void);
void		  rootmap_draw(void);
void		  rootmap_focus(struct map *);
void		  rootmap_center(struct map *, int, int);
void		  rootmap_scroll(struct map *, int, int);
__inline__ void	  rootmap_redraw(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_ROOTMAP_H_ */

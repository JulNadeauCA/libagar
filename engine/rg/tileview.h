/*	$Csoft: tileview.h,v 1.9 2003/06/18 00:47:04 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_BG_TILEVIEW_H_
#define _AGAR_BG_TILEVIEW_H_

#include <engine/widget/widget.h>
#include <engine/rg/tileset.h>

#include "begin_code.h"

struct tileview {
	struct widget wid;
	struct tileset *ts;
	struct tile *tile;
};

__BEGIN_DECLS
struct tileview	*tileview_new(void *, struct tileset *, struct tile *);

void	 tileview_init(struct tileview *, struct tileset *, struct tile *);
void	 tileview_destroy(void *);
void	 tileview_draw(void *);
void	 tileview_scale(void *, int, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_BG_TILEVIEW_H */

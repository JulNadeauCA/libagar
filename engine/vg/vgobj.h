/*	$Csoft: vgobj.h,v 1.1 2004/04/11 03:27:45 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VGOBJ_H_
#define _AGAR_VGOBJ_H_

#include <engine/vg/vg.h>

#include "begin_code.h"

struct vgobj {
	struct object obj;
	struct vg *vg;
};

__BEGIN_DECLS
void		 vgobj_init(void *, const char *);
void		 vgobj_destroy(void *);
struct window	*vgobj_edit(void *);
int		 vgobj_load(void *, struct netbuf *);
int		 vgobj_save(void *, struct netbuf *);

void	 vg_geo_changed(int, union evarg *);
void	 vg_changed(int, union evarg *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_VGOBJ_H_ */

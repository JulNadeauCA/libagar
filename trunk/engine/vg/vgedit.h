/*	$Csoft: vgedit.h,v 1.1 2004/03/30 16:05:42 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VGEDIT_H_
#define _AGAR_VGEDIT_H_

#include <engine/vg/vg.h>

#include "begin_code.h"

struct vgedit {
	struct object obj;
	struct vg *vg;
};

__BEGIN_DECLS
void		 vgedit_init(void *, const char *);
void		 vgedit_destroy(void *);
struct window	*vgedit_edit(void *);
int		 vgedit_load(void *, struct netbuf *);
int		 vgedit_save(void *, struct netbuf *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_VGEDIT_H_ */

/*	$Csoft: drawing.h,v 1.1 2004/11/24 05:34:56 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_DRAWING_H_
#define _AGAR_VG_DRAWING_H_

#include <engine/vg/vg.h>

#include "begin_code.h"

struct drawing {
	struct object obj;
	struct vg *vg;
};

__BEGIN_DECLS
void	 drawing_init(void *, const char *);
void	 drawing_reinit(void *);
void	 drawing_destroy(void *);
void	*drawing_edit(void *);
int	 drawing_load(void *, struct netbuf *);
int	 drawing_save(void *, struct netbuf *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_VG_DRAWING_H_ */

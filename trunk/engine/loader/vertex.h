/*	$Csoft: color.h,v 1.1 2004/04/30 07:00:23 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_LOADER_VERTEX_H_
#define _AGAR_LOADER_VERTEX_H_
#include "begin_code.h"

__BEGIN_DECLS
void	read_vertex(struct netbuf *, struct vg_vertex *);
void	write_vertex(struct netbuf *, struct vg_vertex *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_LOADER_VERTEX_H_ */

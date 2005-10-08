/*	$Csoft: vertex.h,v 1.1 2004/05/01 00:17:27 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_LOADER_VERTEX_H_
#define _AGAR_LOADER_VERTEX_H_
#include "begin_code.h"

__BEGIN_DECLS
void	AG_ReadVertex(AG_Netbuf *, VG_Vtx *);
void	AG_WriteVertex(AG_Netbuf *, VG_Vtx *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_LOADER_VERTEX_H_ */

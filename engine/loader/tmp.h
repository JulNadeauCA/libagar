/*	$Csoft: tmp.h,v 1.1 2005/09/06 04:13:05 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_LOADER_TMP_H_
#define _AGAR_LOADER_TMP_H_
#include "begin_code.h"

__BEGIN_DECLS
AG_Netbuf *AG_TmpOpen(const char *);
void       AG_TmpClose(AG_Netbuf *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_LOADER_NETBUF_H_ */

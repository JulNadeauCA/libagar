/*	$Csoft$	*/
/*	Public domain	*/

#ifndef _AGAR_LOADER_TMP_H_
#define _AGAR_LOADER_TMP_H_
#include "begin_code.h"

__BEGIN_DECLS
struct netbuf *tmp_open(const char *);
void	       tmp_close(struct netbuf *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_LOADER_NETBUF_H_ */

/*	$Csoft$	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
Uint16	*read_unicode(struct netbuf *);
void	 write_unicode(struct netbuf *, const Uint16 *);
size_t	 copy_unicode(Uint16 *, struct netbuf *, size_t);
__END_DECLS

#include "close_code.h"

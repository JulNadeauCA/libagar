/*	$Csoft: unicode.h,v 1.1 2003/06/19 01:53:38 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

#define UNICODE_STRING_MAX	16384

__BEGIN_DECLS
Uint16	*read_unicode(struct netbuf *);
void	 write_unicode(struct netbuf *, const Uint16 *);
size_t	 copy_unicode(Uint16 *, struct netbuf *, size_t);
__END_DECLS

#include "close_code.h"

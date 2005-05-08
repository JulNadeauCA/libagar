/*	$Csoft: unicode.h,v 1.8 2003/10/09 22:39:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_UNICODE_H_
#define _AGAR_UNICODE_H_
#include "begin_code.h"

enum unicode_conv {
	UNICODE_FROM_US_ASCII,		/* US-ASCII -> UCS-4 */
	UNICODE_FROM_UTF8,		/* UTF-8 -> UCS-4 */
	UNICODE_TO_UTF8			/* UCS-4 -> UTF-8 */
};

__BEGIN_DECLS
Uint32	*unicode_import(enum unicode_conv, const char *);
ssize_t	 unicode_export(enum unicode_conv, char *, const Uint32 *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 4);
size_t	 unicode_copy(enum unicode_conv, const char *, Uint32 *, size_t);

size_t	 ucs4_len(const Uint32 *);
size_t	 ucs4_lcpy(Uint32 *, const Uint32 *, size_t);
size_t	 ucs4_lcat(Uint32 *, const Uint32 *, size_t);
Uint32	*ucs4_dup(const Uint32 *);
Uint32	*ucs4_sep(Uint32 **, const Uint32 *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_UNICODE_H_ */

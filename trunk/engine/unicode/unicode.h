/*	$Csoft: unicode.h,v 1.6 2003/09/01 07:42:35 vedge Exp $	*/
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
ssize_t	 unicode_export(enum unicode_conv, char *, const Uint32 *, size_t);
size_t	 ucs4_len(const Uint32 *);
size_t	 ucs4_lcpy(Uint32 *, const Uint32 *, size_t);
size_t	 ucs4_lcat(Uint32 *, const Uint32 *, size_t);
Uint32	*ucs4_dup(const Uint32 *);
Uint32	*ucs4_sep(Uint32 **, const Uint32 *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_UNICODE_H_ */

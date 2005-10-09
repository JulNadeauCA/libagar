/*	$Csoft: unicode.h,v 1.9 2005/05/08 09:21:37 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_UNICODE_H_
#define _AGAR_UNICODE_H_
#include "begin_code.h"

enum ag_unicode_conv {
	AG_UNICODE_FROM_US_ASCII,	/* US-ASCII -> UCS-4 */
	AG_UNICODE_FROM_UTF8,		/* UTF-8 -> UCS-4 */
	AG_UNICODE_TO_UTF8		/* UCS-4 -> UTF-8 */
};

__BEGIN_DECLS
Uint32	*AG_ImportUnicode(enum ag_unicode_conv, const char *);
ssize_t	 AG_ExportUnicode(enum ag_unicode_conv, char *, const Uint32 *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 4);
size_t	 AG_CopyUnicode(enum ag_unicode_conv, const char *, Uint32 *, size_t);

size_t	 AG_UCS4Len(const Uint32 *);
size_t	 AG_UCS4Copy(Uint32 *, const Uint32 *, size_t);
size_t	 AG_UCS4Cat(Uint32 *, const Uint32 *, size_t);
Uint32	*AG_UCS4Dup(const Uint32 *);
Uint32	*AG_UCS4Sep(Uint32 **, const Uint32 *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_UNICODE_H_ */

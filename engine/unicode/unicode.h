/*	$Csoft: unicode.h,v 1.5 2003/08/31 11:58:09 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_UNICODE_H_
#define _AGAR_UNICODE_H_
#include "begin_code.h"

enum unicode_conv {
	UNICODE_FROM_ASCII,		/* ASCII -> UCS-4 */
	UNICODE_FROM_UTF8,		/* UTF-8 -> UCS-4 */
	UNICODE_TO_UTF8			/* UCS-4 -> UTF-8 */
};

__BEGIN_DECLS
void	 unicode_init(void);
void	 unicode_destroy(void);
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

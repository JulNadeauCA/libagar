/*	$Csoft: unicode.h,v 1.2 2003/06/14 22:07:46 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_UNICODE_H_
#define _AGAR_UNICODE_H_
#include "begin_code.h"

enum unicode_conv {
	UNICODE_FROM_ASCII,		/* ASCII text */
	UNICODE_FROM_LATIN1,		/* Latin-1 text */
	UNICODE_FROM_UTF8		/* UTF-8 text */
};

/*
 * ZERO WIDTH NO-BREAKSPACE (Unicode byte order mark).
 * Load routines must handle byteswapping and strip the BOM.
 */
#define UNICODE_BOM_NATIVE	0xfeff
#define UNICODE_BOM_SWAPPED	0xfffe

__BEGIN_DECLS
void	 unicode_init(void);
void	 unicode_destroy(void);

__inline__ Uint16 *unicode_import(enum unicode_conv, const char *);
void		   unicode_convert(enum unicode_conv, Uint16 *, const char *,
		                   size_t);

__inline__ size_t ucslen(const Uint16 *);
size_t		  ucslcpy(Uint16 *, const Uint16 *, size_t);
size_t		  ucslcat(Uint16 *, const Uint16 *, size_t);
Uint16		 *ucsdup(const Uint16 *);
Uint16		 *ucssep(Uint16 **, const Uint16 *);
#ifdef DEBUG
void		  ucsdump(const Uint16 *);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_UNICODE_H_ */

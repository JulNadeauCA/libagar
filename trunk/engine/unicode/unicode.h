/*	$Csoft$	*/
/*	Public domain	*/

#ifndef _AGAR_UNICODE_H_
#define _AGAR_UNICODE_H_
#include "begin_code.h"

enum unicode_conv {
	UNICODE_FROM_ASCII,		/* ASCII text */
	UNICODE_FROM_LATIN1,		/* Latin-1 text */
	UNICODE_FROM_UTF8		/* UTF-8 text */
};

/* ZERO WIDTH NO-BREAKSPACE (Unicode byte order mark) */
#define UNICODE_BOM_NATIVE	0xfeff
#define UNICODE_BOM_SWAPPED	0xfffe

extern int unicode_byteswapped;

__BEGIN_DECLS
extern DECLSPEC void	 unicode_init(void);
extern DECLSPEC void	 unicode_destroy(void);

extern DECLSPEC void	  unicode_convert(enum unicode_conv, Uint16 *,
			                  const char *, size_t);
extern __inline__ Uint16 *unicode_import(enum unicode_conv, const char *);

extern __inline__ size_t ucslen(const Uint16 *);
extern DECLSPEC size_t	 ucslcpy(Uint16 *, const Uint16 *, size_t);
extern DECLSPEC size_t	 ucslcat(Uint16 *, const Uint16 *, size_t);
extern DECLSPEC Uint16	*ucsdup(const Uint16 *);
extern DECLSPEC Uint16	*ucssep(Uint16 **, const Uint16 *);
extern DECLSPEC void	 ucsdump(const Uint16 *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_UNICODE_H_ */

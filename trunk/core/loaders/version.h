/*	Public domain	*/

#ifndef _AGAR_VERSION_H_
#define _AGAR_VERSION_H_
#include "begin_code.h"

typedef struct ag_version {
	Uint32	 major;
	Uint32	 minor;
} AG_Version;

#define AG_VERSION_NAME_MAX	48
#define AG_VERSION_MAX		(AG_VERSION_NAME_MAX+8)

__BEGIN_DECLS
int	AG_ReadVersion(AG_Netbuf *, const char *, const AG_Version *,
	               AG_Version *);
void	AG_WriteVersion(AG_Netbuf *, const char *, const AG_Version *);
int	AG_ReadObjectVersion(AG_Netbuf *, void *, AG_Version *);
void	AG_WriteObjectVersion(AG_Netbuf *, void *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_VERSION_H_ */

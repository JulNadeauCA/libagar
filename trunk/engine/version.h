/*	$Csoft: version.h,v 1.7 2003/04/12 01:45:31 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VERSION_H_
#define _AGAR_VERSION_H_
#include "begin_code.h"

struct version {
	char	*name;
	Uint32	 major;
	Uint32	 minor;
};

#define VERSION_USER_MAX	128
#define VERSION_HOST_MAX	256

__BEGIN_DECLS
extern DECLSPEC int	version_read(struct netbuf *, const struct version *,
			             struct version *);
extern DECLSPEC void	version_write(struct netbuf *, const struct version *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_VERSION_H_ */

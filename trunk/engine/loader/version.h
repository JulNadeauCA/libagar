/*	$Csoft: version.h,v 1.1 2003/06/19 01:53:38 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VERSION_H_
#define _AGAR_VERSION_H_
#include "begin_code.h"

struct version {
	char	*name;
	Uint32	 major;
	Uint32	 minor;
};

#define VERSION_NAME_MAX	48
#define VERSION_MAX		(VERSION_NAME_MAX+8)

__BEGIN_DECLS
int	version_read(struct netbuf *, const struct version *, struct version *);
void	version_write(struct netbuf *, const struct version *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_VERSION_H_ */

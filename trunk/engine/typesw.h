/*	$Csoft: typesw.h,v 1.1 2003/05/18 00:20:05 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_TYPESW_H_
#define _AGAR_TYPESW_H
#include "begin_code.h"

struct object_type {
	char	 type[32];
	size_t	 size;
	const struct object_ops *ops;
};

extern struct object_type *typesw;
extern int ntypesw;

__BEGIN_DECLS
void	typesw_init(void);
void	typesw_destroy(void);
void	typesw_register(const char *, size_t, const struct object_ops *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_TYPESW_H_ */

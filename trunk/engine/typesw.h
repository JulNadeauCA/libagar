/*	$Csoft: typesw.h,v 1.2 2004/02/29 17:34:24 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_TYPESW_H_
#define _AGAR_TYPESW_H
#include "begin_code.h"

struct object_type {
	char type[32];
	size_t size;
	const struct object_ops *ops;
	int icon;
};

extern struct object_type *typesw;
extern int ntypesw;

__BEGIN_DECLS
void	typesw_init(void);
void	typesw_destroy(void);
void	typesw_register(const char *, size_t, const struct object_ops *, int);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_TYPESW_H_ */

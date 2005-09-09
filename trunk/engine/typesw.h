/*	$Csoft: typesw.h,v 1.3 2004/04/11 03:28:18 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_TYPESW_H_
#define _AGAR_TYPESW_H
#include "begin_code.h"

struct object_type {
	char type[OBJECT_TYPE_MAX];
	size_t size;
	const struct object_ops *ops;
	int icon;
};

extern struct object_type *typesw;
extern int ntypesw;

__BEGIN_DECLS
void typesw_init(void);
void typesw_destroy(void);
void typesw_register(const char *, size_t, const struct object_ops *, int);
__inline__ struct object_type *typesw_find(const char *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_TYPESW_H_ */

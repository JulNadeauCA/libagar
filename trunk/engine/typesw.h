/*	$Csoft: typesw.h,v 1.4 2005/09/09 02:11:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_TYPESW_H_
#define _AGAR_TYPESW_H
#include "begin_code.h"

typedef struct ag_object_type {
	char type[AG_OBJECT_TYPE_MAX];
	size_t size;
	const AG_ObjectOps *ops;
	int icon;
} AG_ObjectType;

extern AG_ObjectType *agTypes;
extern int	     agnTypes;

__BEGIN_DECLS
void AG_InitTypeSw(void);
void AG_DestroyTypeSw(void);
void AG_RegisterType(const char *, size_t, const AG_ObjectOps *, int);
__inline__ AG_ObjectType *AG_FindType(const char *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_TYPESW_H_ */

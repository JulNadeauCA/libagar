/*	Public domain	*/

#ifndef _AGAR_TYPESW_H_
#define _AGAR_TYPESW_H
#include "begin_code.h"

typedef struct ag_object_type {
	const AG_ObjectOps *ops;
	int icon;
} AG_ObjectType;

extern AG_ObjectType *agTypes;
extern int	     agnTypes;

__BEGIN_DECLS
void AG_InitTypeSw(void);
void AG_DestroyTypeSw(void);
void AG_RegisterType(const AG_ObjectOps *, int);
__inline__ AG_ObjectType *AG_FindType(const char *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_TYPESW_H_ */

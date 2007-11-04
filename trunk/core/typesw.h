/*	Public domain	*/

#ifndef _AGAR_TYPESW_H_
#define _AGAR_TYPESW_H
#include "begin_code.h"

__BEGIN_DECLS
extern AG_ObjectOps **agClassTbl;
extern int            agClassCount;

void	 AG_InitClassTbl(void);
void	 AG_DestroyClassTbl(void);
void	 AG_RegisterClass(const void *);

static __inline__ const AG_ObjectOps *
AG_FindClass(const char *type)
{
	int i;

	for (i = 0; i < agClassCount; i++) {
		if (strcmp(agClassTbl[i]->type, type) == 0)
			return (agClassTbl[i]);
	}
	AG_SetError("No such class: %s", type);
	return (NULL);
}
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_TYPESW_H_ */

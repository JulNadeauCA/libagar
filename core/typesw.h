/*	Public domain	*/

#ifndef _AGAR_TYPESW_H_
#define _AGAR_TYPESW_H
#include "begin_code.h"

__BEGIN_DECLS
void	 AG_InitClassTbl(void);
void	 AG_DestroyClassTbl(void);
void	 AG_RegisterClass(void *);
void     AG_UnregisterClass(void *);

static __inline__ AG_ObjectClass *
AG_FindClass(const char *name)
{
	int i;

	for (i = 0; i < agClassCount; i++) {
		if (strcmp(agClassTbl[i]->name, name) == 0)
			return (agClassTbl[i]);
	}
	AG_SetError("No such class: %s", name);
	return (NULL);
}
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_TYPESW_H_ */

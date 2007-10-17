/*	Public domain	*/

#ifndef _AGAR_TYPESW_H_
#define _AGAR_TYPESW_H
#include "begin_code.h"

__BEGIN_DECLS
extern AG_ObjectOps **agClassTbl;
extern int            agClassCount;

void			 AG_InitClassTbl(void);
void			 AG_DestroyClassTbl(void);
void			 AG_RegisterClass(const void *);
const AG_ObjectOps	*AG_FindClass(const char *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_TYPESW_H_ */

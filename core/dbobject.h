/*	Public domain	*/

#ifndef _AGAR_CORE_DBOBJECT_H_
#define _AGAR_CORE_DBOBJECT_H_
#include <agar/core/begin.h>

typedef struct ag_dbobject {
	struct ag_object obj;
} AG_DbObject;

#define AGDBOBJECT(p) ((AG_DbObject *)(p))

__BEGIN_DECLS
extern AG_ObjectClass agDbObjectClass;

AG_DbObject *_Nullable AG_DbObjectNew(void);
int AG_DbObjectLoad(void *_Nonnull, AG_Db *_Nonnull, const char *_Nonnull);
int AG_DbObjectSave(void *_Nonnull, AG_Db *_Nonnull);
int AG_DbObjectInsert(AG_Db *_Nonnull, void *_Nonnull);
int AG_DbObjectDelete(AG_Db *_Nonnull, const char *_Nonnull);
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_DBOBJECT_H_ */

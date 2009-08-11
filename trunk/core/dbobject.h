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

AG_DbObject *AG_DbObjectNew(void);
AG_List     *AG_DbObjectList(AG_Db *);
int          AG_DbObjectLoad(void *, AG_Db *, const char *);
int          AG_DbObjectSave(void *, AG_Db *);
int          AG_DbObjectInsert(AG_Db *, void *);
int          AG_DbObjectDelete(AG_Db *, const char *);
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_DBOBJECT_H_ */

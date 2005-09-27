/*	$Csoft: objmgr.h,v 1.8 2005/09/17 15:22:23 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJMGR_H_
#define _AGAR_OBJMGR_H_
#include "begin_code.h"

__BEGIN_DECLS
AG_Window	*AG_ObjMgrWindow(void);
void		 AG_ObjMgrInit(void);
void		 AG_ObjMgrDestroy(void);
void		 AG_ObjMgrReopen(AG_Object *);
void		 AG_ObjMgrOpenData(void *, int);
void		 AG_ObjMgrCloseData(void *);
void		 AG_ObjMgrOpenGeneric(AG_Object *);
void		 AG_ObjMgrQuitDlg(void *);
void		 AG_ObjMgrSaveTo(void *);
void		 AG_ObjMgrGenericMenu(void *, void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_OBJMGR_H_ */

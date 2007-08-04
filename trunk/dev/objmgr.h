/*	$Csoft: objmgr.h,v 1.8 2005/09/17 15:22:23 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_DEV_BROWSER_H_
#define _AGAR_DEV_BROWSER_H_
#include "begin_code.h"

struct ag_window;

__BEGIN_DECLS
struct ag_window *DEV_BrowserWindow(void);
void		  DEV_BrowserInit(void);
void		  DEV_BrowserDestroy(void);
void		  DEV_BrowserReopen(AG_Object *);
void		  DEV_BrowserOpenData(void *);
void		  DEV_BrowserCloseData(void *);
void		  DEV_BrowserOpenGeneric(AG_Object *);
void		  DEV_BrowserQuitDlg(void *);
void		  DEV_BrowserSaveTo(void *, const char *);
void		  DEV_BrowserLoadFrom(void *, const char *);
void		  DEV_BrowserGenericMenu(void *, void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_DEV_BROWSER_H_ */

/*	Public domain	*/

#ifndef _AGAR_DEV_DEV_H_
#define _AGAR_DEV_DEV_H_

#include <agar/config/ag_network.h>
#include <agar/config/ag_threads.h>
#include <agar/config/have_jpeg.h>

#include <agar/dev/begin.h>

struct ag_menu_item;

__BEGIN_DECLS
void	   DEV_InitSubsystem(Uint);
void	   DEV_ToolMenu(struct ag_menu_item *);

#if defined(AG_NETWORK) && defined(AG_THREADS) && defined(HAVE_JPEG)
AG_Window *DEV_ScreenshotUploader(void);
#endif
#if defined(AG_NETWORK) && defined(AG_THREADS)
AG_Window *DEV_DebugServer(void);
int	   DEV_DebugServerStart(void);
#endif
AG_Window *DEV_TimerInspector(void);
AG_Window *DEV_UnicodeBrowser(void);
AG_Window *DEV_DisplaySettings(void);
AG_Window *DEV_CPUInfo(void);

AG_Window *DEV_Browser(void *);
void	   DEV_BrowserInit(void *);
void	   DEV_BrowserDestroy(void);
void	   DEV_BrowserOpenData(void *);
void	   DEV_BrowserCloseData(void *);
void	   DEV_BrowserOpenGeneric(AG_Object *);
void	   DEV_BrowserSaveTo(void *, const char *);
void	   DEV_BrowserLoadFrom(void *, const char *);
void	   DEV_BrowserGenericMenu(void *, void *);

void	   DEV_ConfigShow(void);
void	  *DEV_ObjectEdit(void *);

AG_Window *DEV_ClassInfo(void);
__END_DECLS

#include <agar/dev/close.h>
#endif	/* _AGAR_DEV_DEV_H_ */

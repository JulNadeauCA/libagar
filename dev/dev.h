/*	Public domain	*/

#ifndef _AGAR_DEV_DEV_H_
#define _AGAR_DEV_DEV_H_

#include <agar/dev/begin.h>

struct ag_menu_item;

__BEGIN_DECLS
void DEV_InitSubsystem(Uint);
void DEV_DestroySubsystem(void);
void DEV_ToolMenu(struct ag_menu_item *_Nonnull);

#ifdef AG_TIMERS
AG_Window *_Nullable DEV_TimerInspector(void);
#endif
#ifdef AG_UNICODE
AG_Window *_Nullable DEV_UnicodeBrowser(void);
#endif
AG_Window *_Nullable DEV_DisplaySettings(void);
AG_Window *_Nullable DEV_CPUInfo(void);
#ifdef AG_TIMERS
AG_Window *_Nullable DEV_Browser(void *_Nonnull);
void	             DEV_BrowserInit(void *_Nonnull);
void	             DEV_BrowserDestroy(void);
void	             DEV_BrowserOpenData(void *_Nonnull);
void	             DEV_BrowserCloseData(void *_Nonnull);
void	             DEV_BrowserOpenGeneric(AG_Object *_Nonnull);
AG_Window *_Nullable DEV_BrowserSaveToDlg(void *_Nonnull, const char *_Nonnull);
AG_Window *_Nullable DEV_BrowserLoadFromDlg(void *_Nonnull, const char *_Nonnull);
void                 DEV_BrowserGenericMenu(void *_Nonnull, void *_Nonnull,
                                            AG_Window *_Nonnull);
AG_Window *_Nullable DEV_ClassInfo(void);
AG_Window *_Nullable DEV_FontInfo(void);
#endif /* AG_TIMERS */
void                 DEV_ConfigShow(void);
void *_Nonnull       DEV_ObjectEdit(void *_Nonnull);
__END_DECLS

#include <agar/dev/close.h>
#endif	/* _AGAR_DEV_DEV_H_ */

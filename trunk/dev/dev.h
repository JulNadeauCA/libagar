/*	Public domain	*/

#ifndef _AGAR_DEV_DEV_H_
#define _AGAR_DEV_DEV_H_

#include <config/network.h>
#include <config/threads.h>
#include <config/have_jpeg.h>

#include "begin_code.h"

struct ag_menu_item;

__BEGIN_DECLS
void	   DEV_InitSubsystem(Uint);
void	   DEV_ToolMenu(struct ag_menu_item *);

#if defined(NETWORK) && defined(THREADS) && defined(HAVE_JPEG)
AG_Window *DEV_ScreenshotUploader(void);
#endif
#if defined(NETWORK) && defined(THREADS)
AG_Window *DEV_DebugServer(void);
int	   DEV_DebugServerStart(void);
#endif
AG_Window *DEV_LeakDetector(void);
AG_Window *DEV_TimerInspector(void);
AG_Window *DEV_UnicodeBrowser(void);
AG_Window *DEV_DisplaySettings(void);
AG_Window *DEV_GuiDebugger(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_DEV_DEV_H_ */

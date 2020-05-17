/*	Public domain	*/

#ifndef _AGAR_MICRO_GUI_H_
#define _AGAR_MICRO_GUI_H_

#include <agar/micro/begin.h>

#define MA_ZOOM_MIN 0
#define MA_ZOOM_MAX 0
#define MA_ZOOM_1_1 0			/* Starts at 100% */
#ifndef MA_ZOOM_DEFAULT
#define MA_ZOOM_DEFAULT MA_ZOOM_1_1
#endif

__BEGIN_DECLS
extern Uint8 maGUI, maRenderingContext;
extern Uint8 maKbdDelay_2;
extern Uint8 maKbdRepeat;
extern Uint8 maMouseDblclickDelay, maPageIncrement;

Sint8 MA_InitGraphics(const char *_Nullable);
void  MA_DestroyGraphics(void);
void  MA_InitGUI(void);
void  MA_DestroyGUI(void);
Sint8 MA_InitGUIGlobals(void);
void  MA_DestroyGUIGlobals(void);
__END_DECLS

#include <agar/micro/close.h>
#endif /* _AGAR_MICRO_GUI_H_ */

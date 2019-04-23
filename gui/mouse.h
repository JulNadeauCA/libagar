/*	Public domain	*/

#ifndef _AGAR_GUI_MOUSE_H_
#define _AGAR_GUI_MOUSE_H_
#include <agar/gui/input_device.h>
#include <agar/gui/begin.h>

typedef enum ag_mouse_button {
	AG_MOUSE_NONE		= 0x00,
	AG_MOUSE_LEFT		= 0x01,
	AG_MOUSE_MIDDLE		= 0x02,
	AG_MOUSE_RIGHT		= 0x03,
	AG_MOUSE_WHEELUP	= 0x04,
	AG_MOUSE_WHEELDOWN	= 0x05,
	AG_MOUSE_X1		= 0x06,
	AG_MOUSE_X2		= 0x07,
	AG_MOUSE_ANY		= 0xff
} AG_MouseButton;

typedef enum ag_mouse_button_action {
	AG_BUTTON_PRESSED,
	AG_BUTTON_RELEASED
} AG_MouseButtonAction;

#define AG_MOUSE_BUTTON(b)	(1<<((b)-1))
#define AG_MOUSE_LMASK		AG_MOUSE_BUTTON(1)
#define AG_MOUSE_MMASK		AG_MOUSE_BUTTON(2)
#define AG_MOUSE_RMASK		AG_MOUSE_BUTTON(3)

struct ag_window;

typedef struct ag_mouse {
	struct ag_input_device _inherit;
	Uint nButtons;		/* Button count (0 = unknown) */
	Uint btnState;		/* Last button state (AG_MouseButton) */
	int x, y;		/* Last cursor position */
	int xRel, yRel;		/* Last relative motion */
} AG_Mouse;

__BEGIN_DECLS
extern AG_ObjectClass agMouseClass;

AG_Mouse *_Nullable AG_MouseNew(void *_Nonnull, const char *_Nonnull);

Uint8 AG_MouseGetState(AG_Mouse *_Nonnull, int *_Nullable, int *_Nullable);

void AG_MouseMotionUpdate(AG_Mouse *_Nonnull, int,int);
void AG_MouseCursorUpdate(struct ag_window *_Nonnull, int,int);
void AG_MouseButtonUpdate(AG_Mouse *_Nonnull, AG_MouseButtonAction, int);
void AG_ProcessMouseMotion(struct ag_window *_Nonnull, int,int, int,int, Uint);
void AG_ProcessMouseButtonUp(struct ag_window *_Nonnull, int,int, AG_MouseButton);
void AG_ProcessMouseButtonDown(struct ag_window *_Nonnull, int,int,
                               AG_MouseButton);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_MOUSE_H_ */

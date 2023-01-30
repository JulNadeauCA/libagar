/*	Public domain	*/

#ifndef _AGAR_GUI_CONTROLLER_H_
#define _AGAR_GUI_CONTROLLER_H_
#include <agar/gui/input_device.h>
#include <agar/gui/begin.h>

struct ag_window;
struct ag_driver_event;

#define AG_CTRL_PRESSED  1
#define AG_CTRL_RELEASED 0

typedef enum {
	AG_CTRL_TYPE_UNKNOWN = 0,
	AG_CTRL_TYPE_XBOX360 = 1,
	AG_CTRL_TYPE_XBOXONE = 2,
	AG_CTRL_TYPE_PS3 = 3,
	AG_CTRL_TYPE_PS4 = 4,
	AG_CTRL_TYPE_NINTENDO_SWITCH_PRO = 5,
	AG_CTRL_TYPE_VIRTUAL = 6,
	AG_CTRL_TYPE_PS5 = 7,
	AG_CTRL_TYPE_AMAZON_LUNA = 8,
	AG_CTRL_TYPE_GOOGLE_STADIA = 9,
	AG_CTRL_TYPE_NVIDIA_SHIELD = 10,
	AG_CTRL_TYPE_NINTENDO_SWITCH_JOYCON_LEFT = 11,
	AG_CTRL_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT = 12,
	AG_CTRL_TYPE_NINTENDO_SWITCH_JOYCON_PAIR = 13,
	AG_CTRL_TYPE_LAST
} AG_ControllerType;

typedef enum {
	AG_CTRL_AXIS_INVALID = -1,
	AG_CTRL_AXIS_LEFT_X = 0,
	AG_CTRL_AXIS_LEFT_Y = 1,
	AG_CTRL_AXIS_RIGHT_X = 2,
	AG_CTRL_AXIS_RIGHT_Y = 3,
	AG_CTRL_AXIS_TRIGGER_LEFT = 4,
	AG_CTRL_AXIS_TRIGGER_RIGHT = 5,
	AG_CTRL_AXIS_MAX
} AG_ControllerAxis;

typedef enum {
	AG_CTRL_BUTTON_INVALID = -1,
	AG_CTRL_BUTTON_A = 0,
	AG_CTRL_BUTTON_B = 1,
	AG_CTRL_BUTTON_X = 2,
	AG_CTRL_BUTTON_Y = 3,
	AG_CTRL_BUTTON_BACK = 4,
	AG_CTRL_BUTTON_GUIDE = 5,
	AG_CTRL_BUTTON_START = 6,
	AG_CTRL_BUTTON_LEFT_STICK = 7,
	AG_CTRL_BUTTON_RIGHT_STICK = 8,
	AG_CTRL_BUTTON_LEFT_SHOULDER = 9,
	AG_CTRL_BUTTON_RIGHT_SHOULDER = 10,
	AG_CTRL_BUTTON_DPAD_UP = 11,
	AG_CTRL_BUTTON_DPAD_DOWN = 12,
	AG_CTRL_BUTTON_DPAD_LEFT = 13,
	AG_CTRL_BUTTON_DPAD_RIGHT = 14,
	AG_CTRL_BUTTON_MISC1 = 15,      /* Share button (Xbox Series X), */
	                                /* Microphone button (PS5), */
	                                /* or Capture (Nintendo Switch Pro), */
	                                /* or Microphone button (Amazon Luna). */
	AG_CTRL_BUTTON_PADDLE1 = 16,    /* Xbox Elite paddle P1 */
	AG_CTRL_BUTTON_PADDLE2 = 17,    /* Xbox Elite paddle P3 */
	AG_CTRL_BUTTON_PADDLE3 = 18,    /* Xbox Elite paddle P2 */
	AG_CTRL_BUTTON_PADDLE4 = 19,    /* Xbox Elite paddle P4 */
	AG_CTRL_BUTTON_TOUCHPAD = 20,   /* PS4/PS5 touchpad button */
	AG_CTRL_BUTTON_MAX = 21,
	AG_CTRL_BUTTON_LAST
} AG_ControllerButton;

typedef struct ag_controller {
	struct ag_joystick _inherit;     /* AG_InputDevice -> AG_Joystick ->
	                                    AG_Controller */
	Uint flags;
	AG_ControllerType type;          /* Specific controller type */
} AG_Controller;

#define AGCONTROLLER(obj) ((AG_Controller *)(obj))

__BEGIN_DECLS
extern AG_ObjectClass agControllerClass;

AG_Controller *_Nullable AG_ControllerNew(void *_Nonnull, const char *_Nonnull);

void AG_ProcessController(struct ag_window *_Nullable,
                          const struct ag_driver_event *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_JOYSTICK_H_ */

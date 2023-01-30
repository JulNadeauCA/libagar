/*	Public domain	*/

#ifndef _AGAR_GUI_JOYSTICK_H_
#define _AGAR_GUI_JOYSTICK_H_
#include <agar/gui/input_device.h>
#include <agar/gui/begin.h>

struct ag_window;
struct ag_driver_event;

/*
 * Joystick Event.
 */
#define AG_JOY_PRESSED  1
#define AG_JOY_RELEASED 0

typedef enum ag_joy_hat_position {
	AG_JOY_HAT_CENTERED   = 0x00,
	AG_JOY_HAT_UP         = 0x01,
	AG_JOY_HAT_RIGHT      = 0x02,
	AG_JOY_HAT_DOWN       = 0x04,
	AG_JOY_HAT_LEFT       = 0x08,
	AG_JOY_HAT_RIGHT_UP   = (AG_JOY_HAT_RIGHT | AG_JOY_HAT_UP),
	AG_JOY_HAT_RIGHT_DOWN = (AG_JOY_HAT_RIGHT | AG_JOY_HAT_DOWN),
	AG_JOY_HAT_LEFT_UP    = (AG_JOY_HAT_LEFT | AG_JOY_HAT_UP),
	AG_JOY_HAT_LEFT_DOWN  = (AG_JOY_HAT_LEFT | AG_JOY_HAT_DOWN),
} AG_JoyHatPosition;

typedef enum ag_joystick_type {
	AG_JOYSTICK_TYPE_UNKNOWN,
	AG_JOYSTICK_TYPE_GAMECONTROLLER,
	AG_JOYSTICK_TYPE_WHEEL,
	AG_JOYSTICK_TYPE_ARCADE_STICK,
	AG_JOYSTICK_TYPE_FLIGHT_STICK,
	AG_JOYSTICK_TYPE_DANCE_PAD,
	AG_JOYSTICK_TYPE_GUITAR,
	AG_JOYSTICK_TYPE_DRUM_KIT,
	AG_JOYSTICK_TYPE_ARCADE_PAD,
	AG_JOYSTICK_TYPE_THROTTLE,
	AG_JOYSTICK_TYPE_LAST
} AG_JoystickType;

typedef struct ag_joystick {
	struct ag_input_device _inherit;     /* AG_InputDevice -> AG_Joystick */
	Uint flags;
#define AG_JOYSTICK_HAS_LED             0x01 /* Joystick has LED */
#define AG_JOYSTICK_HAS_RUMBLE          0x02 /* Joystick has rumble */
#define AG_JOYSTICK_HAS_RUMBLE_TRIGGERS 0x04 /* Joystick has rumble */
	AG_JoystickType type;                /* Joystick type */
	char *name;                          /* Joystick name */

	int deviceIdx;                       /* Device index (SDL2 device_index) */
	int instanceID;                      /* Monotonically-increasing ID as
	                                        device is plugged and unplugged */
	int playerIdx;                       /* SDL2 player_index, XInput user index */
	Uint nButtons;                       /* Number of hats */
	Uint nAxes;                          /* Number of axes */
	Uint nBalls;                         /* Number of balls */
	Uint nHats;                          /* Number of hats */
	Uint16 vendorID;                     /* Vendor ID */
	Uint16 productID;                    /* Product ID */
	char guid[64];                       /* Joystick GUID */
	void *instancePtr;                   /* Opaque instance pointer */
} AG_Joystick;

#define AGJOYSTICK(obj) ((AG_Joystick *)(obj))

__BEGIN_DECLS
extern AG_ObjectClass agJoystickClass;

AG_Joystick *_Nullable AG_JoystickNew(void *_Nonnull, const char *_Nonnull);
void                   AG_JoystickDestroy(void *_Nonnull, AG_Joystick *_Nonnull);

void AG_ProcessJoystick(struct ag_window *_Nullable,
                        const struct ag_driver_event *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_JOYSTICK_H_ */

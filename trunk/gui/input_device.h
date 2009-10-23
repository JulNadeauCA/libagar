/*	Public domain	*/

#ifndef _AGAR_GUI_INPUT_DEVICE_H_
#define _AGAR_GUI_INPUT_DEVICE_H_
#include <agar/gui/begin.h>

typedef struct ag_input_device {
	struct ag_object _inherit;
	Uint flags;
	void *drv;		/* Associated graphics driver */
	char *desc;		/* User description */
	AG_EventQ events;	/* Queue of input events */
} AG_InputDevice;

#define AGINPUTDEV(obj) ((AG_InputDevice *)(obj))

__BEGIN_DECLS
extern AG_ObjectClass agInputDeviceClass;
extern AG_Object      agInputDevices;	/* Input devices VFS */

AG_InputDevice *AG_InputDeviceNew(void *);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_INPUT_DEVICE_H_ */

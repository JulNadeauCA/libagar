/*	Public domain	*/

#ifndef _AGAR_GUI_INPUT_DEVICE_H_
#define _AGAR_GUI_INPUT_DEVICE_H_
#include <agar/gui/begin.h>

struct ag_widget;

typedef struct ag_input_device {
	struct ag_object _inherit;
	void *_Nullable drv;                 /* Associated graphics driver */
	char *_Nullable desc;                /* User description */
	Uint flags;
	Uint                                 nWidGrab; /* Widget grab count */
	struct ag_widget *_Nullable *_Nonnull widGrab; /* Widget grabs */
} AG_InputDevice;

/*
 * Touch Event (Gesture, Multigesture or Dollar Gesture).
 */
typedef enum ag_touch_event_type {
	AG_TOUCH_FINGER_MOTION,
	AG_TOUCH_FINGER_DOWN,
	AG_TOUCH_FINGER_UP,
	AG_TOUCH_MULTIGESTURE,
	AG_TOUCH_DOLLAR_GESTURE,
	AG_TOUCH_DOLLAR_RECORD,
	AG_TOUCH_EVENT_LAST
} AG_TouchEventType;

typedef struct ag_touch_event {
	AG_TouchEventType type;              /* Touch event type */
	Uint32 timestamp;                    /* Unix time */
	Sint64 touchID;                      /* Touch device ID */
	union {
		struct {
			Sint64 fingerID;     /* Finger ID */
			float x, y;          /* Normalized (0 to 1) */
			float dx, dy;        /* Normalized (-1 to +1)*/
			float pressure;      /* Normalized (0 to 1) */
			Uint32 _pad;
		} finger;
		struct {
			float dTheta, dDist; /* Angle and distance */
			float x, y;          /* Coordinates */
			Uint numFingers;     /* Finger count */
		} multigesture;
		struct {
			Sint64 gestureID;    /* Gesture ID */
			Uint numFingers;     /* Finger count */
			float error;         /* Accuracy error */
			float x, y;          /* Normalized center */
		} dollar;
	};
} AG_TouchEvent;

#define   AGINPUTDEVICE(obj)    ((AG_InputDevice *)(obj))
#define  AGcINPUTDEVICE(obj)    ((const AG_InputDevice *)(obj))
#define  AG_INPUTDEVICE_ISA(o) (((AGOBJECT(o)->cid & 0xff000000) >> 24) == 0x06)
#define  AG_INPUTDEVICE_SELF()    AGINPUTDEVICE(  AG_OBJECT(0,         "AG_InputDevice:*") )
#define  AG_INPUTDEVICE_PTR(n)    AGINPUTDEVICE(  AG_OBJECT((n),       "AG_InputDevice:*") )
#define  AG_INPUTDEVICE_NAMED(n)  AGINPUTDEVICE(  AG_OBJECT_NAMED((n), "AG_InputDevice:*") )
#define AG_cINPUTDEVICE_SELF()   AGcINPUTDEVICE( AG_cOBJECT(0,         "AG_InputDevice:*") )
#define AG_cINPUTDEVICE_PTR(n)   AGcINPUTDEVICE( AG_cOBJECT((n),       "AG_InputDevice:*") )
#define AG_cINPUTDEVICE_NAMED(n) AGcINPUTDEVICE( AG_cOBJECT_NAMED((n), "AG_InputDevice:*") )

__BEGIN_DECLS
extern AG_ObjectClass agInputDeviceClass;
extern AG_Object      agInputDevices;	/* Input devices VFS */

void AG_GrabInputDevice(void *_Nonnull, void *_Nonnull);
void AG_UngrabInputDevice(void *_Nonnull, void *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_INPUT_DEVICE_H_ */

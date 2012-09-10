/*	Public domain	*/

#ifndef _AGAR_WIDGET_SLIDER_H_
#define _AGAR_WIDGET_SLIDER_H_

#include <agar/gui/widget.h>

#include <agar/gui/begin.h>

enum ag_slider_type {
	AG_SLIDER_HORIZ,
	AG_SLIDER_VERT
};

enum ag_slider_button {
	AG_SLIDER_BUTTON_NONE,
	AG_SLIDER_BUTTON_DEC,
	AG_SLIDER_BUTTON_INC,
	AG_SLIDER_BUTTON_SCROLL
};

typedef struct ag_slider {
	struct ag_widget wid;
	Uint flags;
#define AG_SLIDER_HFILL		0x01
#define AG_SLIDER_VFILL		0x02
#define AG_SLIDER_FOCUSABLE	0x04
#define AG_SLIDER_EXCL		0x08	/* Exclusive access to binding */
#define AG_SLIDER_EXPAND	(AG_SLIDER_HFILL|AG_SLIDER_VFILL)

	int value;			/* Default value binding */
	int min, max;			/* Default range bindings */
	enum ag_slider_type type;	/* Style of scrollbar */
	int ctlPressed;			/* Control is pressed */
	int wControlPref;		/* Control size (preferred) */
	int wControl;			/* Control size (effective) */
	AG_Timer moveTo;		/* Timer for keyboard motion */
	int xOffs;			/* Cursor offset for scrolling */
	int extent;			/* Available area for scrolling */
} AG_Slider;

#define AGSLIDER(p) ((AG_Slider *)p)

__BEGIN_DECLS
extern AG_WidgetClass agSliderClass;

AG_Slider *AG_SliderNew(void *, enum ag_slider_type, Uint);
AG_Slider *AG_SliderNewInt(void *, enum ag_slider_type, Uint, int *, int *, int *);
AG_Slider *AG_SliderNewIntR(void *, enum ag_slider_type, Uint, int *, int, int);
AG_Slider *AG_SliderNewUint(void *, enum ag_slider_type, Uint, Uint *, Uint *, Uint *);
AG_Slider *AG_SliderNewUintR(void *, enum ag_slider_type, Uint, Uint *, Uint, Uint);
AG_Slider *AG_SliderNewFlt(void *, enum ag_slider_type, Uint, float *, float *, float *);
AG_Slider *AG_SliderNewFltR(void *, enum ag_slider_type, Uint, float *, float, float);
AG_Slider *AG_SliderNewDbl(void *, enum ag_slider_type, Uint, double *, double *, double *);
AG_Slider *AG_SliderNewDblR(void *, enum ag_slider_type, Uint, double *, double, double);
void       AG_SliderSetControlSize(AG_Slider *, int);

#ifdef AG_LEGACY
void       AG_SliderSetIntIncrement(AG_Slider *, int) DEPRECATED_ATTRIBUTE;
void       AG_SliderSetRealIncrement(AG_Slider *, double) DEPRECATED_ATTRIBUTE;
AG_Slider *AG_SliderNewUint8(void *, enum ag_slider_type, Uint, Uint8 *, Uint8 *, Uint8 *) DEPRECATED_ATTRIBUTE;
AG_Slider *AG_SliderNewUint8R(void *, enum ag_slider_type, Uint, Uint8 *, Uint8, Uint8) DEPRECATED_ATTRIBUTE;
AG_Slider *AG_SliderNewSint8(void *, enum ag_slider_type, Uint, Sint8 *, Sint8 *, Sint8 *) DEPRECATED_ATTRIBUTE;
AG_Slider *AG_SliderNewSint8R(void *, enum ag_slider_type, Uint, Sint8 *, Sint8, Sint8) DEPRECATED_ATTRIBUTE;
AG_Slider *AG_SliderNewUint16(void *, enum ag_slider_type, Uint, Uint16 *, Uint16 *, Uint16 *) DEPRECATED_ATTRIBUTE;
AG_Slider *AG_SliderNewUint16R(void *, enum ag_slider_type, Uint, Uint16 *, Uint16, Uint16) DEPRECATED_ATTRIBUTE;
AG_Slider *AG_SliderNewSint16(void *, enum ag_slider_type, Uint, Sint16 *, Sint16 *, Sint16 *) DEPRECATED_ATTRIBUTE;
AG_Slider *AG_SliderNewSint16R(void *, enum ag_slider_type, Uint, Sint16 *, Sint16, Sint16) DEPRECATED_ATTRIBUTE;
AG_Slider *AG_SliderNewUint32(void *, enum ag_slider_type, Uint, Uint32 *, Uint32 *, Uint32 *) DEPRECATED_ATTRIBUTE;
AG_Slider *AG_SliderNewUint32R(void *, enum ag_slider_type, Uint, Uint32 *, Uint32, Uint32) DEPRECATED_ATTRIBUTE;
AG_Slider *AG_SliderNewSint32(void *, enum ag_slider_type, Uint, Sint32 *, Sint32 *, Sint32 *) DEPRECATED_ATTRIBUTE;
AG_Slider *AG_SliderNewSint32R(void *, enum ag_slider_type, Uint, Sint32 *, Sint32, Sint32) DEPRECATED_ATTRIBUTE;
#endif /* AG_LEGACY */

__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_WIDGET_SLIDER_H_ */

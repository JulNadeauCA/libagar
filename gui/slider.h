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
	struct ag_widget wid;		/* AG_Widget -> AG_Slider */
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
	int xOffs;			/* Cursor offset for scrolling */
	int extent;			/* Available area for scrolling */
	AG_Timer moveTo;		/* Timer for keyboard motion */
} AG_Slider;

#define AGSLIDER(obj)            ((AG_Slider *)(obj))
#define AGCSLIDER(obj)           ((const AG_Slider *)(obj))
#define AG_SLIDER_SELF()          AGSLIDER( AG_OBJECT(0,"AG_Widget:AG_Slider:*") )
#define AG_SLIDER_PTR(n)          AGSLIDER( AG_OBJECT((n),"AG_Widget:AG_Slider:*") )
#define AG_SLIDER_NAMED(n)        AGSLIDER( AG_OBJECT_NAMED((n),"AG_Widget:AG_Slider:*") )
#define AG_CONST_SLIDER_SELF()   AGCSLIDER( AG_CONST_OBJECT(0,"AG_Widget:AG_Slider:*") )
#define AG_CONST_SLIDER_PTR(n)   AGCSLIDER( AG_CONST_OBJECT((n),"AG_Widget:AG_Slider:*") )
#define AG_CONST_SLIDER_NAMED(n) AGCSLIDER( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Slider:*") )

__BEGIN_DECLS
extern AG_WidgetClass agSliderClass;

AG_Slider *_Nonnull AG_SliderNew(void *_Nullable, enum ag_slider_type, Uint);
AG_Slider *_Nonnull AG_SliderNewInt(void *_Nullable, enum ag_slider_type, Uint,
                                    int *_Nullable, int *_Nullable, int *_Nullable);
AG_Slider *_Nonnull AG_SliderNewIntR(void *_Nullable, enum ag_slider_type, Uint,
                                     int *_Nullable, int, int);
AG_Slider *_Nonnull AG_SliderNewUint(void *_Nullable, enum ag_slider_type, Uint,
                                     Uint *_Nullable, Uint *_Nullable, Uint *_Nullable);
AG_Slider *_Nonnull AG_SliderNewUintR(void *_Nullable, enum ag_slider_type, Uint,
                                      Uint *_Nullable, Uint, Uint);
AG_Slider *_Nonnull AG_SliderNewFlt(void *_Nullable, enum ag_slider_type, Uint,
                                    float *_Nullable, float *_Nullable,
				    float *_Nullable);
AG_Slider *_Nonnull AG_SliderNewFltR(void *_Nullable, enum ag_slider_type, Uint,
                                     float *_Nullable, float, float);
AG_Slider *_Nonnull AG_SliderNewDbl(void *_Nullable, enum ag_slider_type, Uint,
                                    double *_Nullable, double *_Nullable,
				    double *_Nullable);
AG_Slider *_Nonnull AG_SliderNewDblR(void *_Nullable, enum ag_slider_type, Uint,
                                     double *_Nullable, double, double);

void AG_SliderSetControlSize(AG_Slider *_Nonnull, int);
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_WIDGET_SLIDER_H_ */

/*	Public domain	*/

#ifndef _AGAR_GUI_DUMMY_H_
#define _AGAR_GUI_DUMMY_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

typedef struct ag_dummy {
	struct ag_widget wid;
	Uint flags;
#define AG_DUMMY_HFILL	0x01	/* Fill available width */
#define AG_DUMMY_VFILL	0x02	/* Fill available height */
#define AG_DUMMY_EXPAND	(AG_DUMMY_HFILL|AG_DUMMY_VFILL)
	int mySurface;		/* Surface handle */
} AG_Dummy;

__BEGIN_DECLS
AG_Dummy  *AG_DummyNew(void *, Uint, const char *);
void	   AG_DummyInit(AG_Dummy *, Uint, const char *);
void	   AG_DummyDestroy(void *);
void	   AG_DummyDraw(void *);
void	   AG_DummyScale(void *, int, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_GUI_DUMMY_H_ */

/*	Public domain	*/

#ifndef _AGAR_WIDGET_LABEL_H_
#define _AGAR_WIDGET_LABEL_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

#define AG_LABEL_MAX		1024
#define AG_LABEL_MAX_POLLPTRS	32

enum ag_label_type {
	AG_LABEL_STATIC,		/* Static text */
	AG_LABEL_POLLED,		/* Polling (thread unsafe) */
	AG_LABEL_POLLED_MT		/* Polling (thread safe) */
};

typedef struct ag_label {
	struct ag_widget wid;
	enum ag_label_type type;
	Uint flags;
#define AG_LABEL_HFILL		0x01		/* Fill horizontal space */
#define AG_LABEL_VFILL		0x02		/* Fill vertical space */
#define AG_LABEL_EXPAND		(AG_LABEL_HFILL|AG_LABEL_VFILL)
	AG_Mutex lock;
	int surface;
	int wPre, hPre;				/* Prescale dimensions */
	int lPad, rPad, tPad, bPad;		/* Label padding */
	struct {
		char fmt[AG_LABEL_MAX];
		void *ptrs[AG_LABEL_MAX_POLLPTRS];
		int nptrs;
		AG_Mutex *lock;
	} poll;
} AG_Label;

__BEGIN_DECLS
AG_Label *AG_LabelNewPolled(void *, Uint, const char *, ...);
AG_Label *AG_LabelNewPolledMT(void *, Uint, AG_Mutex *, const char *, ...)
	 NONNULL_ATTRIBUTE(3);
AG_Label *AG_LabelNewStatic(void *, Uint, const char *, ...)
	 FORMAT_ATTRIBUTE(printf, 3, 4);
AG_Label *AG_LabelNewStaticString(void *, Uint, const char *);

void	AG_LabelInit(AG_Label *, enum ag_label_type, Uint, const char *);
void	AG_LabelDestroy(void *);
void	AG_LabelDraw(void *);
void	AG_LabelScale(void *, int, int);
void	AG_LabelText(AG_Label *, const char *, ...)
	FORMAT_ATTRIBUTE(printf, 2, 3)
	NONNULL_ATTRIBUTE(2);
#define AG_LabelPrintf AG_LabelText

void	 AG_LabelSetSurface(AG_Label *, SDL_Surface *);
void	 AG_LabelSetPadding(AG_Label *, int, int, int, int);
#define	 AG_LabelSetPaddingLeft(lbl,v)   AG_LabelSetPadding(lbl,(v),-1,-1,-1)
#define	 AG_LabelSetPaddingRight(lbl,v)  AG_LabelSetPadding(lbl,-1,(v),-1,-1)
#define	 AG_LabelSetPaddingTop(lbl,v)    AG_LabelSetPadding(lbl,-1,-1,(v),-1)
#define	 AG_LabelSetPaddingBottom(lbl,v) AG_LabelSetPadding(lbl,-1,-1,-1,(v))
void	 AG_LabelPrescale(AG_Label *, Uint, const char *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_LABEL_H_ */

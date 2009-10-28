/*	Public domain	*/

#ifndef _AGAR_WIDGET_LABEL_H_
#define _AGAR_WIDGET_LABEL_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/text.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/text.h>
#endif

#include "begin_code.h"

#define AG_LABEL_MAX		1024
#define AG_LABEL_MAX_POLLPTRS	32

enum ag_label_type {
	AG_LABEL_STATIC,		/* Static text */
	AG_LABEL_POLLED,		/* Polling (thread unsafe) */
	AG_LABEL_POLLED_MT		/* Polling (thread safe) */
};

struct ag_label_flag {
	Uint idx;			/* Flag arg in format string */
	const char *text;		/* Label text */
	Uint32 v;			/* Bitmask */
	enum ag_widget_binding_type type;
	AG_SLIST_ENTRY(ag_label_flag) lflags;
};

typedef struct ag_label {
	struct ag_widget wid;
	enum ag_label_type type;
	Uint flags;
#define AG_LABEL_HFILL		0x01	/* Fill horizontal space */
#define AG_LABEL_VFILL		0x02	/* Fill vertical space */
#define AG_LABEL_NOMINSIZE	0x04	/* No minimum enforced size */
#define AG_LABEL_PARTIAL	0x10	/* Partial mode (RO) */
#define AG_LABEL_REGEN		0x20	/* Regenerate surface at next draw */
#define AG_LABEL_EXPAND		(AG_LABEL_HFILL|AG_LABEL_VFILL)
	char *text;			/* Text buffer */
	int surface;			/* Label surface */
	int surfaceCont;		/* [...] surface */
	int wPre, hPre;			/* SizeHint dimensions */
	int lPad, rPad, tPad, bPad;	/* Label padding */
	enum ag_text_justify justify;	/* Justification mode */
	struct {
		AG_Mutex *lock;			   /* Lock for polled data */
		void *ptrs[AG_LABEL_MAX_POLLPTRS]; /* Pointers to polled data */
		int nptrs;
	} poll;
	AG_SLIST_HEAD(,ag_label_flag) lflags;	/* Label flag descriptions */
} AG_Label;

__BEGIN_DECLS
extern AG_WidgetClass agLabelClass;

AG_Label *AG_LabelNewPolled(void *, Uint, const char *, ...);
AG_Label *AG_LabelNewPolledMT(void *, Uint, AG_Mutex *, const char *, ...)
	     NONNULL_ATTRIBUTE(3);
AG_Label *AG_LabelNewStatic(void *, Uint, const char *, ...)
	     FORMAT_ATTRIBUTE(printf, 3, 4);
AG_Label *AG_LabelNewStaticString(void *, Uint, const char *);
#define   AG_LabelNew AG_LabelNewStatic
#define   AG_LabelNewString AG_LabelNewStaticString

void	AG_LabelString(AG_Label *, const char *);
void	AG_LabelText(AG_Label *, const char *, ...)
	    FORMAT_ATTRIBUTE(printf, 2, 3)
	    NONNULL_ATTRIBUTE(2);

void	 AG_LabelSetPadding(AG_Label *, int, int, int, int);
void	 AG_LabelJustify(AG_Label *, enum ag_text_justify);
#define	 AG_LabelSetPaddingLeft(lbl,v)   AG_LabelSetPadding((lbl),(v),-1,-1,-1)
#define	 AG_LabelSetPaddingRight(lbl,v)  AG_LabelSetPadding((lbl),-1,(v),-1,-1)
#define	 AG_LabelSetPaddingTop(lbl,v)    AG_LabelSetPadding((lbl),-1,-1,(v),-1)
#define	 AG_LabelSetPaddingBottom(lbl,v) AG_LabelSetPadding((lbl),-1,-1,-1,(v))
void	 AG_LabelSizeHint(AG_Label *, Uint, const char *);
#define	 AG_LabelPrescale AG_LabelSizeHint

void	 AG_LabelFlagNew(AG_Label *, Uint, const char *,
	                 enum ag_widget_binding_type, Uint32);
#define	 AG_LabelFlag(lbl,i,s,v) \
	 AG_LabelFlagNew((lbl),(i),(s),AG_WIDGET_FLAG,(Uint)(v))
#define	 AG_LabelFlag8(lbl,i,s,v) \
	 AG_LabelFlagNew((lbl),(i),(s),AG_WIDGET_FLAG8,(Uint8)(v))
#define	 AG_LabelFlag16(lbl,i,s,v) \
	 AG_LabelFlagNew((lbl),(i),(s),AG_WIDGET_FLAG16,(Uint16)(v))
#define	 AG_LabelFlag32(lbl,i,s,v) \
	 AG_LabelFlagNew((lbl),(i),(s),AG_WIDGET_FLAG32,(Uint32)(v))

/* Legacy */
#define AG_LabelPrintf AG_LabelText
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_LABEL_H_ */
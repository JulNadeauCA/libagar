/*	Public domain	*/

#ifndef _AGAR_WIDGET_LABEL_H_
#define _AGAR_WIDGET_LABEL_H_

#include <agar/gui/widget.h>
#include <agar/gui/text.h>

#include <agar/gui/begin.h>

#define AG_LABEL_MAX		1024	/* Max format string length */
#define AG_LABEL_MAX_POLLPTRS	32	/* Max polled pointers */

struct ag_label;
struct ag_text_cache;

/* Type of label */
enum ag_label_type {
	AG_LABEL_STATIC,		/* Display static text */
	AG_LABEL_POLLED			/* Display an AG_FmtString(3) */
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
#define AG_LABEL_FRAME		0x80	/* Draw visible frame */
#define AG_LABEL_EXPAND		(AG_LABEL_HFILL|AG_LABEL_VFILL)
	char *text;			/* Text buffer (for static labels) */
	int surface;			/* Label surface */
	int surfaceCont;		/* [...] surface */
	int wPre, hPre;			/* SizeHint dimensions */
	int lPad, rPad, tPad, bPad;	/* Label padding */
	enum ag_text_justify justify;	/* Justification mode */
	enum ag_text_valign valign;	/* Vertical alignment */
	struct ag_text_cache *tCache;	/* Cache for polled labels */
	AG_Rect rClip;			/* Clipping rectangle */
	AG_FmtString *fmt;		/* Polled label data */
	char  *pollBuf;			/* Buffer for polled labels */
	size_t pollBufSize;
} AG_Label;

__BEGIN_DECLS
extern AG_WidgetClass agLabelClass;

AG_Label *AG_LabelNew(void *, Uint, const char *, ...)
                      FORMAT_ATTRIBUTE(printf, 3, 4);
AG_Label *AG_LabelNewS(void *, Uint, const char *);
AG_Label *AG_LabelNewPolled(void *, Uint, const char *, ...);
AG_Label *AG_LabelNewPolledMT(void *, Uint, AG_Mutex *, const char *, ...);

void      AG_LabelText(AG_Label *, const char *, ...)
                       FORMAT_ATTRIBUTE(printf, 2, 3)
                       NONNULL_ATTRIBUTE(2);
void      AG_LabelTextS(AG_Label *, const char *);

void	 AG_LabelSetPadding(AG_Label *, int, int, int, int);
void	 AG_LabelJustify(AG_Label *, enum ag_text_justify);
void	 AG_LabelValign(AG_Label *, enum ag_text_valign);
#define	 AG_LabelSetPaddingLeft(lbl,v)   AG_LabelSetPadding((lbl),(v),-1,-1,-1)
#define	 AG_LabelSetPaddingRight(lbl,v)  AG_LabelSetPadding((lbl),-1,(v),-1,-1)
#define	 AG_LabelSetPaddingTop(lbl,v)    AG_LabelSetPadding((lbl),-1,-1,(v),-1)
#define	 AG_LabelSetPaddingBottom(lbl,v) AG_LabelSetPadding((lbl),-1,-1,-1,(v))
void	 AG_LabelSizeHint(AG_Label *, Uint, const char *);

#ifdef AG_LEGACY
# define AG_LabelNewStatic	AG_LabelNew
# define AG_LabelNewString	AG_LabelNewS
# define AG_LabelNewStaticS	AG_LabelNewS
# define AG_LabelPrintf		AG_LabelText
# define AG_LabelString(lbl,s)	AG_LabelTextS((lbl),(s))
# define AG_LabelPrescale	AG_LabelSizeHint
# define AG_LABEL_POLLED_MT	AG_LABEL_POLLED
# define AG_LabelSetFont        AG_SetFont
#endif /* AG_LEGACY */
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_LABEL_H_ */

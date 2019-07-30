/*	Public domain	*/

#ifndef _AGAR_WIDGET_LABEL_H_
#define _AGAR_WIDGET_LABEL_H_

#include <agar/gui/widget.h>
#include <agar/gui/text.h>

#include <agar/gui/begin.h>

#if AG_MODEL == AG_SMALL
# define AG_LABEL_MAX		256	/* Max format string length */
# define AG_LABEL_MAX_POLLPTRS	8	/* Max polled pointers */
# define AG_SMALL_LABEL_MAX	32	/* Max length of small labels */
#else
# define AG_LABEL_MAX		1024	/* Max format string length */
# define AG_LABEL_MAX_POLLPTRS	32	/* Max polled pointers */
# define AG_SMALL_LABEL_MAX	256	/* Max length of small labels */
#endif

struct ag_label;
struct ag_text_cache;

/* Type of label */
enum ag_label_type {
	AG_LABEL_STATIC,		/* Display static text */
	AG_LABEL_POLLED			/* Display an AG_FmtString(3) */
};

typedef struct ag_label {
	struct ag_widget wid;			/* AG_Widget -> AG_Label */
	enum ag_label_type type;
	Uint flags;
#define AG_LABEL_HFILL		0x01		/* Fill horizontal space */
#define AG_LABEL_VFILL		0x02		/* Fill vertical space */
#define AG_LABEL_NOMINSIZE	0x04		/* No minimum enforced size */
#define AG_LABEL_PARTIAL	0x10		/* Partial mode (RO) */
#define AG_LABEL_REGEN		0x20		/* Regenerate surface at next draw */
#define AG_LABEL_FRAME		0x80		/* Draw visible frame */
#define AG_LABEL_EXPAND		(AG_LABEL_HFILL|AG_LABEL_VFILL)
	char *_Nullable text;			/* Text buffer (for static labels) */
	int surface;				/* Label surface */
	int surfaceCont;			/* [...] surface */
	int wPre, hPre;				/* SizeHint dimensions */
	int lPad, rPad, tPad, bPad;		/* Label padding */
	enum ag_text_justify justify;		/* Justification mode */
	enum ag_text_valign valign;		/* Vertical alignment */
	struct ag_text_cache *_Nonnull tCache;	/* Cache for polled labels */
	AG_Rect rClip;				/* Clipping rectangle */
#ifdef AG_ENABLE_STRING
	AG_FmtString *_Nullable fmt;		/* Polled label data */
	char *_Nullable pollBuf;		/* Buffer for polled labels */
	AG_Size         pollBufSize;
# if AG_MODEL == AG_MEDIUM
	Uint32 _pad;
# endif
#endif
} AG_Label;

#define AGLABEL(obj)            ((AG_Label *)(obj))
#define AGCLABEL(obj)           ((const AG_Label *)(obj))
#define AG_LABEL_SELF()          AGLABEL( AG_OBJECT(0,"AG_Widget:AG_Label:*") )
#define AG_LABEL_PTR(n)          AGLABEL( AG_OBJECT((n),"AG_Widget:AG_Label:*") )
#define AG_LABEL_NAMED(n)        AGLABEL( AG_OBJECT_NAMED((n),"AG_Widget:AG_Label:*") )
#define AG_CONST_LABEL_SELF()   AGCLABEL( AG_CONST_OBJECT(0,"AG_Widget:AG_Label:*") )
#define AG_CONST_LABEL_PTR(n)   AGCLABEL( AG_CONST_OBJECT((n),"AG_Widget:AG_Label:*") )
#define AG_CONST_LABEL_NAMED(n) AGCLABEL( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Label:*") )

__BEGIN_DECLS
extern AG_WidgetClass agLabelClass;

AG_Label *_Nonnull AG_LabelNewS(void *_Nullable, Uint, const char *_Nullable);
AG_Label *_Nonnull AG_LabelNew(void *_Nullable, Uint, const char *_Nullable, ...)
                               FORMAT_ATTRIBUTE(printf,3,4);
#ifdef AG_ENABLE_STRING
AG_Label *_Nonnull AG_LabelNewPolled(void *_Nullable, Uint,
                                     const char *_Nonnull, ...);
AG_Label *_Nonnull AG_LabelNewPolledMT(void *_Nullable, Uint,
                                       _Nonnull_Mutex AG_Mutex *_Nonnull,
				       const char *_Nonnull, ...);
#endif

void AG_LabelTextS(AG_Label *_Nonnull, const char *_Nonnull);
void AG_LabelText(AG_Label *_Nonnull, const char *_Nonnull, ...)
                 FORMAT_ATTRIBUTE(printf,2,3);

void	AG_LabelSetPadding(AG_Label *_Nonnull, int,int,int,int);
void	AG_LabelJustify(AG_Label *_Nonnull, enum ag_text_justify);
void	AG_LabelValign(AG_Label *_Nonnull, enum ag_text_valign);
#define	AG_LabelSetPaddingLeft(lbl,v)   AG_LabelSetPadding((lbl),(v),-1,-1,-1)
#define	AG_LabelSetPaddingRight(lbl,v)  AG_LabelSetPadding((lbl),-1,(v),-1,-1)
#define	AG_LabelSetPaddingTop(lbl,v)    AG_LabelSetPadding((lbl),-1,-1,(v),-1)
#define	AG_LabelSetPaddingBottom(lbl,v) AG_LabelSetPadding((lbl),-1,-1,-1,(v))
void	AG_LabelSizeHint(AG_Label *_Nonnull, Uint, const char *_Nullable);

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

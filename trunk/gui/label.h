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
	AG_LABEL_STATIC,		/* Static text */
	AG_LABEL_POLLED,		/* Polling (thread unsafe) */
	AG_LABEL_POLLED_MT		/* Polling (thread safe) */
};

/* Bit flag description. */
struct ag_label_flag {
	Uint idx;			/* Flag arg in format string */
	const char *text;		/* Label text */
	Uint32 v;			/* Bitmask */
	AG_VariableType type;
	AG_SLIST_ENTRY(ag_label_flag) lflags;
};

/* Extended format specifier for polled labels. */
typedef void (*AG_LabelFormatFn)(struct ag_label *, char *, size_t, int);
typedef struct ag_label_format_spec {
	char *fmt;
	size_t fmtLen;
	AG_LabelFormatFn fn;
} AG_LabelFormatSpec;

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
	char *text;			/* Text buffer */
	int surface;			/* Label surface */
	int surfaceCont;		/* [...] surface */
	int wPre, hPre;			/* SizeHint dimensions */
	int lPad, rPad, tPad, bPad;	/* Label padding */
	enum ag_text_justify justify;	/* Justification mode */
	enum ag_text_valign valign;	/* Vertical alignment */
	struct {
		AG_Mutex *lock;			   /* Lock for polled data */
		void *ptrs[AG_LABEL_MAX_POLLPTRS]; /* Pointers to polled data */
		int nptrs;
	} poll;
	AG_SLIST_HEAD_(ag_label_flag) lflags;	/* Label flag descriptions */
	struct ag_text_cache *tCache;		/* Cache for polled labels */
	AG_Rect rClip;				/* Clipping rectangle */
} AG_Label;

__BEGIN_DECLS
extern AG_WidgetClass agLabelClass;

AG_Label *AG_LabelNew(void *, Uint, const char *, ...)
                      FORMAT_ATTRIBUTE(printf, 3, 4);
AG_Label *AG_LabelNewS(void *, Uint, const char *);
AG_Label *AG_LabelNewPolled(void *, Uint, const char *, ...);
AG_Label *AG_LabelNewPolledMT(void *, Uint, AG_Mutex *, const char *, ...)
                              NONNULL_ATTRIBUTE(3);

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

void	 AG_LabelFlagNew(AG_Label *, Uint, const char *, AG_VariableType, Uint32);
#define	 AG_LabelFlag(lbl,i,s,v) \
	 AG_LabelFlagNew((lbl),(i),(s),AG_VARIABLE_P_FLAG,(Uint)(v))
#define	 AG_LabelFlag8(lbl,i,s,v) \
	 AG_LabelFlagNew((lbl),(i),(s),AG_VARIABLE_P_FLAG8,(Uint8)(v))
#define	 AG_LabelFlag16(lbl,i,s,v) \
	 AG_LabelFlagNew((lbl),(i),(s),AG_VARIABLE_P_FLAG16,(Uint16)(v))
#define	 AG_LabelFlag32(lbl,i,s,v) \
	 AG_LabelFlagNew((lbl),(i),(s),AG_VARIABLE_P_FLAG32,(Uint32)(v))

#define AG_LABEL_ARG(lbl,_type) (*(_type *)lbl->poll.ptrs[fPos])
void	 AG_RegisterLabelFormat(const char *, AG_LabelFormatFn);
void	 AG_UnregisterLabelFormat(const char *);
void	 AG_LabelInitFormats(void);
void	 AG_LabelDestroyFormats(void);

#ifdef AG_LEGACY
# define AG_LabelNewStatic	AG_LabelNew
# define AG_LabelNewString	AG_LabelNewS
# define AG_LabelNewStaticS	AG_LabelNewS
# define AG_LabelPrintf		AG_LabelText
# define AG_LabelString(lbl,s)	AG_LabelTextS((lbl),(s))
# define AG_LabelPrescale	AG_LabelSizeHint
#endif /* AG_LEGACY */
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_LABEL_H_ */

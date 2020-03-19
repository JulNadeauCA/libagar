/*	Public domain	*/

#ifndef _AGAR_WIDGET_LABEL_H_
#define _AGAR_WIDGET_LABEL_H_

#include <agar/gui/widget.h>
#include <agar/gui/text.h>

#include <agar/gui/begin.h>

#if AG_MODEL == AG_LARGE
# define AG_LABEL_MAX		1024	/* Max format string length */
# define AG_LABEL_MAX_POLLPTRS	32	/* Max polled pointers */
# define AG_SMALL_LABEL_MAX	256	/* Max length of small labels */
#else
# define AG_LABEL_MAX		512 	/* Max format string length */
# define AG_LABEL_MAX_POLLPTRS	16	/* Max polled pointers */
# define AG_SMALL_LABEL_MAX	128	/* Max length of small labels */
#endif

struct ag_label;
struct ag_text_cache;

enum ag_label_type {
	AG_LABEL_STATIC,        /* Display static text */
	AG_LABEL_POLLED,        /* Display an AG_FmtString(3) */
	AG_LABEL_TYPE_LAST
};

typedef struct ag_label {
	struct ag_widget wid;           /* AG_Widget -> AG_Label */
	enum ag_label_type type;        /* Static or Polled */
	Uint flags;
#define AG_LABEL_HFILL   0x001
#define AG_LABEL_VFILL   0x002
#define AG_LABEL_PARTIAL 0x010          /* Partially hidden (horizontal) */
#define AG_LABEL_REGEN   0x020          /* Surface needs to be regenerated */
#define AG_LABEL_FRAME   0x080          /* Draw 3D-style frame around label */
#define AG_LABEL_SLOW    0x100          /* Poll every 2s (default 500ms) */
#define AG_LABEL_EXPAND (AG_LABEL_HFILL | AG_LABEL_VFILL)

	char *_Nullable text;           /* Text buffer (for STATIC labels) */

	int surface;                    /* Cached label surface */
	int surfaceCtd;                 /* Cached ellipsis ("...") surface */
	int wPre, hPre;                 /* Explicit size requisition */

	enum ag_text_justify justify;   /* Justification mode */
	enum ag_text_valign valign;     /* Vertical alignment */

	struct ag_text_cache *_Nonnull tCache; /* Rendered text cache (POLLED) */
	AG_FmtString *_Nullable fmt;           /* Polled label data */
	char *_Nullable pollBuf;               /* Polled label buffer */
	AG_Size         pollBufSize;
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad;
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

AG_Label *_Nonnull AG_LabelNewPolled(void *_Nullable, Uint,
                                     const char *_Nonnull, ...);

AG_Label *_Nonnull AG_LabelNewPolledMT(void *_Nullable, Uint,
                                       _Nonnull_Mutex AG_Mutex *_Nonnull,
				       const char *_Nonnull, ...);

AG_Label *_Nonnull AG_LabelNew(void *_Nullable, Uint, const char *_Nullable, ...)
                               FORMAT_ATTRIBUTE(printf,3,4);

AG_Label *_Nonnull AG_LabelNewS(void *_Nullable, Uint, const char *_Nullable);

void AG_LabelSizeHint(AG_Label *_Nonnull, Uint, const char *_Nullable);
void AG_LabelJustify(AG_Label *_Nonnull, enum ag_text_justify);
void AG_LabelValign(AG_Label *_Nonnull, enum ag_text_valign);

void AG_LabelText(AG_Label *_Nonnull, const char *_Nonnull, ...)
                 FORMAT_ATTRIBUTE(printf,2,3);

void AG_LabelTextS(AG_Label *_Nonnull, const char *_Nonnull);

#ifdef AG_LEGACY
# define AG_LABEL_NOMINSIZE 0x004 /* Has been the default behavior since 1.5 */
# define AG_LABEL_POLLED_MT AG_LABEL_POLLED
# define AG_LabelNewStatic AG_LabelNew
# define AG_LabelNewString AG_LabelNewS
# define AG_LabelNewStaticS AG_LabelNewS
# define AG_LabelPrintf AG_LabelText
# define AG_LabelString(lbl,s) AG_LabelTextS((lbl),(s))
# define AG_LabelPrescale AG_LabelSizeHint
# define AG_LabelSetFont AG_SetFont
# define AG_LabelSetPadding(lbl,l,r,t,b) AG_SetStyleF((lbl), "padding", "%d %d %d %d", (t),(r),(b),(l))
# define AG_LabelSetPaddingLeft(lbl,v)   AG_SetStyleF((lbl), "padding", "%d 0 0 0", (v))
# define AG_LabelSetPaddingRight(lbl,v)  AG_SetStyleF((lbl), "padding", "0 %d 0 0", (v))
# define AG_LabelSetPaddingTop(lbl,v)    AG_SetStyleF((lbl), "padding", "0 0 %d 0", (v))
# define AG_LabelSetPaddingBottom(lbl,v) AG_SetStyleF((lbl), "padding", "0 0 0 %d", (v))
#endif /* AG_LEGACY */
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_LABEL_H_ */

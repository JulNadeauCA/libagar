/*	Public domain */

#ifndef _AGAR_WIDGET_TREETBL_H_
#define _AGAR_WIDGET_TREETBL_H_

#include <agar/gui/label.h>
#include <agar/gui/scrollbar.h>

#include <agar/gui/begin.h>

#define AG_TREETBL_LABEL_MAX AG_LABEL_MAX

struct ag_treetbl;

typedef char *(*AG_TreetblDataFn)(struct ag_treetbl *, int, int);
typedef int (*AG_TreetblSortFn)(struct ag_treetbl *, int, int, int, int);

enum ag_treetbl_sort_mode {
	AG_TREETBL_SORT_NOT = 0,
	AG_TREETBL_SORT_ASC = 1,
	AG_TREETBL_SORT_DSC = 2
};

typedef struct ag_treetbl_col {
	int cid;				/* Column identifier */
	struct ag_treetbl *tbl;			/* Back pointer to Treetbl */

	Uint idx;				/* Index into row->cell[] */
	Uint flags;
#define AG_TREETBL_COL_DYNAMIC		0x01	/* Updates periodically */
#define AG_TREETBL_COL_EXPANDER		0x02	/* Should hold +/- boxes */
#define AG_TREETBL_COL_RESIZABLE	0x04	/* User-resizable */
#define AG_TREETBL_COL_MOVING		0x08	/* Being displaced */
#define AG_TREETBL_COL_FILL		0x20	/* Expand to remaining space */
#define AG_TREETBL_COL_SELECTED		0x40	/* Column is selected */
#define AG_TREETBL_COL_SORTING		0x80	/* Controls sorting */

	char label[AG_TREETBL_LABEL_MAX];	/* Header text */
	int labelSu;				/* Cached text surface or -1 */
	int w;					/* Column width */
} AG_TreetblCol;

typedef AG_TAILQ_HEAD(ag_treetbl_rowq, ag_treetbl_row) AG_TreetblRowQ;

typedef struct ag_treetbl_cell {
	char *text;
	AG_Surface *image;
} AG_TreetblCell;

typedef struct ag_treetbl_row {
	int rid;			/* Row identifier */
	struct ag_treetbl *tbl;		/* Back pointer to Treetbl */
	AG_TreetblCell *cell;		/* Array of cells */

	Uint flags;
#define AG_TREETBL_ROW_EXPANDED	0x01	/* Tree expanded */
#define AG_TREETBL_ROW_DYNAMIC	0x02	/* Update dynamically */
#define AG_TREETBL_ROW_SELECTED	0x04	/* Row is selected */

	struct ag_treetbl_row *parent;
	AG_TreetblRowQ children;
	AG_TAILQ_ENTRY(ag_treetbl_row) siblings;
	AG_TAILQ_ENTRY(ag_treetbl_row) backstore;
} AG_TreetblRow;

typedef struct ag_treetbl {
	struct ag_widget wid;
	
	Uint flags;
#define AG_TREETBL_MULTI	0x001	/* Allow multiple selections */
#define AG_TREETBL_MULTITOGGLE  0x002
#define AG_TREETBL_REORDERCOLS	0x004	/* Allow column reordering */
#define AG_TREETBL_NODUPCHECKS	0x008	/* Skip checks for duplicate IDs */
#define AG_TREETBL_SORT		0x010	/* Enable sorting */
#define AG_TREETBL_POLLED	0x020	/* Polling mode */
#define AG_TREETBL_HFILL	0x040
#define AG_TREETBL_VFILL	0x080
#define AG_TREETBL_EXPAND	(AG_TREETBL_HFILL|AG_TREETBL_VFILL)

	AG_Rect r;			/* View area */
	int hCol;			/* Header height */
	int hRow;			/* Per-row height */
	int dblclicked;			/* Used by double click */
	int wHint, hHint;		/* Size hint */
	
	Uint n;				/* Column count */
	AG_TreetblCol *column;		/* Column array */

	enum ag_treetbl_sort_mode sortMode;	/* Sorting mode */
	
	AG_TreetblRowQ children;	/* Tree of rows */
	AG_TreetblRowQ backstore;	/* For polling */
	int nExpandedRows;		/* Number of rows visible */
	
	AG_Scrollbar *vBar;		/* Vertical scrollbar */
	AG_Scrollbar *hBar;		/* Horizontal scrollbar */
	
	AG_TreetblDataFn cellDataFn;	/* Callback to get cell data */
	AG_TreetblSortFn sortFn;	/* Compare function */
	
	struct {
		Uint redraw_last;			/* Last drawn */
		Uint redraw_rate;			/* Refresh rate */
		int dirty;				/* Needs update */
		Uint count;				/* Visible rows per view */

		struct ag_treetbl_rowdocket_item {	/* Visible row cache */
			AG_TreetblRow *row;
			Uint depth;
		} *items;
	} visible;
} AG_Treetbl;

__BEGIN_DECLS
extern AG_WidgetClass agTreetblClass;

AG_Treetbl *AG_TreetblNew(void *, Uint, AG_TreetblDataFn, AG_TreetblSortFn);
void        AG_TreetblSizeHint(AG_Treetbl *, int, int);
void        AG_TreetblSetRefreshRate(AG_Treetbl *, Uint);
void        AG_TreetblSetColHeight(AG_Treetbl *, int);
void        AG_TreetblSetSortCol(AG_Treetbl *, AG_TreetblCol *);
void        AG_TreetblSetSortMode(AG_Treetbl *, enum ag_treetbl_sort_mode mode);
void        AG_TreetblSetExpanderCol(AG_Treetbl *, AG_TreetblCol *);

AG_TreetblCol *AG_TreetblAddCol(AG_Treetbl *, int, const char *,
                                const char *, ...);
void           AG_TreetblSelectCol(AG_Treetbl *, AG_TreetblCol *);
int            AG_TreetblSelectColID(AG_Treetbl *, int);
void           AG_TreetblDeselectCol(AG_Treetbl *, AG_TreetblCol *);
int            AG_TreetblDeselectColID(AG_Treetbl *, int);

AG_TreetblRow *AG_TreetblAddRow(AG_Treetbl *, AG_TreetblRow *, int,
                                const char *, ...);

void           AG_TreetblDelRow(AG_Treetbl *, AG_TreetblRow *);
void           AG_TreetblClearRows(AG_Treetbl *);
void           AG_TreetblRestoreRows(AG_Treetbl *);

void           AG_TreetblSelectRow(AG_Treetbl *, AG_TreetblRow *);
void           AG_TreetblDeselectRow(AG_Treetbl *, AG_TreetblRow *);
void           AG_TreetblSelectAll(AG_Treetbl *, AG_TreetblRow *);
void           AG_TreetblDeselectAll(AG_Treetbl *, AG_TreetblRow *);
AG_TreetblRow *AG_TreetblSelectedRow(AG_Treetbl *);

void           AG_TreetblExpandRow(AG_Treetbl *, AG_TreetblRow *);
void           AG_TreetblCollapseRow(AG_Treetbl *, AG_TreetblRow *);

#if 0
void           AG_TreetblCellPrintf(AG_Treetbl *, AG_TreetblRow *, int,
                                    const char *, ...);
#endif

#define	AG_TreetblRowToggle(TV, ROW)				\
	do {							\
		if (NULL == (ROW)) break			\
		if ((ROW)->flags & AG_TREETBL_ROW_EXPANDED)	\
			AG_TreetblCollapseRow(TV, (ROW));	\
		else						\
			AG_TreetblExpandRow(TV, (ROW));		\
	} while (0)

#define AG_TreetblBegin	AG_TreetblClearRows
#define AG_TreetblEnd	AG_TreetblRestoreRows

/*
 * Return a pointer if the row with the given identifer exists in or in a
 * descendant of the rowq. If it does not exist, return NULL.
 */
static __inline__ AG_TreetblRow *
AG_TreetblLookupRowRecurse(AG_TreetblRowQ *searchIn, int rid)
{
	AG_TreetblRow *row, *row2;

	AG_TAILQ_FOREACH(row, searchIn, siblings) {
		if (row->rid == rid)
			return (row);

		if (!AG_TAILQ_EMPTY(&row->children)) {
			row2 = AG_TreetblLookupRowRecurse(&row->children, rid);
			if (row2 != NULL)
				return (row2);
		}
	}
	return (NULL);
}

/*
 * Lookup a row by ID. Return value is only valid as long as the Treetbl
 * remains locked.
 */
static __inline__ AG_TreetblRow *
AG_TreetblLookupRow(AG_Treetbl *tt, int rowID)
{
	AG_TreetblRow *row;

	AG_ObjectLock(tt);
	row = AG_TreetblLookupRowRecurse(&tt->children, rowID);
	AG_ObjectUnlock(tt);
	return (row);
}

/* Delete a row by ID */
static __inline__ int
AG_TreetblDelRowID(AG_Treetbl *tt, int rowID)
{
	AG_TreetblRow *row;
	if ((row = AG_TreetblLookupRow(tt,rowID)) == NULL) { return (-1); }
	AG_TreetblDelRow(tt, row);
	return (0);
}

/* Select a row by ID */
static __inline__ int
AG_TreetblSelectRowID(AG_Treetbl *tt, int rowID)
{
	AG_TreetblRow *row;
	if ((row = AG_TreetblLookupRow(tt,rowID)) == NULL) { return (-1); }
	AG_TreetblSelectRow(tt, row);
	return (0);
}

/* Deselect a row by ID */
static __inline__ int
AG_TreetblDeselectRowID(AG_Treetbl *tt, int rowID)
{
	AG_TreetblRow *row;
	if ((row = AG_TreetblLookupRow(tt,rowID)) == NULL) { return (-1); }
	AG_TreetblDeselectRow(tt, row);
	return (0);
}

/* Expand a row by ID */
static __inline__ int
AG_TreetblExpandRowID(AG_Treetbl *tt, int rowID)
{
	AG_TreetblRow *row;
	if ((row = AG_TreetblLookupRow(tt,rowID)) == NULL) { return (-1); }
	AG_TreetblExpandRow(tt, row);
	return (0);
}

/* Collapse a row by ID */
static __inline__ int
AG_TreetblCollapseRowID(AG_Treetbl *tt, int rowID)
{
	AG_TreetblRow *row;
	if ((row = AG_TreetblLookupRow(tt,rowID)) == NULL) { return (-1); }
	AG_TreetblCollapseRow(tt, row);
	return (0);
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_TREETBL_H_ */

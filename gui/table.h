/*	Public domain	*/

#ifndef _AGAR_WIDGET_TABLE_H_
#define _AGAR_WIDGET_TABLE_H_

#include <agar/gui/scrollbar.h>
#include <agar/gui/menu.h>

#include <agar/gui/begin.h>

#define AG_TABLE_TXT_MAX	128	/* Length of fixed text cells */
#define AG_TABLE_FMT_MAX	16	/* Length of cell specifier string */
#define AG_TABLE_COL_NAME_MAX	48	/* Column name string */
#define AG_TABLE_HASHBUF_MAX	64	/* Buffer used in hash function */

struct ag_table;
	
enum ag_table_selmode {
	AG_TABLE_SEL_ROWS,	/* Select entire rows */
	AG_TABLE_SEL_CELLS,	/* Select individual cells */
	AG_TABLE_SEL_COLS	/* Select entire columns */
};

typedef struct ag_table_popup {
	int m, n;				/* Row/column (-1 = all) */
	AG_Menu *menu;
	AG_MenuItem *item;
	AG_Window *panel;
	AG_SLIST_ENTRY(ag_table_popup) popups;
} AG_TablePopup;

enum ag_table_cell_type {
	AG_CELL_NULL,
	AG_CELL_STRING,
	AG_CELL_INT,
	AG_CELL_UINT,
	AG_CELL_LONG,
	AG_CELL_ULONG,
	AG_CELL_FLOAT,
	AG_CELL_DOUBLE,
	AG_CELL_PSTRING,
	AG_CELL_PINT,
	AG_CELL_PUINT,
	AG_CELL_PLONG,
	AG_CELL_PULONG,
	AG_CELL_PUINT8,
	AG_CELL_PSINT8,
	AG_CELL_PUINT16,
	AG_CELL_PSINT16,
	AG_CELL_PUINT32,
	AG_CELL_PSINT32,
	AG_CELL_PFLOAT,
	AG_CELL_PDOUBLE,
#ifdef AG_HAVE_64BIT
	AG_CELL_INT64,
	AG_CELL_UINT64,
	AG_CELL_PINT64,
	AG_CELL_PUINT64,
#endif
	AG_CELL_POINTER,
	AG_CELL_FN_SU,
	AG_CELL_FN_SU_NODUP,
	AG_CELL_FN_TXT,
	AG_CELL_WIDGET
};

typedef struct ag_table_cell {
	enum ag_table_cell_type type;
	union {
		char s[AG_TABLE_TXT_MAX];
		int i;
		double f;
		void *p;
		long l;
#ifdef AG_HAVE_64BIT
		Uint64 u64;
#else
		Uint32 u64[2];	/* Padding */
#endif
	} data;
	char fmt[AG_TABLE_FMT_MAX];		/* Format string */
	AG_Surface *(*fnSu)(void *, int, int);  /* For AG_CELL_FN_SU */
	void (*fnTxt)(void *, char *, size_t);	/* For AG_CELL_FN_TXT */
	AG_Widget *widget;			/* For AG_CELL_WIDGET */
	int selected;				/* Cell is selected */
	int surface;				/* Named of mapped surface */
	struct ag_table *tbl;			/* Back pointer to Table */
	Uint id;				/* Optional user-specified ID */
	Uint flags;
#define AG_TABLE_CELL_NOCOMPARE	0x01		/* Ignore when comparing cells
						   against backing store. */
	AG_TAILQ_ENTRY(ag_table_cell) cells;	/* In AG_TableBucket */
	AG_TAILQ_ENTRY(ag_table_cell) cells_list; /* In AG_Table */
	Uint nPrev;				/* For SEL_ROWS mode */
} AG_TableCell;

typedef struct ag_table_bucket {
	AG_TAILQ_HEAD_(ag_table_cell) cells;
} AG_TableBucket;

typedef struct ag_table_col {
	char name[AG_TABLE_COL_NAME_MAX];
	int (*sortFn)(const void *, const void *);
	Uint flags;
#define AG_TABLE_COL_FILL	 0x01
#define AG_TABLE_SORT_ASCENDING	 0x02
#define AG_TABLE_SORT_DESCENDING 0x04
#define AG_TABLE_HFILL		 0x08
#define AG_TABLE_VFILL		 0x10
#define AG_TABLE_EXPAND		 (AG_TABLE_HFILL|AG_TABLE_VFILL)
	int selected;			/* Entire column is selected */
	int w;				/* Width (px) */
	int wPct;			/* Width (percent or -1) */
	int x;				/* Current position */
	int surface;			/* Text surface mapping */
	AG_CursorArea *ca;		/* Column resize cursor-change area */
} AG_TableCol;

typedef struct ag_table {
	struct ag_widget wid;
	Uint flags;
#define AG_TABLE_MULTI		0x001	/* Multiple selections (ctrl/shift) */
#define AG_TABLE_MULTITOGGLE	0x002	/* Toggle multiple selections */
#define AG_TABLE_REDRAW_CELLS	0x004	/* Redraw the cells */
#define AG_TABLE_POLL		0x008	/* Table is polled */
#define AG_TABLE_HIGHLIGHT_COLS	0x040	/* Highlight column selection */
#define AG_TABLE_WIDGETS	0x080	/* Embedded widgets are in use */
#define AG_TABLE_MULTIMODE	(AG_TABLE_MULTI|AG_TABLE_MULTITOGGLE)
#define AG_TABLE_NOAUTOSORT	0x100	/* Disable automatic sorting */
#define AG_TABLE_NEEDSORT	0x200	/* Need sorting */
	enum ag_table_selmode selMode;	/* Selection mode */
	int wHint, hHint;		/* Size hint */

	const char *sep;		/* Field separators */
	int hRow;			/* Row height (px) */
	int hCol;			/* Column header height (px) */
	int wColMin;			/* Minimum column width (px) */
	int wColDefault;		/* "Default" pixel width to use in
					   size requisition for "%" and FILL
					   columns */
	int xOffs;			/* Column display offset */
	int mOffs;			/* Row offset (for poll function) */

	AG_TableCol     *cols;		/* Column data */
	AG_TableCell   **cells;		/* Current cell data (sorted rows) */
	AG_TableBucket *cPrev;		/* Saved cells (value hash) */
	Uint            nPrevBuckets;
	AG_TAILQ_HEAD_(ag_table_cell) cPrevList;	

	int n;				/* Number of columns */
	int m;				/* Number of rows */
	int mVis;			/* Maximum number of visible rows */
	int nResizing;			/* Column being resized (or -1) */
	AG_Scrollbar *vbar;		/* Vertical scrollbar */
	AG_Scrollbar *hbar;		/* Horizontal scrollbar */
	AG_Event *poll_ev;		/* Poll event */
	AG_Event *dblClickRowEv;	/* Row double click callback */
	AG_Event *dblClickColEv;	/* Column double click callback */
	AG_Event *dblClickCellEv;	/* Cell double click callback */
	int dblClickedRow;		/* For SEL_ROWS */
	int dblClickedCol;		/* For SEL_COLS */
	int dblClickedCell;		/* For SEL_CELLS */
	Uint32 wheelTicks;		/* For wheel acceleration */
	AG_Timer moveTo;		/* Timers for keyboard motion */
	AG_Rect r;			/* View area */
	int wTot;			/* Total width for all columns */
	AG_Color selColor;		/* Selection color */

	AG_SLIST_HEAD_(ag_table_popup) popups; /* Registered popup menus */

	Uint colAction;
#define AG_TABLE_COL_SELECT	0x01	/* Select column */
#define AG_TABLE_COL_SORT	0x02	/* Set sorting mode */

	AG_Event *clickRowEv;		/* Row double click callback */
	AG_Event *clickColEv;		/* Column double click callback */
	AG_Event *clickCellEv;		/* Cell double click callback */
	Uint nSorting;			/* Index of sorting column
					   (computed from flags) */
	AG_Timer pollTo;		/* For polled table update */
	AG_Timer dblClickTo;		/* For double click */
} AG_Table;

__BEGIN_DECLS
extern AG_WidgetClass agTableClass;

AG_Table *AG_TableNew(void *, Uint);
AG_Table *AG_TableNewPolled(void *, Uint, void (*fn)(AG_Event *),
 			    const char *, ...);
void	  AG_TableSizeHint(AG_Table *, int, int);
#define	  AG_TablePrescale AG_TableSizeHint

void AG_TableSetPollInterval(AG_Table *, Uint);
void AG_TableSetSeparator(AG_Table *, const char *);
void AG_TableSetRowClickFn(AG_Table *, AG_EventFn, const char *, ...);
void AG_TableSetColClickFn(AG_Table *, AG_EventFn, const char *, ...);
void AG_TableSetCellClickFn(AG_Table *, AG_EventFn, const char *, ...);
void AG_TableSetRowDblClickFn(AG_Table *, AG_EventFn, const char *, ...);
void AG_TableSetColDblClickFn(AG_Table *, AG_EventFn, const char *, ...);
void AG_TableSetCellDblClickFn(AG_Table *, AG_EventFn, const char *, ...);
void AG_TableSetColHeight(AG_Table *, int);
void AG_TableSetRowHeight(AG_Table *, int);
void AG_TableSetColMin(AG_Table *, int);
void AG_TableSetDefaultColWidth(AG_Table *, int);
void AG_TableSetSelectionMode(AG_Table *, enum ag_table_selmode);
void AG_TableSetSelectionColor(AG_Table *, Uint8, Uint8, Uint8, Uint8);
void AG_TableSetColumnAction(AG_Table *, Uint);

void	  AG_TableClear(AG_Table *);
void	  AG_TableBegin(AG_Table *);
void	  AG_TableEnd(AG_Table *);
void      AG_TableSort(AG_Table *);

void	  AG_TableInitCell(AG_Table *, AG_TableCell *);
void	  AG_TablePrintCell(const AG_TableCell *, char *, size_t);
void	  AG_TableFreeCell(AG_Table *, AG_TableCell *);

int	  AG_TableAddRow(AG_Table *, const char *, ...);
void	  AG_TableSelectRow(AG_Table *, int);
void	  AG_TableDeselectRow(AG_Table *, int);
void	  AG_TableSelectAllRows(AG_Table *);
void	  AG_TableDeselectAllRows(AG_Table *);
int	  AG_TableRowSelected(AG_Table *, int);

int	  AG_TableAddCol(AG_Table *, const char *, const char *,
	                 int (*)(const void *, const void *));
void	  AG_TableSelectAllCols(AG_Table *);
void	  AG_TableDeselectAllCols(AG_Table *);

void	  AG_TableRedrawCells(AG_Table *);
int	  AG_TableCompareCells(const AG_TableCell *, const AG_TableCell *);

AG_MenuItem *AG_TableSetPopup(AG_Table *, int, int);
int	     AG_TableSaveASCII(AG_Table *, FILE *, char);

/* Return cell at [m,n]. */
static __inline__ AG_TableCell *
AG_TableGetCell(AG_Table *t, int m, int n)
{
	return (&t->cells[m][n]);
}

/* Cell selection control */
static __inline__ int
AG_TableCellSelected(AG_Table *t, int m, int n)
{
	return (t->cells[m][n].selected);
}
static __inline__ void
AG_TableSelectCell(AG_Table *t, int m, int n)
{
	t->cells[m][n].selected = 1;
}
static __inline__ void
AG_TableDeselectCell(AG_Table *t, int m, int n)
{
	t->cells[m][n].selected = 0;
}

/* Column selection control */
static __inline__ int
AG_TableColSelected(AG_Table *t, int n)
{
	return (t->cols[n].selected);
}
static __inline__ void
AG_TableSelectCol(AG_Table *t, int n)
{
	t->cols[n].selected = 1;
}
static __inline__ void
AG_TableDeselectCol(AG_Table *t, int n)
{
	t->cols[n].selected = 0;
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_TABLE_H_ */

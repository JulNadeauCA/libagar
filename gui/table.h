/*	Public domain	*/

#ifndef _AGAR_WIDGET_TABLE_H_
#define _AGAR_WIDGET_TABLE_H_

#include <agar/gui/scrollbar.h>
#include <agar/gui/menu.h>

#include <agar/begin.h>

#define AG_TABLE_TXT_MAX 128
#define AG_TABLE_FMT_MAX 16
#define AG_TABLE_COL_NAME_MAX 48
	
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
#ifdef HAVE_64BIT
	AG_CELL_INT64,
	AG_CELL_UINT64,
	AG_CELL_PINT64,
	AG_CELL_PUINT64,
#endif
	AG_CELL_POINTER,
	AG_CELL_FN_SU,
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
#ifdef HAVE_64BIT
		Uint64 u64;
#else
		Uint32 u64[2];	/* Padding */
#endif
	} data;
	char fmt[AG_TABLE_FMT_MAX];		/* Format string */
	AG_Surface *(*fnSu)(void *, int, int);  /* For AG_CELL_FN_SURFACE */
	void (*fnTxt)(void *, char *, size_t);	/* For AG_CELL_FN_TXT */
	AG_Widget *widget;			/* For AG_CELL_WIDGET */
	int selected;				/* Cell is selected */
	int surface;				/* Named of mapped surface */
} AG_TableCell;

typedef struct ag_table_col {
	char name[AG_TABLE_COL_NAME_MAX];
	int (*sort_fn)(const void *, const void *);
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
	AG_TableCell *pool;		/* Pool of inactive cells */
	Uint         mpool;		/* Number of rows in pool */
} AG_TableCol;

typedef struct ag_table {
	struct ag_widget wid;
	Uint flags;
#define AG_TABLE_MULTI		0x01	/* Multiple selections (ctrl/shift) */
#define AG_TABLE_MULTITOGGLE	0x02	/* Toggle multiple selections */
#define AG_TABLE_REDRAW_CELLS	0x04	/* Redraw the cells */
#define AG_TABLE_POLL		0x08	/* Table is polled */
#define AG_TABLE_HIGHLIGHT_COLS	0x40	/* Highlight column selection */
#define AG_TABLE_WIDGETS	0x80	/* Embedded widgets are in use */
#define AG_TABLE_MULTIMODE	(AG_TABLE_MULTI|AG_TABLE_MULTITOGGLE)
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
	AG_TableCol *cols;		/* Column data */
	AG_TableCell **cells;		/* Row data */
	Uint n;				/* Number of columns */
	Uint m;				/* Number of rows */
	Uint mVis;			/* Maximum number of visible rows */
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
	AG_Timeout incTo, decTo;	/* Timers for keyboard motion */
	AG_Rect r;			/* View area */
	int wTot;			/* Total width for all columns */
	Uint8 selColor[4];		/* Selection color (RGBA) */

	AG_SLIST_HEAD(,ag_table_popup) popups; /* Registered popup menus */
} AG_Table;

__BEGIN_DECLS
extern AG_WidgetClass agTableClass;

AG_Table *AG_TableNew(void *, Uint);
AG_Table *AG_TableNewPolled(void *, Uint, void (*fn)(AG_Event *),
 			    const char *, ...);
void	  AG_TableSizeHint(AG_Table *, int, int);
#define	  AG_TablePrescale AG_TableSizeHint

void AG_TableSetSeparator(AG_Table *, const char *);
void AG_TableSetRowDblClickFn(AG_Table *, AG_EventFn, const char *, ...);
void AG_TableSetColDblClickFn(AG_Table *, AG_EventFn, const char *, ...);
void AG_TableSetCellDblClickFn(AG_Table *, AG_EventFn, const char *, ...);
void AG_TableSetColHeight(AG_Table *, int);
void AG_TableSetRowHeight(AG_Table *, int);
void AG_TableSetColMin(AG_Table *, int);
void AG_TableSetDefaultColWidth(AG_Table *, int);
void AG_TableSetSelectionMode(AG_Table *, enum ag_table_selmode);
void AG_TableSetSelectionColor(AG_Table *, Uint8, Uint8, Uint8, Uint8);

void	  AG_TableFreeCell(AG_Table *, AG_TableCell *);
int	  AG_TablePoolAdd(AG_Table *, Uint, Uint);
void	  AG_TablePoolFree(AG_Table *, Uint);

void	  AG_TableClear(AG_Table *);
void	  AG_TableBegin(AG_Table *);
void	  AG_TableEnd(AG_Table *);

void	  AG_TableInitCell(AG_Table *, AG_TableCell *);
#define	  AG_TableCellSelected(t,m,n) ((t)->cells[m][n].selected)
#define	  AG_TableSelectCell(t,m,n) ((t)->cells[m][n].selected = 1)
#define	  AG_TableDeselectCell(t,m,n) ((t)->cells[m][n].selected = 1)

int	  AG_TableAddRow(AG_Table *, const char *, ...);
void	  AG_TableSelectRow(AG_Table *, Uint);
void	  AG_TableDeselectRow(AG_Table *, Uint);
void	  AG_TableSelectAllRows(AG_Table *);
void	  AG_TableDeselectAllRows(AG_Table *);
int	  AG_TableRowSelected(AG_Table *, Uint);

int	  AG_TableAddCol(AG_Table *, const char *, const char *,
	                 int (*)(const void *, const void *));
void	  AG_TableSelectAllCols(AG_Table *);
void	  AG_TableDeselectAllCols(AG_Table *);
#define	  AG_TableSelectCol(t,n) do { (t)->cols[n].selected = 1; } while (0)
#define	  AG_TableDeselectCol(t,n) do { (t)->cols[n].selected = 0; } while (0)
#define	  AG_TableColSelected(t,n) ((t)->cols[n].selected)

void	  AG_TableRedrawCells(AG_Table *);
int	  AG_TableCompareCells(const AG_TableCell *, const AG_TableCell *);

AG_MenuItem *AG_TableSetPopup(AG_Table *, int, int);
int	     AG_TableSaveASCII(AG_Table *, FILE *, char);
__END_DECLS

#include <agar/close.h>
#endif /* _AGAR_WIDGET_TABLE_H_ */

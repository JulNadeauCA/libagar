/*	Public domain	*/

#ifndef _AGAR_WIDGET_TABLE_H_
#define _AGAR_WIDGET_TABLE_H_

#include <agar/gui/scrollbar.h>
#include <agar/gui/menu.h>
#include <agar/gui/begin.h>

#ifndef AG_TABLE_BUF_MAX
#define AG_TABLE_BUF_MAX (AG_MODEL*2)   /* Buffer size for cell text */
#endif
#ifndef AG_TABLE_COL_NAME_MAX
#define AG_TABLE_COL_NAME_MAX (AG_MODEL)  /* Length of column label */
#endif

#define AG_TABLE_FMT_MAX     20		/* Length of cell specifier string */
#define AG_TABLE_HASHBUF_MAX 64		/* Buffer used in hash function */

struct ag_table;
	
enum ag_table_selmode {
	AG_TABLE_SEL_ROWS,	/* Select entire rows */
	AG_TABLE_SEL_CELLS,	/* Select individual cells */
	AG_TABLE_SEL_COLS,	/* Select entire columns */
	AG_TABLE_SEL_LAST
};

enum ag_table_fn {
	AG_TABLE_FN_POLL,		/* Poll event (AG_TableNewPolled()) */
	AG_TABLE_FN_ROW_CLICK,		/* Clicked a row */
	AG_TABLE_FN_ROW_DBLCLICK,	/* Double-clicked a row */
	AG_TABLE_FN_COL_CLICK,		/* Clicked a column */
	AG_TABLE_FN_COL_DBLCLICK,	/* Double-clicked a column */
	AG_TABLE_FN_CELL_CLICK,		/* Clicked a cell */
	AG_TABLE_FN_CELL_DBLCLICK,	/* Double-clicked a cell */
	AG_TABLE_FN_LAST
};

typedef struct ag_table_popup {
	int m, n;				/* Row/column (-1 = all) */
	AG_Menu     *_Nonnull  menu;		/* Popup menu */
	AG_MenuItem *_Nonnull  item;		/* Popup menu root item */
	AG_Window   *_Nullable panel;		/* Expanded window */
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
	AG_CELL_SINT64,
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
	enum ag_table_cell_type type;		/* Type of cell */
	char fmt[AG_TABLE_FMT_MAX];		/* Format specifier */

	union {
		char s[AG_TABLE_BUF_MAX];
		int i;
		double f;
		void *_Nullable p;
		long l;
#ifdef AG_HAVE_64BIT
		Uint64 u64;
#else
		Uint32 u64[2];
#endif
	} data;

	AG_Surface *_Nonnull (*_Nullable fnSu)(void *_Nullable, int,int);
	AG_Size              (*_Nullable fnTxt)(void *_Nullable, char *_Nonnull,
	                                        AG_Size);

	AG_Widget *_Nullable widget;		/* For AG_CELL_WIDGET */
	int selected;				/* Cell is selected */
	int surface;				/* Named of mapped surface */
	struct ag_table *_Nonnull tbl;		/* Back pointer to Table */
	Uint id;				/* Optional user-specified ID */
	Uint nPrev;				/* For SEL_ROWS mode */

	AG_TAILQ_ENTRY(ag_table_cell) cells;	/* In AG_TableBucket */
	AG_TAILQ_ENTRY(ag_table_cell) cells_list; /* In AG_Table */
} AG_TableCell;

typedef struct ag_table_bucket {
	AG_TAILQ_HEAD_(ag_table_cell) cells;
} AG_TableBucket;

typedef struct ag_table_col {
	char name[AG_TABLE_COL_NAME_MAX];
	int (*_Nullable sortFn)(const void *_Nonnull, const void *_Nonnull);
	Uint flags;
#define AG_TABLE_COL_FILL       0x01    /* Expand to fill any remaining space */
#define AG_TABLE_COL_ASCENDING  0x02    /* Sort rows ascending */
#define AG_TABLE_COL_DESCENDING 0x04    /* Sort rows descending */
#define AG_TABLE_COL_SELECTED   0x10    /* Selection flag */
	int w;				/* Width (effective; px) */
	int wPct;			/* Width in % (or -1) */
	int surface;			/* Text surface mapping */
	AG_CursorArea *_Nullable ca;	/* Column resize cursor-change area */
} AG_TableCol;

typedef struct ag_table {
	struct ag_widget wid;		/* AG_Widget -> AG_Table */
	Uint flags;
#define AG_TABLE_MULTI          0x001	/* Multiple selections (ctrl/shift) */
#define AG_TABLE_MULTITOGGLE    0x002	/* Toggle multiple selections */
#define AG_TABLE_REDRAW_CELLS   0x004	/* Redraw the cells */
#define AG_TABLE_POLL           0x008	/* Table is polled */
#define AG_TABLE_HFILL          0x010
#define AG_TABLE_VFILL          0x020
#define AG_TABLE_EXPAND        (AG_TABLE_HFILL|AG_TABLE_VFILL)
#define AG_TABLE_HIGHLIGHT_COLS 0x040	/* Highlight column selection */
#define AG_TABLE_WIDGETS        0x080	/* Embedded widgets are in use */
#define AG_TABLE_NOAUTOSORT     0x100	/* Disable automatic sorting */
#define AG_TABLE_NEEDSORT       0x200	/* Need sorting */

	enum ag_table_selmode selMode;	/* Selection mode */
	int wHint, hHint;		/* Size hint */

	const char *_Nonnull sep;	/* Field separators */
	int hRow;			/* Row height (px) */
	int hCol;			/* Column header height (px) */
	int wColMin;			/* Minimum column width (px) */
	int wColDefault;		/* "Default" pixel width to use in
					   size requisition for "%" and FILL
					   columns */
	int xOffs;			/* Column display offset */
	int mOffs;			/* Row offset (for poll function) */

	AG_TableCol *_Nullable cols;		 /* Column data */
	AG_TableCell *_Nullable *_Nonnull cells; /* Cell data (sorted rows) */

	AG_TableBucket *_Nonnull cPrev;		 /* Saved (recyclable) cells */
	Uint                     nPrevBuckets;

	int nResizing;			/* Column being resized (or -1) */

	AG_TAILQ_HEAD_(ag_table_cell) cPrevList;	

	int n;				/* Number of columns */
	int m;				/* Number of rows */
	int mVis;			/* Maximum number of visible rows */
	int lineScrollAmount;		/* Wheel scroll increment */

	AG_Scrollbar *_Nonnull vbar;	/* Vertical scrollbar */
	AG_Scrollbar *_Nonnull hbar;	/* Horizontal scrollbar */

	AG_Event *_Nullable fn[AG_TABLE_FN_LAST];  /* Registered callbacks */

	AG_Rect r;			/* View area */
	int wTot;			/* Total width for all columns */

	Uint colAction;
#define AG_TABLE_COL_SELECT	0x01	/* Select column */
#define AG_TABLE_COL_SORT	0x02	/* Set sorting mode */

	AG_SLIST_HEAD_(ag_table_popup) popups; /* Registered popup menus */

	Uint nSorting;			/* Index of sorting column */
	int dblClickedRow;		/* For SEL_ROWS */
	int dblClickedCol;		/* For SEL_COLS */
	int dblClickedCell;		/* For SEL_CELLS */
	AG_Timer moveTo;		/* For keyboard motion */
	AG_Timer pollTo;		/* For polled table update */
	AG_Timer dblClickTo;		/* For double click */
} AG_Table;

#define AGTABLE(p)              ((AG_Table *)(p))
#define AGCTABLE(p)             ((const AG_Table *)(p))
#define AG_TABLE_SELF()          AGTABLE( AG_OBJECT(0,"AG_Widget:AG_Table:*") )
#define AG_TABLE_PTR(n)          AGTABLE( AG_OBJECT((n),"AG_Widget:AG_Table:*") )
#define AG_TABLE_NAMED(n)        AGTABLE( AG_OBJECT_NAMED((n),"AG_Widget:AG_Table:*") )
#define AG_CONST_TABLE_SELF()   AGCTABLE( AG_CONST_OBJECT(0,"AG_Widget:AG_Table:*") )
#define AG_CONST_TABLE_PTR(n)   AGCTABLE( AG_CONST_OBJECT((n),"AG_Widget:AG_Table:*") )
#define AG_CONST_TABLE_NAMED(n) AGCTABLE( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Table:*") )

__BEGIN_DECLS
extern AG_WidgetClass agTableClass;

AG_Table *_Nonnull AG_TableNew(void *_Nullable, Uint);

AG_Table *_Nonnull AG_TableNewPolled(void *_Nullable, Uint,
                                     void (*_Nonnull fn)(AG_Event *_Nonnull),
				     const char *_Nullable, ...);

void AG_TableSetPollInterval(AG_Table *_Nonnull, Uint);

void AG_TableSizeHint(AG_Table *_Nonnull, int, int);
void AG_TableSetSeparator(AG_Table *_Nonnull, const char *_Nonnull);
void AG_TableSetColHeight(AG_Table *_Nonnull, int);
void AG_TableSetRowHeight(AG_Table *_Nonnull, int);
void AG_TableSetColMin(AG_Table *_Nonnull, int);
void AG_TableSetDefaultColWidth(AG_Table *_Nonnull, int);

AG_MenuItem *_Nonnull AG_TableSetPopup(AG_Table *_Nonnull, int,int);

void AG_TableSetFn(AG_Table *_Nonnull, enum ag_table_fn, _Nonnull AG_EventFn,
                   const char *_Nullable, ...);
void AG_TableSetSelectionMode(AG_Table *_Nonnull, enum ag_table_selmode);
void AG_TableSetColumnAction(AG_Table *_Nonnull, Uint);

void AG_TableSort(AG_Table *_Nonnull);
void AG_TableClear(AG_Table *_Nonnull);
void AG_TableBegin(AG_Table *_Nonnull);
void AG_TableEnd(AG_Table *_Nonnull);

void    AG_TableInitCell(AG_Table *_Nonnull, AG_TableCell *_Nonnull);
AG_Size AG_TablePrintCell(const AG_TableCell *_Nonnull, char *_Nonnull, AG_Size);
void    AG_TableFreeCell(AG_Table *_Nonnull, AG_TableCell *_Nonnull);

int  AG_TableAddRow(AG_Table *_Nonnull, const char *_Nonnull, ...);
void AG_TableDelRow(AG_Table *_Nonnull, int);

void AG_TableSelectRow(AG_Table *_Nonnull, int);
void AG_TableDeselectRow(AG_Table *_Nonnull, int);
void AG_TableSelectAllRows(AG_Table *_Nonnull);
void AG_TableDeselectAllRows(AG_Table *_Nonnull);
int  AG_TableRowSelected(AG_Table *_Nonnull, int);

int  AG_TableAddCol(AG_Table *_Nonnull, const char *_Nullable,
                    const char *_Nullable,
		    int (*_Nullable)(const void *_Nonnull, const void *_Nonnull));

void AG_TableSelectAllCols(AG_Table *_Nonnull);
void AG_TableDeselectAllCols(AG_Table *_Nonnull);

int  AG_TableCompareCells(const AG_TableCell *_Nonnull,
                          const AG_TableCell *_Nonnull);

AG_TableCell *_Nonnull AG_TableGetCell(AG_Table *_Nonnull, int,int)
                                      _Pure_Attribute;
int  AG_TableCellSelected(AG_Table *_Nonnull, int,int) _Pure_Attribute;
void AG_TableSelectCell(AG_Table *_Nonnull, int,int);
void AG_TableDeselectCell(AG_Table *_Nonnull, int,int);
int  AG_TableColSelected(AG_Table *_Nonnull, int) _Pure_Attribute;
void AG_TableSelectCol(AG_Table *_Nonnull, int);
void AG_TableDeselectCol(AG_Table *_Nonnull, int);
int  AG_TableSaveASCII(AG_Table *_Nonnull, void *_Nonnull, char);

#ifdef AG_LEGACY
#define AG_TablePrescale(c,r) AG_TableSizeHint((c),(r))
#define AG_TableSetSelectionColor(t,r,g,b,a) {				\
	AG_WCOLOR((t),AG_SELECTION_COLOR).r = r;			\
	AG_WCOLOR((t),AG_SELECTION_COLOR).g = g;			\
	AG_WCOLOR((t),AG_SELECTION_COLOR).b = b;			\
	AG_WCOLOR((t),AG_SELECTION_COLOR).a = a;			\
	AG_Verbose("AG_TableSetSelectionColor() is deprecated\n");	\
}
#endif /* AG_LEGACY */
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_TABLE_H_ */

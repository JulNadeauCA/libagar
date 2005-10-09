/*	$Csoft: table.h,v 1.6 2005/10/04 05:41:24 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TABLE_H_
#define _AGAR_WIDGET_TABLE_H_

#include "begin_code.h"

#define AG_TABLE_TXT_MAX 128
#define AG_TABLE_FMT_MAX 16
#define AG_TABLE_COL_NAME_MAX 48

typedef struct ag_table_cell {
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
#ifdef SDL_HAS_64BIT_TYPE
		AG_CELL_INT64,
		AG_CELL_UINT64,
		AG_CELL_PINT64,
		AG_CELL_PUINT64,
#endif
		AG_CELL_POINTER,
		AG_CELL_FN_SU,
		AG_CELL_FN_TXT,
	} type;
	union {
		char s[AG_TABLE_TXT_MAX];
		int i;
		double f;
		void *p;
		long l;
#ifdef SDL_HAS_64BIT_TYPE
		Uint64 u64;
#endif
	} data;
	char fmt[AG_TABLE_FMT_MAX];		/* Format string */
	SDL_Surface *(*fnSu)(void *, int, int); /* For AG_CELL_FN_SURFACE */
	void *(*fnTxt)(void *, char *, size_t);	/* For AG_CELL_FN_TEXT */
	int selected;				/* Cell is selected */
	int surface;				/* Named of mapped surface */
} AG_TableCell;

typedef struct ag_table_col {
	char name[AG_TABLE_COL_NAME_MAX];
	int (*sort_fn)(const void *, const void *);
	int flags;
#define AG_TABLE_COL_FILL	 0x01
#define AG_TABLE_SORT_ASCENDING	 0x02
#define AG_TABLE_SORT_DESCENDING 0x04
	int selected;			/* Entire column is selected */
	int w;				/* Width in pixel */
	int x;				/* Current position */
	int surface;			/* Text surface mapping */
	AG_TableCell *pool;		/* Pool of inactive cells */
	u_int        mpool;
} AG_TableCol;

struct ag_scrollbar;

typedef struct ag_table {
	struct ag_widget wid;
	u_int flags;
#define AG_TABLE_MULTI		0x01	/* Multiple selections (ctrl/shift) */
#define AG_TABLE_MULTITOGGLE	0x02	/* Toggle multiple selections */
#define AG_TABLE_REDRAW_CELLS	0x04	/* Redraw the cells */
#define AG_TABLE_MULTIMODE	(AG_TABLE_MULTI|AG_TABLE_MULTITOGGLE)
	enum ag_table_selmode {
		AG_TABLE_SEL_ROWS,	/* Select entire rows */
		AG_TABLE_SEL_CELLS,	/* Select individual cells */
		AG_TABLE_SEL_COLS	/* Select entire columns */
	} selmode;
	void *selected_row;		/* Default `selected-row' binding */
	void *selected_col;		/* Default `selected-col' binding */
	void *selected_cell;		/* Default `selected-cell' binding */
	int prew, preh;			/* Prescale hint */

	AG_Mutex lock;
	int row_h;			/* Row height in pixels */
	int col_h;			/* Column header height in pixels */
	int xoffs;			/* Column display offset */
	int moffs;			/* Row offset (for poll funciton) */
	AG_TableCol *cols;		/* Column data */
	AG_TableCell **cells;		/* Row data */
	u_int n;			/* Number of columns */
	u_int m;			/* Number of rows */
	u_int mVis;			/* Maximum number of visible rows */
	int nResizing;			/* Column being resized (or -1) */
	struct ag_scrollbar *vbar;	/* Vertical scrollbar */
	struct ag_scrollbar *hbar;	/* Horizontal scrollbar */
	AG_Event *poll_ev;		/* Poll event */
} AG_Table;

__BEGIN_DECLS
AG_Table *AG_TableNew(void *, u_int);
AG_Table *AG_TablePolled(void *, u_int, void (*fn)(AG_Event *),
 			 const char *, ...);
void	  AG_TableInit(AG_Table *, u_int);
void	  AG_TableScale(void *, int, int);
void	  AG_TableDraw(void *);
void	  AG_TableDestroy(void *);
void	  AG_TablePrescale(AG_Table *, const char *, int);

void	  AG_TableFreeCell(AG_Table *, AG_TableCell *);
int	  AG_TablePoolAdd(AG_Table *, u_int, u_int);
void	  AG_TablePoolFree(AG_Table *, u_int);

void	  AG_TableClear(AG_Table *);
void	  AG_TableBegin(AG_Table *);
void	  AG_TableEnd(AG_Table *);

void	  AG_TableInitCell(AG_Table *, AG_TableCell *);
#define	  AG_TableCellSelected(t,m,n) ((t)->cells[m][n].selected)

int	  AG_TableAddRow(AG_Table *, const char *, ...);
void	  AG_TableSelectRow(AG_Table *, u_int);
void	  AG_TableDeselectRow(AG_Table *, u_int);
void	  AG_TableDeselectAllRows(AG_Table *);
int	  AG_TableRowSelected(AG_Table *, u_int);

int	  AG_TableAddCol(AG_Table *, const char *, const char *,
	                 int (*)(const void *, const void *));
void	  AG_TableSelectAllCols(AG_Table *);
void	  AG_TableDeselectAllCols(AG_Table *);
#define	  AG_TableColSelected(t,n) ((t)->cols[n].selected)

void	  	  AG_TableUpdateScrollbars(AG_Table *);
__inline__ void	  AG_TableRedrawCells(AG_Table *);
__inline__ int	  AG_TableCompareCells(const AG_TableCell *,
			               const AG_TableCell *);

int	  AG_TableSaveASCII(AG_Table *, FILE *, char);

__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TABLE_H_ */

/*	Public domain */

#ifndef _AGAR_WIDGET_TABLEVIEW_H_
#define _AGAR_WIDGET_TABLEVIEW_H_

#ifdef _AGAR_INTERNAL
#include <gui/label.h>
#include <gui/scrollbar.h>
#else
#include <agar/gui/label.h>
#include <agar/gui/scrollbar.h>
#endif

#include "begin_code.h"

#define AG_TABLEVIEW_LABEL_MAX AG_LABEL_MAX

typedef Uint32 AG_TableviewRowID;
typedef Uint32 AG_TableviewColID;

struct ag_tableview;

typedef char *(*AG_TableviewDataFn)(struct ag_tableview *, AG_TableviewColID,
		AG_TableviewRowID);
typedef int (*AG_TableviewSortFn)(AG_TableviewColID, AG_TableviewRowID,
	      AG_TableviewRowID, char);

enum {
	AG_TABLEVIEW_SORT_NOT = 0,
	AG_TABLEVIEW_SORT_ASC = 1,
	AG_TABLEVIEW_SORT_DSC = 2,
	 
	AG_TABLEVIEW_SORTED_AFIRST = 1,
	AG_TABLEVIEW_SORTED_BFIRST = 2
};

typedef struct ag_tableview_col {
	AG_TableviewColID cid;
	Uint idx;			/* Index into row->cell[] */

	/* Flags */
	int mousedown :1;
	int moving :1;
	int editable :1;
	int resizable :1;
	int update :1;
	int fill :1;
	int dynamic :1;

	char label[AG_TABLEVIEW_LABEL_MAX];	/* Header text */
	SDL_Surface *label_img;			/* Rendered header text */
	int label_id;
	int w;					/* Column width */
} AG_TableviewCol;

TAILQ_HEAD(ag_tableview_rowq, ag_tableview_row);

typedef struct ag_tableview_row {
	AG_TableviewRowID rid;
	struct ag_tableview_cell {
		char *text;
		SDL_Surface *image;
	} *cell;
	int selected : 1;
	int expanded : 1;
	int dynamic : 1;
	struct ag_tableview_row *parent;
	struct ag_tableview_rowq children;
	void *userp;
	TAILQ_ENTRY(ag_tableview_row) siblings;
	TAILQ_ENTRY(ag_tableview_row) backstore;
} AG_TableviewRow;

typedef struct ag_tableview {
	struct ag_widget wid;
	
	/* child widgets */
	AG_Scrollbar *sbar_v;		/* Vertical scrollbar */
	AG_Scrollbar *sbar_h;		/* Horizontal scrollbar */
	//AG_Textbox *editbox;		/* Cell edition widget */
	
	AG_Mutex lock;
	AG_TableviewDataFn data_callback;	/* Callback to get cell data */
	AG_TableviewSortFn sort_callback;	/* Callback to compare */
	
	/* Flags */
	int selmulti :1;		/* Allow more than 1 select */
	int selsingle :1;
	int selnoclear :1;		/* Keep at least 1 selection */
	int reordercols :1;		/* Allow column reordering */
	int header :1;			/* Draw column headings */
	int sort :1;			/* Do sort procedures */
	int locked :1;			/* Table format is set */
	int polled :1;

	int head_height;		/* Header height */
	int row_height;			/* Per-row height */
	int dblclicked;			/* Used by double click */
	/*int keymoved;			Used by key repeat */
	int prew, preh;			/* Size hint */
	
	/* columns */
	Uint columncount;
	struct ag_tableview_col *column;
	char sortMode;			/* Sort mode (a or b) */
	
	/*
	 * Special columns - columns with a unique purpose. ID_INVALID if
	 * unused.
	 */
	AG_TableviewColID sortColumn;		/* Column we sort by */
	AG_TableviewColID enterEdit;		/* Column we edit on enter */
	AG_TableviewColID expanderColumn;	/* Column for +/- boxes */
	
	/* rows */
	struct ag_tableview_rowq children;	/* List of rows */
	struct ag_tableview_rowq backstore;	/* List of saved rows */
	int expandedrows;			/* Number of rows visible */
	
	/* drawing hints */
	struct {
		Uint redraw_last;
		Uint redraw_rate;
		int dirty;
		Uint count;
		struct ag_tableview_rowdocket_item {
			struct ag_tableview_row *row;
			Uint depth;
		} *items;
	} visible;
} AG_Tableview;

/* Flags for AG_TableviewColAdd() */
#define AG_TABLEVIEW_COL_EDITABLE	0x01	/* Cells are editable */
#define AG_TABLEVIEW_COL_KEYEDIT	0x02	/* Begin edits on enter */
#define AG_TABLEVIEW_COL_NORESIZE	0x04	/* Disallow resizing */
#define AG_TABLEVIEW_COL_UPDATE		0x08	/* Updates periodically */
#define AG_TABLEVIEW_COL_DYNAMIC	0x20	/* Uses the callback */
#define AG_TABLEVIEW_COL_EXPANDER	0x40	/* Should hold +/- boxes */
#define AG_TABLEVIEW_COL_FILL		0x80	/* Fill remaining space */

/* Flags for AG_TableviewNew() */
#define AG_TABLEVIEW_SELMULTI	 0x001 /* Multiple selections (ctrl/shift) */
#define AG_TABLEVIEW_SELSINGLE	 0x002
#define AG_TABLEVIEW_SELNOCLEAR	 0x004
#define AG_TABLEVIEW_HORIZ	 0x008 /* Can scroll horizontally if needed */
#define AG_TABLEVIEW_REORDERCOLS 0x010 /* Users may reorder the columns */
#define AG_TABLEVIEW_NOHEADER	 0x020 /* do not display the header */
#define AG_TABLEVIEW_NOSORT	 0x040 /* do not sort. header not clickable */
#define AG_TABLEVIEW_POLLED	 0x080 /* remember selections */
#define AG_TABLEVIEW_HFILL	 0x100
#define AG_TABLEVIEW_VFILL	 0x200
#define AG_TABLEVIEW_EXPAND	 (AG_TABLEVIEW_HFILL|AG_TABLEVIEW_VFILL)

/* Flags for tableview_add_row() */
#define AG_TABLEVIEW_STATIC_ROW	0x01	/* Don't update row dynamically */

__BEGIN_DECLS
extern const AG_WidgetClass agTableviewClass;

AG_Tableview	*AG_TableviewNew(void *, Uint, AG_TableviewDataFn,
		                 AG_TableviewSortFn);
void		 AG_TableviewSizeHint(AG_Tableview *, const char *, int);
#define		 AG_TableviewPrescale AG_TableviewSizeHint
void		 AG_TableviewSetUpdate(AG_Tableview *, Uint);

AG_TableviewCol *AG_TableviewColAdd(AG_Tableview *, int, AG_TableviewColID,
			            const char *, const char *);
void		 AG_TableviewColSelect(AG_Tableview *, AG_TableviewColID);
AG_TableviewRow *AG_TableviewRowGet(AG_Tableview *, AG_TableviewRowID);
AG_TableviewRow *AG_TableviewRowAddFn(AG_Tableview *, int, AG_TableviewRow *,
			              void *, AG_TableviewRowID, ...);
AG_TableviewRow *AG_TableviewRowSelected(AG_Tableview *);

void AG_TableviewRowDel(AG_Tableview *, AG_TableviewRow *);
void AG_TableviewRowDelAll(AG_Tableview *);
void AG_TableviewRowRestoreAll(AG_Tableview *);
void AG_TableviewRowSelect(AG_Tableview *, AG_TableviewRow *);
void AG_TableviewRowSelectAll(AG_Tableview *, AG_TableviewRow *);
void AG_TableviewRowDeselectAll(AG_Tableview *, AG_TableviewRow *);
void AG_TableviewRowExpand(AG_Tableview *, AG_TableviewRow *);
void AG_TableviewRowCollapse(AG_Tableview *, AG_TableviewRow *);
void AG_TableviewRowExpandAll(AG_Tableview *, AG_TableviewRow *);
void AG_TableviewRowCollapseAll(AG_Tableview *, AG_TableviewRow *);
void AG_TableviewCellPrintf(AG_Tableview *, AG_TableviewRow *, int,
		           const char *, ...);

#define AG_TableviewRowGetID(ROW) (*(AG_TableviewRowID *)(ROW))
#define AG_TableviewRowAdd(...) AG_TableviewRowAddFn(__VA_ARGS__, -1)
#define AG_TableviewRowDeselect(TV, ROW) ((ROW)->selected = 0)
#define	AG_TableviewRowToggle(TV, ROW)				\
	do {							\
		if (NULL == (ROW)) break			\
		if ((ROW)->flags & TABLEVIEW_ROW_EXPANDED)	\
			AG_TableviewRowCollapse(TV, (ROW));	\
		else						\
			AG_TableviewRowExpand(TV, (ROW));	\
	} while (0)

#define AG_TableviewRowIDAdd(TV, ID, IDNEW, USERP) \
	AG_TableviewRowAdd((TV), AG_TableviewRowGet((TV), (ID)), (USERP), \
	(IDNEW))

#define AG_TableviewRowIDDel(TV, ID) \
	AG_TableviewRowDelete((TV), AG_TableviewRowGet((TV), (ID)))

#define AG_TableviewRowIDSelect(TV, ID) \
	AG_TableviewRowSelect((TV), AG_TableviewRowGet((TV), (ID)))

#define AG_TableviewRowIDDeselect(TV, ID)				\
	do {								\
		AG_TableviewRow *_row = AG_TableviewRowGet((TV), (ID)); \
		if (_row != NULL) _row->selected = 0; \
	} while(0)

#define AG_TableviewRowIDSelectAll(TV, ID) \
	AG_TableviewRowSelectAll((TV), AG_TableviewRowGet((TV), (ID)))

#define AG_TableviewRowIDDeselect_all(TV, ID) \
	AG_TableviewRowDeselectAll((TV), AG_TableviewRowGet((TV), (ID)))

#define AG_TableviewRowIDExpand(TV, ID) \
	AG_TableviewRowExpand((TV), AG_TableviewRowGet((TV), (ID)))

#define AG_TableviewRowIDCollapse(TV, ID) \
	AG_TableviewRowCollapse((TV), AG_TableviewRowGet((TV), (ID)))

#define AG_TableviewRowIDToggle(TV, ID)					\
	do {								\
		AG_TableviewRow *_row = AG_TableviewRowGet((TV), (ID)); \
		if (NULL == _row) break;				\
		if (_row->flags & TABLEVIEW_ROW_EXPANDED) {		\
			AG_TableviewRowCollapse((TV), _row);		\
		} else {						\
			AG_TableviewRowExpand((TV), _row);		\
		}							\
	} while(0)
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TABLEVIEW_H_ */

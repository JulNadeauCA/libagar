/*	$Csoft: tableview.h,v 1.6 2005/03/11 08:56:33 vedge Exp $	*/
/*	Public domain */

#ifndef _AGAR_WIDGET_TABLEVIEW_H_
#define _AGAR_WIDGET_TABLEVIEW_H_

#include "begin_code.h"

#define TABLEVIEW_LABEL_MAX	128

typedef Uint32 rowID;
typedef Uint32 colID;

typedef char *(*datafunc)(colID, rowID);
typedef int (*compfunc)(colID, rowID, rowID, char mode);

enum {
	TABLEVIEW_SORT_NOT = 0,
	TABLEVIEW_SORT_ASC = 1,
	TABLEVIEW_SORT_DSC = 2,
	 
	TABLEVIEW_SORTED_AFIRST = 1,
	TABLEVIEW_SORTED_BFIRST = 2
};

struct tableview_column {
	colID cid;
	u_int idx;		/* Index into row->cell[] */

	/* Flags */
	int mousedown :1;
	int moving :1;
	int editable :1;
	int resizable :1;
	int update :1;
	int fill :1;
	int dynamic :1;

	char label[TABLEVIEW_LABEL_MAX];/* Header text */
	SDL_Surface *label_img;		/* Rendered header text */
	int w;				/* Column width */
};

TAILQ_HEAD(tableview_rowq, tableview_row);

struct tableview_row {
	rowID rid;
	struct cell {
		char *text;
		SDL_Surface *image;
	} *cell;
	int selected : 1;
	int expanded : 1;
	struct tableview_row *parent;
	TAILQ_ENTRY(tableview_row) siblings;
	struct tableview_rowq children;
};

struct tableview {
	struct widget wid;
	
	/* child widgets */
	struct scrollbar *sbar_v;		/* Vertical scrollbar */
	struct scrollbar *sbar_h;		/* Horizontal scrollbar */
	//struct textbox *editbox;		/* cell edition widget */
	
	/* mutex lock required below here */
	pthread_mutex_t lock;
	
	datafunc data_callback;		/* Callback to get cell data */
	compfunc sort_callback;		/* Callback to compare */
	
	/* Flags */
	int selmulti :1;		/* Allow more than 1 select */
	int selsingle :1;
	int selnoclear :1;		/* Keep at least 1 selection */
	int reordercols :1;		/* Allow column reordering */
	int header :1;			/* Draw column headings */
	int sort :1;			/* Do sort procedures */
	int locked :1;			/* Table format is set */

	int head_height;		/* Header height */
	int row_height;			/* Per-row height */
	int dblclicked;			/* Used by double click */
	/*int keymoved;			Used by key repeat */
	int prew, preh;			/* Prescale hint */
	
	/* columns */
	u_int			 columncount;
	struct tableview_column *column;
	char sortMode;			/* Sort mode (a or b) */
	
	/*
	 * Special columns - columns with a unique purpose. ID_INVALID if
	 * unused.
	 */
	colID sortColumn;		/* Column we sort by */
	colID enterEdit;		/* Column we edit on enter */
	colID expanderColumn;		/* Column for +/- boxes */
	
	/* rows */
	struct tableview_rowq children;		/* List of rows */
	int expandedrows;			/* Number of rows visible */
	
	/* drawing hints */
	struct {
		u_int redraw_last;
		u_int redraw_rate;
		int dirty;
		u_int count;
		struct rowdocket_item {
			struct tableview_row *row;
			u_int depth;
		} *items;
	} visible;
};

/* Flags for tableview_col_add() */
#define TABLEVIEW_COL_EDITABLE	0x01	/* Cells are editable */
#define TABLEVIEW_COL_ENTEREDIT	0x02	/* Begin edits on enter */
#define TABLEVIEW_COL_RESIZABLE	0x04	/* Should allow resizing */
#define TABLEVIEW_COL_UPDATE	0x08	/* Updates periodically */
#define TABLEVIEW_COL_FILL	0x10	/* Fills all unused space */
#define TABLEVIEW_COL_DYNAMIC	0x20	/* Uses the callback */
#define TABLEVIEW_COL_EXPANDER	0x40	/* Should hold +/- boxes */

/* Flags for tableview_new() and tableview_init() */
#define TABLEVIEW_SELMULTI	0x01 /* Multiple selections (ctrl/shift) */
//#define TABLEVIEW_SELSINGLE	0x02
//#define TABLEVIEW_SELNOCLEAR	0x04
#define TABLEVIEW_HORIZ		0x08 /* Can scroll horizontally if needed */
#define TABLEVIEW_REORDERCOLS	0x10 /* Users may reorder the columns */
#define TABLEVIEW_NOHEADER	0x20 /* do not display the header */
#define TABLEVIEW_NOSORT	0x40 /* do not sort. header not clickable */

__BEGIN_DECLS
void tableview_destroy(void *p);
void tableview_scale(void *, int, int);
void tableview_draw(void *);

struct tableview *tableview_new(void *, int, datafunc, compfunc);
void tableview_init(struct tableview *, int, datafunc, compfunc);
void tableview_prescale(struct tableview *, const char *, int);
void tableview_set_update(struct tableview *, u_int);

void tableview_col_add(struct tableview *, int, colID, const char *, char *);

#define tableview_row_getID(ROW) (*(rowID *)(ROW))

struct tableview_row *tableview_row_get(struct tableview *, rowID);
struct tableview_row *tableview_row_addfn(struct tableview *, int,
				          struct tableview_row *, rowID, ...);
#define tableview_row_add(...) \
	tableview_row_addfn(__VA_ARGS__, -1)

void tableview_row_del(struct tableview *, struct tableview_row *);
void tableview_row_del_all(struct tableview *);
void tableview_row_select(struct tableview *, struct tableview_row *);

#define tableview_row_deselect(TV, ROW) \
	((ROW)->flags &= ~TABLEVIEW_ROW_SELECTED)

void tableview_row_select_all(struct tableview *, struct tableview_row *);
void tableview_row_deselect_all(struct tableview *, struct tableview_row *);
void tableview_row_expand(struct tableview *, struct tableview_row *);
void tableview_row_collapse(struct tableview *, struct tableview_row *);
void tableview_row_expand_all(struct tableview *, struct tableview_row *);
void tableview_row_collapse_all(struct tableview *, struct tableview_row *);

#define	tableview_row_toggle(TV, ROW)				\
	do {							\
		if (NULL == (ROW)) break			\
		if ((ROW)->flags & TABLEVIEW_ROW_EXPANDED)	\
			tableview_row_collapse(TV, (ROW));	\
		else						\
			tableview_row_expand(TV, (ROW));	\
	} while (0)

#define tableview_rowid_add(TV, ID, IDNEW) \
	tableview_row_add((TV), tableview_row_get((TV), (ID)), (IDNEW))

#define tableview_rowid_delete(TV, ID) \
	tableview_row_delete((TV), tableview_row_get((TV), (ID)))

#define tableview_rowid_select(TV, ID) \
	tableview_row_select((TV), tableview_row_get((TV), (ID)))

#define tableview_rowid_deselect(TV, ID)				\
	do {								\
		struct tableview_row *_row = tableview_row_get((TV), (ID)); \
		if (_row) _row->flags &= ~TABLEVIEW_ROW_SELECTED;	\
	} while(0)

#define tableview_rowid_select_all(TV, ID) \
	tableview_row_select_all((TV), tableview_row_get((TV), (ID)))

#define tableview_rowid_deselect_all(TV, ID) \
	tableview_row_deselect_all((TV), tableview_row_get((TV), (ID)))

#define tableview_rowid_expand(TV, ID) \
	tableview_row_expand((TV), tableview_row_get((TV), (ID)))

#define tableview_rowid_collapse(TV, ID) \
	tableview_row_collapse((TV), tableview_row_get((TV), (ID)))
/*
#define tableview_rowid_expand_all(TV, ID) \
	tableview_row_expand_all(TV, tableview_row_get(TV, ID))
#define tableview_rowid_collapse_all(TV, ID) \
	tableview_row_collapse_all(TV, tableview_row_get(TV, ID))
*/

#define tableview_rowid_toggle(TV, ID)					\
	do {								\
		struct tableview_row *_row = tableview_row_get((TV), (ID)); \
		if (NULL == _row) break;				\
		if (_row->flags & TABLEVIEW_ROW_EXPANDED) {		\
			tableview_row_collapse((TV), _row);		\
		} else {						\
			tableview_row_expand((TV), _row);		\
		}							\
	} while(0)

__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TABLEVIEW_H_ */

/*	Public domain	*/

#ifndef _AGAR_WIDGET_TABLEVIEW_H_
#define _AGAR_WIDGET_TABLEVIEW_H_

#include "begin_code.h"

typedef Uint32 rowID;
typedef Uint32 colID;

typedef char *(*datafunc)(colID, rowID);
typedef int (*compfunc)(colID, rowID, rowID, char mode);

enum {
    tableview_sort_not = 0,
    tableview_sort_asc = 1,
    tableview_sort_dsc = 2,
    
    tableview_sorted_afirst = 1,
    tableview_sorted_bfirst = 2
};

/* ********
    Flags for tableview_col_add() */
#define TABLEVIEW_COL_EDITABLE   0x01 /* this column's cells are editable */
#define TABLEVIEW_COL_ENTEREDIT  0x02 /* this column's begin edits on enter */
#define TABLEVIEW_COL_RESIZABLE  0x04 /* this column should allow resizing */
#define TABLEVIEW_COL_UPDATE     0x08 /* this column updates periodically */
#define TABLEVIEW_COL_FILL       0x10 /* this column fills all unused space */
#define TABLEVIEW_COL_DYNAMIC    0x20 /* this column uses the callback */
#define TABLEVIEW_COL_EXPANDER   0x40 /* this column should hold +/- boxes */

/* ********
    Flags for tableview_new() and tableview_init() */
#define TABLEVIEW_SELMULTI       0x01 /* Multiple selections (ctrl/shift) */
//#define TABLEVIEW_SELSINGLE      0x02
//#define TABLEVIEW_SELNOCLEAR     0x04
#define TABLEVIEW_HORIZ          0x08 /* Can scroll horizontally if needed */
#define TABLEVIEW_REORDERCOLS    0x10 /* Users may reorder the columns */
#define TABLEVIEW_NOHEADER       0x20 /* do not display the header */
#define TABLEVIEW_NOSORT         0x40 /* do not sort. header not clickable */

__BEGIN_DECLS

/* internal tableview functions */
void                tableview_destroy(void *p);
void                tableview_scale(void *, int, int);
void                tableview_draw(void *);

/* general tableview functions */
struct tableview   *tableview_new(void *, int, datafunc, compfunc);
void                tableview_init(struct tableview *, int, datafunc,
                            compfunc);
void                tableview_prescale(struct tableview *, const char *, int);
void                tableview_set_update(struct tableview *, unsigned int);

/* column functions */
void                tableview_col_add(struct tableview *, int, colID, 
                        const char *, char *);

/* row functions */
#define             tableview_row_getID(ROW) (*(rowID *)ROW)
struct tableview_row *tableview_row_get(struct tableview *, rowID);
struct tableview_row *tableview_row_addfn(struct tableview *, int, 
                        struct tableview_row *, rowID, ...);
#define             tableview_row_add(...) tableview_row_addfn(__VA_ARGS__, -1)
void                tableview_row_del(struct tableview *, 
                        struct tableview_row *);
void                tableview_row_del_all(struct tableview *);
void                tableview_row_select(struct tableview *, 
                        struct tableview_row *);
#define             tableview_row_deselect(TV, ROW) \
                        ((ROW)->flags &= ~TABLEVIEW_ROW_SELECTED)
void                tableview_row_select_all(struct tableview *, 
                        struct tableview_row *);
void                tableview_row_deselect_all(struct tableview *,
                        struct tableview_row *);
void                tableview_row_expand(struct tableview *,
                        struct tableview_row *);
void                tableview_row_collapse(struct tableview *,
                        struct tableview_row *);
void                tableview_row_expand_all(struct tableview *,
                        struct tableview_row *);
void                tableview_row_collapse_all(struct tableview *, 
                        struct tableview_row *);
#define             tableview_row_toggle(TV, ROW)                   \
                        do{ if(NULL == ROW) break;                  \
                            if(ROW->flags & TABLEVIEW_ROW_EXPANDED) \
                                tableview_row_collapse(TV, ROW);    \
                            else                                    \
                                tableview_row_expand(TV, ROW);      \
                        } while(0)

/* rowID functions */
#define tableview_rowid_add(TV, ID, IDNEW)                              \
            tableview_row_add(TV, tableview_row_get(TV, ID), IDNEW)
#define tableview_rowid_delete(TV, ID)                                  \
            tableview_row_delete(TV, tableview_row_get(TV, ID))
#define tableview_rowid_select(TV, ID)                                  \
            tableview_row_select(tv, tableview_row_get(TV, ID))
#define tableview_rowid_deselect(TV, ID)                                \
            do{ struct tableview_row *row = tableview_row_get(TV, ID);  \
                if(row) row->flags &= ~TABLEVIEW_ROW_SELECTED;          \
            } while(0)
#define tableview_rowid_select_all(TV, ID)                              \
            tableview_row_select_all(TV, tableview_row_get(TV, ID))
#define tableview_rowid_deselect_all(TV, ID)                            \
            tableview_row_deselect_all(TV, tableview_row_get(TV, ID))
#define tableview_rowid_expand(TV, ID)                                  \
            tableview_row_expand(TV, tableview_row_get(TV, ID))
#define tableview_rowid_collapse(TV, ID)                                \
            tableview_row_collapse(TV, tableview_row_get(TV, ID))
/*
#define tableview_rowid_expand_all(TV, ID)                              \
            tableview_row_expand_all(TV, tableview_row_get(TV, ID))
#define tableview_rowid_collapse_all(TV, ID)                            \
            tableview_row_collapse_all(TV, tableview_row_get(TV, ID))
*/
#define tableview_rowid_toggle(TV, ID)                                  \
            do{ struct tableview_row *row = tableview_row_get(TV, ID);  \
                if(NULL == row) break;                                  \
                if(row->flags & TABLEVIEW_ROW_EXPANDED)                 \
                    tableview_row_collapse(TV, row);                    \
                else                                                    \
                    tableview_row_expand(TV, row);                      \
            } while(0)
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TABLEVIEW_H_ */
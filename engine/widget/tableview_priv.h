#ifndef _AGAR_WIDGET_TABLEVIEW_PRIV_H_
#define _AGAR_WIDGET_TABLEVIEW_PRIV_H_

/* XXX req begin_code.h and end_code.h? */

#define LBL_LEN  128

#define ID_INVALID ((unsigned int)-1)

/* worker function for foreach_visible_column()
 * arguments: tableview, visible_start, visible_end,
 * visible_index, and the argument passed to foreach_.
 * Should return 0 if foreach_ should stop.
 */
typedef int (*visible_do)(struct tableview *, int, int, Uint32,
    void *, void *);

/* ********
    tableview_column column struct */
struct tableview_column {
    colID           cid;
    unsigned int    idx; /* this column's data is at row->cell[idx] */
    /* Flags */
    int             mousedown   :1;
    int             moving      :1;
    int             editable    :1;
    int             resizable   :1;
    int             update      :1;
    int             fill        :1;
    int             dynamic     :1;

    char		    label[LBL_LEN]; /* header text */
    SDL_Surface    *label_img;      /* rendered header text */
    int             w;              /* column width */
};


/* ********
    tableview_rowq struct */
TAILQ_HEAD(tableview_rowq, tableview_row);


/* ********
    tableview_row row struct */
struct tableview_row {
    rowID                       rid;
    
    struct cell {
        char           *text;
        SDL_Surface    *image;
    }                          *cell;
    
    /* Flags */
    int                         selected    :1;
    int                         expanded    :1;
    
    struct tableview_row       *parent;
    TAILQ_ENTRY(tableview_row)  siblings;
    struct tableview_rowq       children;
};


/* ********
    tableview widget struct */
struct tableview {
    struct widget               wid;
    
    /* child widgets */
    struct scrollbar           *sbar_v;		/* Vertical scrollbar */
    struct scrollbar           *sbar_h;		/* Horizontal scrollbar */
    //struct textbox             *editbox;    /* cell edition widget */
    
    /* mutex lock required below here */
    pthread_mutex_t             lock;
    
    datafunc                    data_callback;  /* callback to get cell data */
    compfunc                    sort_callback;  /* callback to compare */
    
    /* Flags */
    int                         selmulti    :1; /* allow more than 1 select */
    int                         selsingle   :1;
    int                         selnoclear  :1; /* keep at least 1 selection */
    int                         reordercols :1; /* allow column reordering */
    int                         header      :1; /* draw column headings */
    int                         sort        :1; /* do sort procedures */
    int                         locked      :1; /* table format is set */

    int                         head_height;    /* header height */
    int                         row_height;     /* per-row height */
    int                         dblclicked;     /* Used by double click */
    /*int                         keymoved;        Used by key repeat */
    int                         prew, preh;     /* Prescale hint */
    
    /* columns */
    unsigned int                columncount;    /* column count */
    struct tableview_column    *column;         /* column structure array */
    char                        sortMode;       /* sort mode (a or b) */
    
    /* special columns - columns with a unique purpose. ID_INVALID if unused */
    colID                       sortColumn;     /* column we sort by */
    colID                       enterEdit;      /* column we edit on enter */
    colID                       expanderColumn; /* column for +/- boxes */
    
    /* rows */
    struct tableview_rowq       children;       /* list of rows */
    int                         expandedrows;   /* number of rows visible */
    
    /* drawing hints */
    struct {
        unsigned int                    redraw_last, redraw_rate;
        int                             dirty;
        unsigned int                    count;
        struct rowdocket_item {
            struct tableview_row               *row;
            unsigned int                        depth;
        }                              *items;
    }                           visible;
};

#endif /* _AGAR_WIDGET_TABLEVIEW_PRIV_H_ */

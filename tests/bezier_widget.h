/*	Public domain	*/

typedef struct my_bezier_widget {
	struct ag_widget _inherit;      /* AG_Widget -> MY_BezierWidget */
	void *ti;                       /* Agar test instance (for agartest) */
	int label;                      /* Surface handle */
} MY_BezierWidget;

#define  MYBEZIERWIDGET(o)        ((MY_BezierWidget *)(o))
#define MYcBEZIERWIDGET(o)        ((const MY_BezierWidget *)(o))
#define  MYBEZIERWIDGET_SELF()    MYBEZIERWIDGET(  AG_OBJECT(0,         "AG_Widget:BezierWidget:*") )
#define  MYBEZIERWIDGET_PTR(n)    MYBEZIERWIDGET(  AG_OBJECT((n),       "AG_Widget:BezierWidget:*") )
#define  MYBEZIERWIDGET_NAMED(n)  MYBEZIERWIDGET(  AG_OBJECT_NAMED((n), "AG_Widget:BezierWidget:*") )
#define MYcBEZIERWIDGET_SELF()   MYcBEZIERWIDGET( AG_cOBJECT(0,         "AG_Widget:BezierWidget:*") )
#define MYcBEZIERWIDGET_PTR(n)   MYcBEZIERWIDGET( AG_cOBJECT((n),       "AG_Widget:BezierWidget:*") )
#define MYcBEZIERWIDGET_NAMED(n) MYcBEZIERWIDGET( AG_cOBJECT_NAMED((n), "AG_Widget:BezierWidget:*") )

extern AG_WidgetClass myBezierWidgetClass;

MY_BezierWidget *MY_BezierWidgetNew(void *, const char *);


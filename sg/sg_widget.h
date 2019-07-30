/*	Public domain	*/

enum sg_widget_style {
	SG_WIDGET_SQUARE,
	SG_WIDGET_DISC
};

typedef struct sg_widget {
	struct sg_geom _inherit;		/* SG_Geom -> SG_Widget */
	enum sg_widget_style style;
	Uint flags;
	M_Real size;
	AG_TAILQ_ENTRY(sg_widget) widgets;
} SG_Widget;

#define SGWIDGET(n) ((SG_Widget *)(n))

__BEGIN_DECLS
extern SG_NodeClass sgWidgetClass;

SG_Widget *_Nonnull SG_WidgetNew(void *_Nullable, enum sg_widget_style,
                                 const char *_Nullable);

#define    SG_WidgetLineWidth(ln,wd)	SG_GeomLineWidth(SGGEOM(ln),(wd))
#define    SG_WidgetLineColor(ln,c)	SG_GeomLineColor(SGGEOM(ln),(c))
#define    SG_WidgetLineStipple(ln,f,p)	SG_GeomLineStipple(SGGEOM(ln),(f),(p))
__END_DECLS

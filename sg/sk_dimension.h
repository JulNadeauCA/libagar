/*	Public domain	*/

#define SK_DIMENSION_TEXT_MAX	256

enum sk_dimension_type {
	SK_DIMENSION_DISTANCE,		/* Linear distance */
	SK_DIMENSION_ANGLE_ENDPOINT,	/* Angle at shared endpoint */
	SK_DIMENSION_ANGLE_INTERSECT	/* Angle at line intersection */
};

typedef struct sk_dimension {
	struct sk_annot annot;
	enum sk_dimension_type type;	/* Type of dimension */
	const AG_Unit *unit;		/* Length/angle unit (NULL = default) */
	SK_Node *n1, *n2;		/* Dimension between nodes */
	SG_Color cLbl;			/* Color of label text */
	SG_Color cLblBorder;		/* Color of label border */
	SG_Color cLineDim;		/* Color of dimension line */
	int xPad, yPad;			/* Label padding (pixels) */
	int lbl;			/* Label texture handle */
	SG_Vector vLbl;			/* Label position (in annotation's
					   frame of reference) */
	char text[SK_DIMENSION_TEXT_MAX]; /* Generated label text */
} SK_Dimension;

#define SKDIMENSION(n) ((SK_Dimension *)(n))

__BEGIN_DECLS
extern SK_NodeOps skDimensionOps;

SK_Dimension	*SK_DimensionNew(void *);
void		 SK_DimensionInit(void *, Uint32);
int		 SK_DimensionLoad(SK *, void *, AG_Netbuf *);
int		 SK_DimensionSave(SK *, void *, AG_Netbuf *);
void		 SK_DimensionDraw(void *, SK_View *);
void		 SK_DimensionRedraw(void *, SK_View *);
void		 SK_DimensionEdit(void *, AG_Widget *, SK_View *);
SG_Real		 SK_DimensionProximity(void *, const SG_Vector *, SG_Vector *);
int		 SK_DimensionDelete(void *);
int		 SK_DimensionMove(void *, const SG_Vector *, const SG_Vector *);
void		 SK_DimensionSetUnit(SK_Dimension *, const AG_Unit *);
__END_DECLS

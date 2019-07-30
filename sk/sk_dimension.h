/*	Public domain	*/

#ifndef SK_DIMENSION_TEXT_MAX
#define SK_DIMENSION_TEXT_MAX 56
#endif

enum sk_dimension_type {
	SK_DIMENSION_NONE,
	SK_DIMENSION_DISTANCE,		/* Linear distance */
	SK_DIMENSION_ANGLE_ENDPOINT,	/* Angle at shared endpoint */
	SK_DIMENSION_ANGLE_INTERSECT	/* Angle at line intersection */
};

typedef struct sk_dimension {
	struct sk_annot annot;
	enum sk_dimension_type type;		/* Type of dimension */
	Uint flags;
#define SK_DIMENSION_CONJ_ANGLE	0x01		/* Display conjugate angle */
	const struct ag_unit *_Nullable unit;	/* Length/angle unit (NULL = default) */
	SK_Node *_Nullable n1;			/* Node #1 */
	SK_Node *_Nullable n2;			/* Node #2 */
	M_Color cLbl;				/* Color of label text */
	M_Color cLblBorder;			/* Color of label border */
	M_Color cLineDim;			/* Color of dimension line */
	int xPad, yPad;				/* Label padding (pixels) */
	char text[SK_DIMENSION_TEXT_MAX];	/* Generated label text */
	M_Vector3 vLbl;				/* Label position (in annotation's
						   frame of reference) */
	M_Real wLbl, hLbl;			/* Label dimensions */
} SK_Dimension;

typedef struct sk_dimension_view {
	int lbl;			/* Label texture handle */
} SK_DimensionView;

#define SKDIMENSION(n) ((SK_Dimension *)(n))

__BEGIN_DECLS
extern SK_NodeOps skDimensionOps;

SK_Dimension *_Nonnull SK_DimensionNew(void *_Nonnull);
void SK_DimensionInit(void *_Nonnull, Uint);
int  SK_DimensionLoad(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
int  SK_DimensionSave(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
void SK_DimensionDraw(void *_Nonnull, struct sk_view *_Nonnull);
void SK_DimensionRedraw(void *_Nonnull, struct sk_view *_Nonnull);
void SK_DimensionEdit(void *_Nonnull, struct ag_widget *_Nonnull,
                      struct sk_view *_Nonnull);
M_Real SK_DimensionProximity(void *_Nonnull, const M_Vector3 *_Nonnull,
                             M_Vector3 *_Nonnull);

int SK_DimensionDelete(void *_Nonnull);
int SK_DimensionMove(void *_Nonnull, const M_Vector3 *_Nonnull,
                     const M_Vector3 *_Nonnull);
void SK_DimensionSetUnit(SK_Dimension *_Nonnull, const struct ag_unit *_Nonnull);
__END_DECLS

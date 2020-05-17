/*	Public domain	*/

typedef struct sk_arc {
	struct sk_node node;
	SK_Point *_Nullable p;		/* Center point of circle */
	SK_Point *_Nullable e1;		/* Endpoint #1 */
	SK_Point *_Nullable e2;		/* Endpoint #2 */
	M_Real  r;			/* Radius of circle */
	M_Color color;			/* Display color */
} SK_Arc;

#define SKARC(n) ((SK_Arc *)(n))

__BEGIN_DECLS
extern SK_NodeOps skArcOps;

SK_Arc *_Nonnull SK_ArcNew(void *_Nonnull);

void   SK_ArcInit(void *_Nonnull, Uint);
int    SK_ArcLoad(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
int    SK_ArcSave(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
void   SK_ArcDraw(void *_Nonnull, struct sk_view *_Nonnull);
M_Real SK_ArcProximity(void *_Nonnull, const M_Vector3 *_Nonnull,
                       M_Vector3 *_Nonnull);

void   SK_ArcWidth(SK_Arc *_Nonnull, M_Real);
void   SK_ArcColor(SK_Arc *_Nonnull, M_Color);
__END_DECLS

/*	Public domain	*/

typedef struct sk_point {
	struct sk_node node;
	Uint flags;
	Uint32 _pad;
	M_Real size;			/* Display size in pixels */
	M_Color color;			/* Display color */
} SK_Point;

#define SKPOINT(n) ((SK_Point *)(n))

__BEGIN_DECLS
extern SK_NodeOps skPointOps;

SK_Point *_Nonnull SK_PointNew(void *_Nonnull);

void      SK_PointInit(void *_Nonnull, Uint);
int       SK_PointLoad(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
int       SK_PointSave(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
void      SK_PointDraw(void *_Nonnull, struct sk_view *_Nonnull);
void      SK_PointEdit(void *_Nonnull, struct ag_widget *_Nonnull,
                       struct sk_view *_Nonnull);
M_Real    SK_PointProximity(void *_Nonnull, const M_Vector3 *_Nonnull,
                            M_Vector3 *_Nonnull);
int       SK_PointDelete(void *_Nonnull);
int       SK_PointMove(void *_Nonnull, const M_Vector3 *_Nonnull,
                       const M_Vector3 *_Nonnull);
SK_Status SK_PointConstrained(void *_Nonnull);

void      SK_PointSize(SK_Point *_Nonnull, M_Real);
void      SK_PointColor(SK_Point *_Nonnull, M_Color);
__END_DECLS

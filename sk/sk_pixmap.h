/*	Public domain	*/

typedef struct sk_pixmap {
	struct sk_node node;
	SK_Point *_Nullable p;		/* Center point */
	M_Real w, h;			/* Dimensions */
	M_Real alpha;			/* Overall alpha */
	int s;				/* Mapped surface */
	Uint32 _pad;
	AG_Surface *_Nullable sSrc;	/* Source surface */
} SK_Pixmap;

#define SK_PIXMAP(n) ((SK_Pixmap *)(n))

__BEGIN_DECLS
extern SK_NodeOps skPixmapOps;

SK_Pixmap *_Nonnull SK_PixmapNew(void *_Nonnull, SK_Point *_Nullable);

void	   SK_PixmapInit(void *_Nonnull, Uint);
void	   SK_PixmapDestroy(void *_Nonnull);
int	   SK_PixmapLoad(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
int	   SK_PixmapSave(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
void	   SK_PixmapDraw(void *_Nonnull, struct sk_view *_Nonnull);
void	   SK_PixmapEdit(void *_Nonnull, struct ag_widget *_Nonnull,
                         struct sk_view *_Nonnull);
M_Real	   SK_PixmapProximity(void *_Nonnull, const M_Vector3 *_Nonnull,
                              M_Vector3 *_Nonnull);
int	   SK_PixmapDelete(void *_Nonnull);
int	   SK_PixmapMove(void *_Nonnull, const M_Vector3 *_Nonnull,
                         const M_Vector3 *_Nonnull);
SK_Status  SK_PixmapConstrained(void *_Nonnull);

void	   SK_PixmapDimensions(SK_Pixmap *_Nonnull, M_Real, M_Real);
void	   SK_PixmapAlpha(SK_Pixmap *_Nonnull, M_Real);
void	   SK_PixmapSurface(SK_Pixmap *_Nonnull, AG_Surface *_Nullable);
__END_DECLS

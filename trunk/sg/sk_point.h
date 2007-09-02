/*	Public domain	*/

typedef struct sk_point {
	struct sk_node node;
	Uint flags;
	SG_Real size;			/* Display size in pixels */
	SG_Color color;			/* Display color */
} SK_Point;

#define SK_POINT(n) ((SK_Point *)(n))

__BEGIN_DECLS
extern SK_NodeOps skPointOps;

SK_Point	*SK_PointNew(void *);
void		 SK_PointInit(void *, Uint32);
int		 SK_PointLoad(SK *, void *, AG_Netbuf *);
int		 SK_PointSave(SK *, void *, AG_Netbuf *);
void		 SK_PointDraw(void *, SK_View *);
void		 SK_PointDrawAbsolute(void *, SK_View *);
void		 SK_PointEdit(void *, AG_Widget *, SK_View *);
SG_Real		 SK_PointProximity(void *, const SG_Vector *, SG_Vector *);
int		 SK_PointDelete(void *);
int		 SK_PointMove(void *, const SG_Vector *, const SG_Vector *);
SK_Status	 SK_PointConstrained(void *);

void		 SK_PointSize(SK_Point *, SG_Real);
void		 SK_PointColor(SK_Point *, SG_Color);
__END_DECLS

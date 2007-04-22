/*	$Csoft$	*/
/*	Public domain	*/

typedef struct sk_point {
	struct sk_node node;
	SG_Vector pos;			/* Coordinates of point */
	SG_Real size;			/* Display size in pixels */
	SG_Color color;			/* Display color */
} SK_Point;

__BEGIN_DECLS
extern SK_NodeOps skPointOps;

SK_Point	*SK_PointNew(void *);
void		 SK_PointInit(void *);
int		 SK_PointLoad(void *, AG_Netbuf *);
int		 SK_PointSave(void *, AG_Netbuf *);
void		 SK_PointDraw(void *, SK_View *);

void		 SK_PointSize(SK_Point *, SG_Real);
void		 SK_PointColor(SK_Point *, SG_Color);
__END_DECLS

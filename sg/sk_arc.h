/*	Public domain	*/

typedef struct sk_arc {
	struct sk_node node;
	SK_Point *p;			/* Center point of circle */
	SK_Point *e1, *e2;		/* Endpoints (constrained) */
	SG_Real   r;			/* Radius of circle */
	SG_Color  color;		/* Display color */
} SK_Arc;

__BEGIN_DECLS
extern SK_NodeOps skArcOps;

SK_Arc	*SK_ArcNew(void *);
void	 SK_ArcInit(void *, Uint32);
int	 SK_ArcLoad(SK *, void *, AG_Netbuf *);
int	 SK_ArcSave(SK *, void *, AG_Netbuf *);
void	 SK_ArcDraw(void *, SK_View *);
void	 SK_ArcEdit(void *, AG_Widget *, SK_View *);
SG_Real	 SK_ArcProximity(void *, const SG_Vector *, SG_Vector *);

void	 SK_ArcWidth(SK_Arc *, SG_Real);
void	 SK_ArcColor(SK_Arc *, SG_Color);
__END_DECLS

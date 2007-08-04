/*	Public domain	*/

typedef struct sk_circle {
	struct sk_node node;
	SK_Point *p;			/* Center point */
	SG_Real   r;			/* Radius */
	SG_Real	  width;		/* Display thickness */
	SG_Color  color;		/* Display color */
} SK_Circle;

#define SK_CIRCLE(n) ((SK_Circle *)(n))

__BEGIN_DECLS
extern SK_NodeOps skCircleOps;

SK_Circle *SK_CircleNew(void *);
void	   SK_CircleInit(void *, Uint32);
int	   SK_CircleLoad(SK *, void *, AG_Netbuf *);
int	   SK_CircleSave(SK *, void *, AG_Netbuf *);
void	   SK_CircleDraw(void *, SK_View *);
void	   SK_CircleEdit(void *, AG_Widget *, SK_View *);
SG_Real	   SK_CircleProximity(void *, const SG_Vector *, SG_Vector *);
int	   SK_CircleDelete(void *);
void	   SK_CircleMove(void *, const SG_Vector *, const SG_Vector *);

void	   SK_CircleWidth(SK_Circle *, SG_Real);
void	   SK_CircleColor(SK_Circle *, SG_Color);
__END_DECLS

/*	$Csoft$	*/
/*	Public domain	*/

typedef struct sk_circle {
	struct sk_node node;
	SK_Point *p;			/* Center point */
	SG_Real   r;			/* Radius */
	SG_Real	  width;		/* Display thickness */
	SG_Color  color;		/* Display color */
} SK_Circle;

__BEGIN_DECLS
extern SK_NodeOps skCircleOps;

SK_Circle *SK_CircleNew(void *);
void	   SK_CircleInit(void *);
int	   SK_CircleLoad(void *, AG_Netbuf *);
int	   SK_CircleSave(void *, AG_Netbuf *);
void	   SK_CircleDraw(void *, SK_View *);

void	   SK_CircleWidth(SK_Circle *, SG_Real);
void	   SK_CircleColor(SK_Circle *, SG_Color);
__END_DECLS

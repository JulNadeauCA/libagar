/*	$Csoft$	*/
/*	Public domain	*/

typedef struct sk_line {
	struct sk_node node;
	SK_Point *p1, *p2;		/* Endpoints */
	SG_Real width;			/* Display width */
	SG_Color color;			/* Display color */
} SK_Line;

__BEGIN_DECLS
extern SK_NodeOps skLineOps;

SK_Line	*SK_LineNew(void *);
void	 SK_LineInit(void *);
int	 SK_LineLoad(void *, AG_Netbuf *);
int	 SK_LineSave(void *, AG_Netbuf *);
void	 SK_LineDraw(void *, SK_View *);

void	 SK_LineWidth(SK_Line *, SG_Real);
void	 SK_LineColor(SK_Line *, SG_Color);
__END_DECLS

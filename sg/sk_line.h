/*	Public domain	*/

typedef struct sk_line {
	struct sk_node node;
	SK_Point *p1, *p2;		/* Endpoints */
	SG_Real width;			/* Display width */
	SG_Color color;			/* Display color */
} SK_Line;

#define SKLINE(n) ((SK_Line *)(n))

__BEGIN_DECLS
extern SK_NodeOps skLineOps;

SK_Line	*SK_LineNew(void *);
void	 SK_LineInit(void *, Uint32);
int	 SK_LineLoad(SK *, void *, AG_Netbuf *);
int	 SK_LineSave(SK *, void *, AG_Netbuf *);
void	 SK_LineDraw(void *, SK_View *);
void	 SK_LineEdit(void *, AG_Widget *, SK_View *);
SG_Real	 SK_LineProximity(void *, const SG_Vector *, SG_Vector *);
int	 SK_LineDelete(void *);
void	 SK_LineMove(void *, const SG_Vector *, const SG_Vector *);

void	 SK_LineWidth(SK_Line *, SG_Real);
void	 SK_LineColor(SK_Line *, SG_Color);
__END_DECLS

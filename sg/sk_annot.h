/*	Public domain	*/

typedef struct sk_annot {
	struct sk_node node;
	Uint flags;
} SK_Annot;

#define SK_ANNOT(n) ((SK_Annot *)(n))

__BEGIN_DECLS
extern SK_NodeOps skAnnotOps;

void		 SK_AnnotInit(void *, Uint32, const SK_NodeOps *);
int		 SK_AnnotLoad(SK *, void *, AG_Netbuf *);
int		 SK_AnnotSave(SK *, void *, AG_Netbuf *);
void		 SK_AnnotDraw(void *, SK_View *);
int		 SK_AnnotDelete(void *);
__END_DECLS


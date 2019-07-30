/*	Public domain	*/

typedef struct sk_annot {
	struct sk_node node;
	Uint flags;
	Uint8 _pad[12];
} SK_Annot;

#define SK_ANNOT(n) ((SK_Annot *)(n))

__BEGIN_DECLS
extern SK_NodeOps skAnnotOps;

void SK_AnnotInit(void *_Nonnull, Uint, const SK_NodeOps *_Nonnull);
int  SK_AnnotLoad(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
int  SK_AnnotSave(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
int  SK_AnnotDelete(void *_Nonnull);
__END_DECLS


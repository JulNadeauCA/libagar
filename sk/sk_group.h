/*	Public domain	*/

typedef struct sk_group {
	struct sk_node _inherit;
	SK_Node *_Nullable *_Nullable nodes;	/* Nodes in group */
	Uint                         nNodes;
	Uint32 _pad;
	M_Color color;				/* Selection color */
} SK_Group;

#define SKGROUP(n) ((SK_Group *)(n))

__BEGIN_DECLS
extern SK_NodeOps skGroupOps;

SK_Group *_Nonnull SK_GroupNew(void *_Nonnull);

void SK_GroupInit(void *_Nonnull, Uint);
int  SK_GroupLoad(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
int  SK_GroupSave(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
void SK_GroupEdit(void *_Nonnull, struct ag_widget *_Nonnull,
                  struct sk_view *_Nonnull);
void SK_GroupColor(SK_Group *_Nonnull, M_Color);
__END_DECLS

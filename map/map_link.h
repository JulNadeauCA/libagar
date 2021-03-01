/*	Public domain	*/

#define MAP_LINK_ID_MAX 512

/* Reference to another map location. */
typedef struct map_link {
	struct map_item _inherit;       /* MAP_Item -> MAP_Link */
	char *_Nullable map;		/* Target map id */
	int x, y;			/* At coordinates */
	Uint dir;			/* Towards direction */
	Uint32 _pad;
} MAP_Link;

#define MAPLINK(mi) ((MAP_Link *)(mi))

__BEGIN_DECLS
extern MAP_ItemClass mapLinkClass;

MAP_Link *_Nonnull MAP_LinkNew(MAP *_Nonnull, MAP_Node *_Nonnull,
                               const char *_Nonnull, int,int, Uint);
__END_DECLS

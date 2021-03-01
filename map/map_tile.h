/*	Public domain	*/

/* Reference to a RG_Tile(3) */
typedef struct map_tile {
	struct map_item _inherit;       /* MAP_Item -> MAP_Tile */
	RG_Tileset *_Nonnull obj;       /* Source tileset */
	Uint id;                        /* Tile ID */
	int xCenter, yCenter;           /* Centering offsets */
	int xMotion, yMotion;           /* Motion offsets */
	AG_Rect rs;                     /* Source rectangle */
	Uint32 _pad;
} MAP_Tile;

#define MAPTILE(mi) ((MAP_Tile *)(mi))

__BEGIN_DECLS
extern MAP_ItemClass mapTileClass;

MAP_Tile *_Nonnull MAP_TileNew(MAP *_Nonnull, MAP_Node *_Nonnull,
                               RG_Tileset *_Nonnull, Uint);

void MAP_TileSet(MAP_Tile *_Nonnull, MAP *_Nonnull,
                 RG_Tileset *_Nullable, Uint);
__END_DECLS

/*	Public domain	*/

/* Reference to a fragment of an image file. */
typedef struct map_img {
	struct map_item _inherit;       /* MAP_Item -> MAP_Img */
	Uint idx;                       /* Index in map's imageFiles[] */
	int xCenter, yCenter;           /* Centering offsets */
	int xMotion, yMotion;           /* Motion offsets */
	AG_Rect rs;                     /* Source rectangle */
	Uint32 _pad;
} MAP_Img;

#define MAPIMG(mi) ((MAP_Img *)(mi))

__BEGIN_DECLS
extern MAP_ItemClass mapImgClass;

MAP_Img *_Nonnull MAP_ImgNew(MAP *_Nonnull, MAP_Node *_Nonnull, Uint);
__END_DECLS

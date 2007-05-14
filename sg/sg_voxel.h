/*	$Csoft$	*/
/*	Public domain	*/

typedef struct sg_voxel {
	SG_Node node;
	int w, h, d;
	SG_Real ***map;			/* Array of cells */
} SG_Voxel;

__BEGIN_DECLS
extern SG_NodeOps sgVoxelOps;

SG_Voxel *SG_VoxelNew(void *, const char *);
void SG_VoxelInit(void *, const char *);
void SG_VoxelReinit(void *);
void SG_VoxelDestroy(void *);
void SG_VoxelDraw(void *, SG_View *);

void SG_VoxelAlloc3(SG_Voxel *, Uint, Uint, Uint);
__inline__ void SG_VoxelReset(SG_Voxel *, SG_Real);
__inline__ void SG_VoxelSet3(SG_Voxel *, Uint, Uint, Uint, SG_Real);
__END_DECLS

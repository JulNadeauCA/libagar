/*	Public domain	*/

typedef struct sg_bsp_node {
	M_Plane P;		/* Partitioning plane */
	SG_Facet *polys;	/* Polygons */
	Uint	 npolys;
	SG_BSPNode *nFront;
	SG_BSPNode *nBack;
} SG_BSPNode;

typedef struct sg_bsp_tree {
	SG_BSPNode *root;
} SG_BSPTree;

__BEGIN_DECLS
void SG_BSPInit(SG_BSPTree *);
void SG_BSPCompile(SG_BSPTree *, );
__END_DECLS

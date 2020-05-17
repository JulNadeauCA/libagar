/*	Public domain	*/

typedef struct sg_blk {
	int m;				/* Material index */
	float T;			/* Temperature (degC) */
	Uint n;				/* Divisions */
	struct sg_blk *c;		/* Sub-cells */
} SG_Blk;

typedef struct sg_map {
	struct sg_node _inherit;	/* SG_Node -> SG_Map */
	Uint flags;
	SG_Blk *root;			/* Root block */
} SG_Map;

#define SGMAP(n) ((SG_Map *)(n))

__BEGIN_DECLS
extern SG_NodeClass sgMapClass;

/* Retrieve block cell by integer coordinates. */
static __inline__ SG_Blk *
SG_BlkGet(SG_Blk *b, int x, int y, int z)
{
	int idx;

	idx = z*(b->n*b->n) + y*(b->n) + x;
#ifdef AG_DEBUG
	if (idx >= b->n)
		AG_FatalError("SG_BlkGet: Bad coords %d,%d,%d", x,y,z);
#endif
	return (&b->c[idx]);
}

SG_Map *SG_MapNew(void *, const char *);
int     SG_MapDivide(SG_Blk *, Uint);
__END_DECLS

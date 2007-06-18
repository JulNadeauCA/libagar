/*	Public domain	*/

typedef struct sg_plane_obj {
	struct sg_node node;
	SG_Plane P;
} SG_PlaneObj;

__BEGIN_DECLS
extern SG_NodeOps sgPlaneObjOps;

SG_PlaneObj	*SG_PlaneObjNew(void *, const char *, SG_Vector, SG_Real);
SG_PlaneObj	*SG_PlaneObjNewPts(void *, const char *, SG_Vector, SG_Vector,
		                   SG_Vector);
void		 SG_PlaneObjInit(void *, const char *);
int		 SG_PlaneObjLoad(void *, AG_Netbuf *);
int		 SG_PlaneObjSave(void *, AG_Netbuf *);
__END_DECLS

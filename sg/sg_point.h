/*	Public domain	*/

typedef struct sg_point {
	struct sg_node node;
	float size;
	SG_Color color;
} SG_Point;

__BEGIN_DECLS
extern SG_NodeOps sgPointOps;

SG_Point *SG_PointNew(void *, const char *);
void	  SG_PointInit(void *, const char *);
int	  SG_PointLoad(void *, AG_Netbuf *);
int	  SG_PointSave(void *, AG_Netbuf *);
void	 *SG_PointEdit(void *);
void	  SG_PointDraw(void *, SG_View *);
void	  SG_PointSize(void *, SG_Real);
void	  SG_PointColor(void *, SG_Color);
__END_DECLS

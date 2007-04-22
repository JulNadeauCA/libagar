/*	$Csoft$	*/
/*	Public domain	*/

typedef struct sg_solid {
	struct sg_object obj;
	SG_Vector vLin;			/* Linear velocity vector */
	SG_Vector vAng;			/* Angular velocity vector */
	SG_Real rho;			/* Density */
	SG_Matrix J;			/* Inertia tensor */
	SG_Vector r;			/* Center of mass */
	SG_Real m;			/* Total mass */
} SG_Solid;

__BEGIN_DECLS
extern SG_NodeOps sgSolidOps;

SG_Solid *SG_SolidNew(void *, const char *);
void	  SG_SolidInit(void *, const char *);
void	  SG_SolidDestroy(void *);
int	  SG_SolidLoad(void *, AG_Netbuf *);
int	  SG_SolidSave(void *, AG_Netbuf *);
#define   SG_SolidDraw SG_ObjectDraw
void	  SG_SolidBox(SG_Solid *, SG_Real, SG_Real, SG_Real);
__END_DECLS

/*	Public domain	*/

typedef struct sg_sphere {
	struct sg_object obj;
	int tesslvl;		/* Tesselation level */
} SG_Sphere;

__BEGIN_DECLS
extern SG_NodeOps sgSphereOps;
SG_Sphere *SG_SphereNew(void *, const char *);
void SG_SphereInit(void *, const char *);
int SG_SphereLoad(void *, AG_DataSource *);
int SG_SphereSave(void *, AG_DataSource *);
void SG_SphereEdit(void *, AG_Widget *, SG_View *);
void SG_SphereGen(SG_Sphere *);
void SG_SphereDraw(void *, SG_View *);
__END_DECLS


/*	Public domain	*/

typedef struct sg_light {
	struct sg_node _inherit;	/* SG_Node -> SG_Light */
	Uint flags;
	int pri;			/* Priority for light allocation */
	int light;			/* Allocated OpenGL light name
					   (or GL_INVALID_ENUM) */
	int tag;			/* User tag */
	M_Color ambient;		/* Ambient component */
	M_Color diffuse;		/* Diffuse component */
	M_Color specular;		/* Specular component */
	M_Vector3 spot_dir;		/* Spot direction vector */
	M_Real spot_exponent;		/* Intensity distribution */
	M_Real spot_cutoff;		/* Spot cutoff angle */
	M_Real Kc;			/* Constant attenuation factor */
	M_Real Kl;			/* Linear attenuation factor */
	M_Real Kq;			/* Quadratic attenuation factor */
#ifdef HAVE_GLU
	GLUquadricObj *_Nonnull qo;	/* For rendering in editor */
#endif
	Uint8 _pad[8];
} SG_Light;

__BEGIN_DECLS
extern SG_NodeClass sgLightClass;

SG_Light *_Nonnull SG_LightNew(void *_Nullable, const char *_Nullable);
void               SG_LightSetup(SG_Light *_Nonnull);

static __inline__ void
SG_LightAmbient(SG_Light *_Nonnull lt, M_Color c)
{
	lt->ambient = c;
}
static __inline__ void
SG_LightDiffuse(SG_Light *_Nonnull lt, M_Color c)
{
	lt->diffuse = c;
}
static __inline__ void
SG_LightSpecular(SG_Light *_Nonnull lt, M_Color c)
{
	lt->specular = c;
}
static __inline__ void
SG_LightSpot(SG_Light *_Nonnull lt, M_Real cutoff, M_Real exponent,
    M_Vector3 dir)
{
	lt->spot_cutoff = cutoff;
	lt->spot_exponent = exponent;
	lt->spot_dir = dir;
}
__END_DECLS

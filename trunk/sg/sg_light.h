/*	$Csoft$	*/
/*	Public domain	*/

typedef struct sg_light {
	struct sg_node node;
	int pri;			/* Priority for light allocation */
	int light;			/* Allocated OpenGL light name
					   (or GL_INVALID_ENUM) */
	SG_Color ambient;		/* Ambient component */
	SG_Color diffuse;		/* Diffuse component */
	SG_Color specular;		/* Specular component */
	SG_Vector spot_dir;		/* Spot direction vector */
	SG_Real spot_exponent;		/* Intensity distribution */
	SG_Real spot_cutoff;		/* Spot cutoff angle */
	SG_Real Kc;			/* Constant attenuation factor */
	SG_Real Kl;			/* Linear attenuation factor */
	SG_Real Kq;			/* Quadratic attenuation factor */
	void *cyl;			/* Quadric object for rendering */
} SG_Light;

__BEGIN_DECLS
extern SG_NodeOps sgLightOps;

SG_Color	 SG_ColorRGB(SG_Real, SG_Real, SG_Real);
SG_Color	 SG_ColorRGBA(SG_Real, SG_Real, SG_Real, SG_Real);
void		 SG_ColorTo4fv(const SG_Color *, float *);
void		 SG_ColorTo4dv(const SG_Color *, double *);

SG_Color	 SG_ReadColor(AG_Netbuf *);
void		 SG_WriteColor(AG_Netbuf *, SG_Color *);

SG_Light	*SG_LightNew(void *, const char *);
void		 SG_LightInit(void *, const char *);
void		 SG_LightDestroy(void *);
int		 SG_LightLoad(void *, AG_Netbuf *);
int		 SG_LightSave(void *, AG_Netbuf *);
__inline__ void	 SG_LightSetup(SG_Light *);
void		 SG_LightAlloc(SG_Light *, int);
void		 SG_LightDraw(void *, SG_View *);
void		 SG_LightEdit(void *, AG_Widget *, SG_View *);
void		 SG_LightMenu(void *, AG_MenuItem *, SG_View *);
__END_DECLS

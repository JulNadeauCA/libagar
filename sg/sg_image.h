/*	Public domain	*/

/* Contour shape */
enum sg_image_shape {
	SG_IMAGE_RECT,			/* Opaque rectangle */
	SG_IMAGE_POLY			/* Complex polygon */
};

typedef struct sg_image {
	struct sg_node _inherit;		/* SG_Node -> SG_Image */
	enum sg_image_shape shape;		/* Contour shape */
	Uint flags;
#define SG_IMAGE_BILLBOARD	0x01		/* Render facing the camera */
#define SG_IMAGE_NODUP		0x02		/* Surface is not a copy */
#define SG_IMAGE_NODUP_ANIM	0x04		/* Animation is not a copy */
#define SG_IMAGE_WIREFRAME	0x08		/* Overlay wireframe */
#define SG_IMAGE_HOLES		0x10		/* Enable hole detection */
#define SG_IMAGE_SAVE_SURFACE	0x20		/* Store surface in save */
#define SG_IMAGE_NOAUTOSIZE	0x40		/* Don't scale proportionally
                                                   to loaded surface size. */
#define SG_IMAGE_SAVED	(SG_IMAGE_BILLBOARD|SG_IMAGE_WIREFRAME|\
			 SG_IMAGE_HOLES|SG_IMAGE_SAVE_SURFACE)

	char path[AG_FILENAME_MAX];		/* Path for loading images
						   or animations */

	AG_Color color;				/* Background color */
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad;
#endif
	AG_Surface *_Nullable su;		/* Source surface
						   (Anim: Effective surface) */
	AG_TexCoord *_Nullable tc;		/* Effective texcoords */
	AG_Surface *_Nullable an;		/* Source animation */
	AG_AnimState ast;			/* Playback instance */
	M_Real w, h;				/* Dimensions */
	M_Real wOrig, hOrig;			/* Dimensions (for scale) */
	void *_Nullable to;			    /* GLU tesselator object */
	SG_GLVertex3 *_Nullable *_Nonnull tessBufs; /* Tesselator buffers */
	Uint                             nTessBufs;
	float tolContour, tolHoles;		/* For simplification */
	Uint32 _pad1;
	M_Polygon contour;			/* Computed polygon contour */
	M_Polygon *_Nullable holes;		/* Computed holes */
	Uint                nHoles;
	Uint32 _pad2;
	AG_TAILQ_HEAD_(sg_view_texture) vtex;	/* Per-view textures */
	AG_TAILQ_HEAD_(sg_view_list) vlist;	/* Per-view display lists */
	Uint8 _pad3[8];
} SG_Image;

#define SGIMAGE(n) ((SG_Image *)(n))

__BEGIN_DECLS
extern SG_NodeClass sgImageClass;

SG_Image *_Nonnull  SG_ImageNew(void *_Nullable, const char *_Nonnull);
SG_Image *_Nullable SG_ImageFromSurface(void *_Nullable, const char *_Nonnull,
                                        const AG_Surface *_Nonnull, M_Real);
SG_Image *_Nullable SG_ImageFromSurfaceNODUP(void *_Nullable, const char *_Nonnull,
                                             AG_Surface *_Nonnull, M_Real);

void SG_ImageFreeCached(void *_Nonnull);
void SG_ImageFreeSurfaces(void *_Nonnull);
int  SG_ImageSetSurface(void *_Nonnull, const AG_Surface *_Nonnull, M_Real);
int  SG_ImageSetSurfaceNODUP(void *_Nonnull, AG_Surface *_Nonnull, M_Real);
int  SG_ImageSetAnim(void *_Nonnull, const AG_Surface *_Nonnull, M_Real);
int  SG_ImageSetAnimNODUP(void *_Nonnull, AG_Surface *_Nonnull, M_Real);
void SG_ImageSetShape(void *_Nonnull, enum sg_image_shape);
void SG_ImageSetShapeAuto(void *_Nonnull);
void SG_ImageSetProj(void *_Nonnull, int);
void SG_ImageAnimPlay(void *_Nonnull);
void SG_ImageAnimStop(void *_Nonnull);

/* Set the tolerance for polygon simplification. */
static __inline__ void
SG_ImageSetTolContour(void *_Nonnull obj, float tol)
{
	SG_Image *img = obj;

	AG_ObjectLock(img);
	img->tolContour = tol;
	AG_ObjectUnlock(img);
}

/* Change the image size. */
static __inline__ void
SG_ImageSetSize(void *_Nonnull obj, M_Real w, M_Real h)
{
	SG_Image *img = obj;

	AG_ObjectLock(img);
	img->w = w;
	img->h = h;
	AG_ObjectUnlock(img);
}

/* Set the image background color. */
static __inline__ void
SG_ImageSetColor(void *_Nonnull obj, const AG_Color *_Nonnull c)
{
	SG_Image *img = obj;

	AG_ObjectLock(img);
	img->color = *c;
	AG_ObjectUnlock(img);
}
__END_DECLS

/*	$Csoft$	*/
/*	Public domain	*/

struct sg_camera_polymode {
	enum {
		SG_CAMERA_POINTS,
		SG_CAMERA_WIREFRAME,
		SG_CAMERA_FLAT_SHADED,
		SG_CAMERA_SMOOTH_SHADED
	} mode;
	int cull;
};

typedef struct sg_camera {
	struct sg_node node;
	Uint flags;
#define SG_CAMERA_ROT_I 0x01	/* Animate rotation around i (debug) */
#define SG_CAMERA_ROT_J 0x02	/* Animate rotation around j (debug) */
#define SG_CAMERA_ROT_K 0x04	/* Animate rotation around k (debug) */

	enum sg_camera_pmode {
		SG_CAMERA_PERSPECTIVE,		/* Perspective projection */
		SG_CAMERA_ORTHOGRAPHIC,		/* Parallel projection */
		SG_CAMERA_USER_PROJ		/* User-specified */
	} pmode;
	struct sg_camera_polymode polyFace;	/* Front-facing polygon modes */
	struct sg_camera_polymode polyBack;	/* Back-facing polygon modes */
	SG_Real fovY;				/* Field of view (degrees) */
	SG_Real aspect;				/* Aspect ratio */
	SG_Real zNear, zFar;			/* Clipping planes */
	SG_Matrix userProj;			/* User projection matrix */
#ifdef DEBUG
	SG_Real rotSpeed;
#endif
} SG_Camera;

__BEGIN_DECLS
extern SG_NodeOps sgCameraOps;

SG_Camera	*SG_CameraNew(void *, const char *);
void		 SG_CameraInit(void *, const char *);
int		 SG_CameraLoad(void *, AG_Netbuf *);
int		 SG_CameraSave(void *, AG_Netbuf *);
void		 SG_CameraEdit(void *, AG_Widget *, SG_View *);
void		 SG_CameraMenu(void *, AG_MenuItem *, SG_View *);

void	 	 SG_CameraProject(SG_Camera *);
void		 SG_CameraGetProjection(SG_Camera *, SG_Matrix *);
__inline__ void	 SG_CameraSetPerspective(SG_Camera *, SG_Real, SG_Real);
__inline__ void	 SG_CameraSetOrthographic(SG_Camera *);
__inline__ void	 SG_CameraSetUser(SG_Camera *, const SG_Matrix *);
__inline__ void	 SG_CameraSetClipPlanes(SG_Camera *, SG_Real, SG_Real);
__inline__ void	 SG_CameraSetup(SG_Camera *);
SG_Vector	 SG_CameraVector(SG_Camera *);
__END_DECLS

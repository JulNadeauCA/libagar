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

typedef struct sg_camera_insn {
	enum {
		SG_CAMERA_CIRCULAR,	/* Circular path around node */
		SG_CAMERA_ELLIPTIC,	/* Elliptic path around two nodes */
		SG_CAMERA_FOLLOW,	/* Look at object and follow */
	} type;
	union {
		struct {
			SG_Node   *ci_center;	/* Center node */
			SG_Vector  ci_axis[2];	/* Axis of rotation */
			SG_Real    ci_angle[2];	/* Rotation increment */
		} circular;
		struct {
			SG_Node   *ci_foci[2];	/* Focus nodes */
			SG_Vector  ci_axis[2];	/* Axis of rotation */
			SG_Real    ci_angle[2];	/* Rotation increment */
		} elliptic;
		struct {
			SG_Node   *ci_node;	/* Node to follow */
			SG_Real    ci_dMin;	/* Minimum distance */
			SG_Real    ci_dMax;	/* Maximum distance */
		} follow;
	} data;
#ifdef _AGAR_INTERNAL
#define ci_circular_center	data.circular.ci_center
#define ci_circular_axis	data.circular.ci_axis
#define ci_circular_angle	data.circular.ci_angle
#define ci_elliptic_foci	data.elliptic.ci_foci
#define ci_elliptic_axis	data.elliptic.ci_axis
#define ci_elliptic_angle	data.elliptic.ci_angle
#define ci_follow_node		data.follow.ci_node
#define ci_follow_dMin		data.follow.ci_dMin
#define ci_follow_dMax		data.follow.ci_dMax
#endif
} SG_CameraInsn;

typedef struct sg_camera {
	struct sg_node node;
	Uint flags;
#define SG_CAMERA_ROT_I 0x01	/* Artificial rotation around i (debug) */
#define SG_CAMERA_ROT_J 0x02	/* Artificial rotation around j (debug) */
#define SG_CAMERA_ROT_K 0x04	/* Artificial rotation around k (debug) */

	enum sg_camera_pmode {
		SG_CAMERA_PERSPECTIVE,		/* Perspective projection */
		SG_CAMERA_ORTHOGRAPHIC,		/* Parallel projection */
		SG_CAMERA_USER_PROJ		/* User-specified */
	} pmode;
	struct sg_camera_polymode polyFace;	/* Front-facing polygon modes */
	struct sg_camera_polymode polyBack;	/* Back-facing polygon modes */
	SG_Real fovY;				/* Field of view (radians) */
	SG_Real aspect;				/* Aspect ratio */
	SG_Real zNear, zFar;			/* Clipping planes */
	SG_Matrix userProj;			/* User projection matrix
						   (column-major) */
#ifdef DEBUG
	SG_Real rotSpeed;
#endif
	SG_CameraInsn insn;
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
void		 SG_CameraDraw(void *, SG_View *);
__END_DECLS

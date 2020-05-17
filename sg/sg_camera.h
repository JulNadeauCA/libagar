/*	Public domain	*/

typedef struct sg_camera_polymode {
	enum {
		SG_CAMERA_POINTS,
		SG_CAMERA_WIREFRAME,
		SG_CAMERA_FLAT_SHADED,
		SG_CAMERA_SMOOTH_SHADED
	} mode;
	int cull;
} SG_CameraPolyMode;

typedef struct sg_camera {
	struct sg_node _inherit;		/* SG_Node -> SG_Camera */

	Uint flags;
#define SG_CAMERA_ROT_I 0x01	/* Artificial rotation around i (debug) */
#define SG_CAMERA_ROT_J 0x02	/* Artificial rotation around j (debug) */
#define SG_CAMERA_ROT_K 0x04	/* Artificial rotation around k (debug) */
#define SG_CAMERA_DRAW	0x08	/* Render a visible camera */

	enum sg_camera_pmode {
		SG_CAMERA_PERSPECTIVE,		/* Perspective projection */
		SG_CAMERA_ORTHOGRAPHIC,		/* Parallel projection */
		SG_CAMERA_USER_PROJ		/* User-specified */
	} pmode;
	SG_CameraPolyMode polyFace;	/* Front-facing polygon modes */
	SG_CameraPolyMode polyBack;	/* Back-facing polygon modes */
	M_Real fov;				/* Field of view (radians) */
	M_Real aspect;				/* Aspect ratio */
	M_Real pNear, pFar;			/* Clipping planes */
	M_Real rotSpeed;			/* For artificial rotate */
	M_Matrix44 userProj[2];			/* User projection matrices
						   (column-major) */
	enum sg_camera_rotctrl {
		SG_CAMERA_ROT_IGNORE,		/* Disable rotation control */
		SG_CAMERA_ROT_CIRCULAR,		/* Circular path (1 node) */
		SG_CAMERA_ROT_ELLIPTIC		/* Elliptic path (2 nodes) */
	} rotCtrl;
	Uint8 _pad1[4];
	SG_Node *_Nullable focus[2];		/* Center nodes */
	Uint8 _pad2[8];
} SG_Camera;

#define SGCAMERA(obj)            ((SG_Camera *)(obj))
#define SGCCAMERA(obj)           ((const SG_Camera *)(obj))
#define SG_CAMERA_SELF()          SGCAMERA( AG_OBJECT(0,"SG_Node:SG_Camera:*") )
#define SG_CAMERA_PTR(n)          SGCAMERA( AG_OBJECT((n),"SG_Node:SG_Camera:*") )
#define SG_CAMERA_NAMED(n)        SGCAMERA( AG_OBJECT_NAMED((n),"SG_Node:SG_Camera:*") )
#define SG_CONST_CAMERA_SELF()   SGCCAMERA( AG_CONST_OBJECT(0,"SG_Node:SG_Camera:*") )
#define SG_CONST_CAMERA_PTR(n)   SGCCAMERA( AG_CONST_OBJECT((n),"SG_Node:SG_Camera:*") )
#define SG_CONST_CAMERA_NAMED(n) SGCCAMERA( AG_CONST_OBJECT_NAMED((n),"SG_Node:SG_Camera:*") )

__BEGIN_DECLS
extern SG_NodeClass sgCameraClass;

SG_Camera *_Nonnull SG_CameraNew(void *_Nullable, const char *_Nullable);
SG_Camera *_Nonnull SG_CameraNewDuplicate(void *_Nullable, const char *_Nullable,
                                          SG_Camera *_Nonnull);

void SG_CameraFrustum(SG_Camera *_Nonnull, M_Rectangle3 *_Nullable,
                      M_Rectangle3 *_Nullable);

void SG_CameraProject(SG_Camera *_Nonnull);
void SG_CameraProjectLeft(SG_Camera *_Nonnull);
void SG_CameraProjectRight(SG_Camera *_Nonnull);
void SG_CameraGetProjection(SG_Camera *_Nonnull, M_Matrix44 *_Nonnull);
void SG_CameraSetBackPolyMode(SG_Camera *_Nonnull, const SG_CameraPolyMode *_Nonnull);
void SG_CameraSetFacePolyMode(SG_Camera *_Nonnull, const SG_CameraPolyMode *_Nonnull);

void SG_CameraSetPerspective(SG_Camera *_Nonnull, M_Real, M_Real);
void SG_CameraSetOrthographic(SG_Camera *_Nonnull);
void SG_CameraSetUser(SG_Camera *_Nonnull, const M_Matrix44 *_Nonnull,
                      const M_Matrix44 *_Nonnull);
void SG_CameraSetClipPlanes(SG_Camera *_Nonnull, M_Real, M_Real);
void SG_CameraSetup(SG_Camera *_Nonnull);
void SG_CameraRotMouse(SG_Camera *_Nonnull, struct sg_view *_Nonnull, int,int);
void SG_CameraMoveMouse(SG_Camera *_Nonnull, struct sg_view *_Nonnull, int,int,int);
void SG_CameraSetRotCtrlCircular(SG_Camera *_Nonnull, SG_Node *_Nullable);
void SG_CameraSetRotCtrlElliptic(SG_Camera *_Nonnull, SG_Node *_Nullable,
                                 SG_Node *_Nullable);
__END_DECLS

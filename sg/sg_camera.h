/*	$Csoft$	*/
/*	Public domain	*/

typedef struct sg_camera {
	SG_Vector3 v;			/* Position relative to root */
	SG_Quaternion o;		/* Orientation */
	TAILQ_ENTRY(sg_camera) cameras;
} SG_Camera;

__BEGIN_DECLS
SG_Camera *SG_CameraNew(void);
void	   SG_CameraInit(SG_Camera *);
__END_DECLS

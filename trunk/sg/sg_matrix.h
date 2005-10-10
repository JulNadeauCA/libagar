/*	$Csoft$	*/
/*	Public domain	*/

typedef struct sg_matrix3 { SG_Real m[9]; } SG_Matrix3;
typedef struct sg_matrix4 { SG_Real m[16]; } SG_Matrix4;

#define SG_MATRIX3_ZERO		{ 0,0,0, 0,0,0, 0,0,0 }
#define SG_MATRIX3_IDENTITY	{ 1,0,0, 0,1,0, 0,0,1 }
#define SG_MATRIX4_ZERO		{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }
#define SG_MATRIX4_IDENTITY	{ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }

__BEGIN_DECLS
void SG_LoadIdentity3(SG_Matrix3 *);
void SG_LoadIdentity4(SG_Matrix4 *);
__END_DECLS

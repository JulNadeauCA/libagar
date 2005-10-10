/*	$Csoft$	*/
/*	Public domain	*/

typedef struct sg_quaternion {
	SG_Real w, x, y, z;
} SG_Quaternion;

__BEGIN_DECLS
SG_Quaternion *SG_QuaternionNew(void);
void	       SG_QuaternionInit(SG_Quaternion *);
__END_DECLS

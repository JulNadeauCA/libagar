/*	$Csoft$	*/
/*	Public domain	*/

typedef struct sg_spherical {
	SG_Real phi;		/* Azimuth (longitude) */
	SG_Real theta;		/* Zenith (colatitude) */
	SG_Real r;		/* Radius */
} SG_Spherical;

__BEGIN_DECLS
__inline__ SG_Spherical	SG_CartToSph(SG_Vector);
__inline__ SG_Vector	SG_SphToCart(SG_Spherical);
__END_DECLS

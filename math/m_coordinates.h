/*	Public domain	*/

__BEGIN_DECLS
M_Rectangular M_RectangularFromSpherical(M_Spherical);
M_Rectangular M_RectangularFromCylindrical(M_Cylindrical);
M_Spherical   M_SphericalFromRectangular(M_Rectangular);
M_Spherical   M_SphericalFromCylindrical(M_Cylindrical);
M_Cylindrical M_CylindricalFromRectangular(M_Rectangular);
M_Cylindrical M_CylindricalFromSpherical(M_Spherical);
__END_DECLS

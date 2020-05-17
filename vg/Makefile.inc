# ex:syn=make
VG_LIBS=	${MATH_LIBS}
VG_CFLAGS=	${MATH_CFLAGS}
VG_LINKS=	ag_vg ${VG_LIBS}
VG_LINKS_STATIC=ag_vg_static ${VG_LIBS}

MODULE = Agar::Surface		PACKAGE = Agar::Surface		PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Surface
new(package, w, h, pf, ...)
	const char *package
	int w
	int h
	Agar::PixelFormat pf
PREINIT:
	Uint flags = 0;
CODE:
	if ((items == 5 && SvTYPE(SvRV(ST(4))) != SVt_PVHV) || items > 5) {
		Perl_croak(aTHX_ "Usage: Agar::Surface->new(w,h,pxFormat,[{opts}])");
	}
	if (items == 5) {
		AP_MapHashToFlags(SvRV(ST(4)), apSurfaceFlagNames, &flags);
	}
	RETVAL = AG_SurfaceNew(pf, w,h, flags);
OUTPUT:
	RETVAL

Agar::Surface
newIndexed(package, w, h, bitsPerPixel, ...)
	const char *package
	int w
	int h
	int bitsPerPixel
PREINIT:
	Uint flags = 0;
CODE:
	if ((items == 5 && SvTYPE(SvRV(ST(4))) != SVt_PVHV) || items > 5) {
		Perl_croak(aTHX_ "Usage: Agar::Surface->newIndexed(w,h,depth,"
		           "[{opts}])");
	}
	if (items == 5) {
		AP_MapHashToFlags(SvRV(ST(4)), apSurfaceFlagNames, &flags);
	}
	RETVAL = AG_SurfaceIndexed(w,h, bitsPerPixel, flags);
OUTPUT:
	RETVAL

Agar::Surface
newGrayscale(package, w, h, bitsPerPixel, ...)
	const char *package
	int w
	int h
	int bitsPerPixel
PREINIT:
	Uint flags = 0;
CODE:
	if ((items == 5 && SvTYPE(SvRV(ST(4))) != SVt_PVHV) || items > 5) {
		Perl_croak(aTHX_ "Usage: Agar::Surface->newGrayscale(w,h,depth,"
		           "[{opts}])");
	}
	if (items == 5) {
		AP_MapHashToFlags(SvRV(ST(4)), apSurfaceFlagNames, &flags);
	}
	RETVAL = AG_SurfaceGrayscale(w,h, bitsPerPixel, flags);
OUTPUT:
	RETVAL

Agar::Surface
newEmpty(package)
	const char *package
CODE:
	RETVAL = AG_SurfaceEmpty();
OUTPUT:
	RETVAL

Agar::Surface
newFromBMP(package, path)
	const char *package
	const char *path
CODE:
	if ((RETVAL = AG_SurfaceFromBMP(path)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

Agar::Surface
newFromPNG(package, path)
	const char *package
	const char *path
CODE:
	if ((RETVAL = AG_SurfaceFromPNG(path)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

Agar::Surface
newFromJPEG(package, path)
	const char *package
	const char *path
CODE:
	if ((RETVAL = AG_SurfaceFromJPEG(path)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

void
DESTROY(s)
	Agar::Surface s
CODE:
	AG_SurfaceFree(s);


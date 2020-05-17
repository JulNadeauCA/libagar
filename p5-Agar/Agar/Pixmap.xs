MODULE = Agar::Pixmap	PACKAGE = Agar::Pixmap	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Pixmap
new(package, parent, surface)
	const char * package
	Agar::Widget parent
	Agar::Surface surface
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Pixmap->new(parent,surface,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_PixmapFromSurface(parent, 0, surface);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

Agar::Pixmap
newScaled(package, parent, surface, w, h)
	const char * package
	Agar::Widget parent
	Agar::Surface surface
	int w
	int h
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 6 && SvTYPE(SvRV(ST(5))) != SVt_PVHV) || items > 6) {
		Perl_croak(aTHX_ "Usage: Agar::Pixmap->newScaled(parent,surface,w,h,[{opts}])");
	}
	if (items == 6) {
		AP_MapHashToFlags(SvRV(ST(5)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_PixmapFromSurfaceScaled(parent, 0, surface, w,h);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
setSourceCoords(self, x, y)
	Agar::Pixmap self
	int x
	int y
CODE:
	AG_PixmapSetCoords(self, x, y);

void
setFlag(self, name)
	Agar::Pixmap self
	const char * name
CODE:
	AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));

void
unsetFlag(self, name)
	Agar::Pixmap self
	const char * name
CODE:
	AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));

Uint
getFlag(self, name)
	Agar::Pixmap self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags, &RETVAL))
		{ XSRETURN_UNDEF; }
OUTPUT:
	RETVAL


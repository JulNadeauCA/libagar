MODULE = Agar::Slider	PACKAGE = Agar::Slider	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Slider
newHoriz(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Slider->newHoriz(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_SliderNew(parent, AG_SLIDER_HORIZ, 0);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

Agar::Slider
newVert(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Slider->newVert(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_SliderNew(parent, AG_SLIDER_VERT, 0);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
setIncrementInt(self, step)
	Agar::Slider self
	int step
CODE:
	AG_SetInt(self, "inc", step);

void
setIncrementFloat(self, step)
	Agar::Slider self
	double step
CODE:
	AG_SetFloat(self, "inc", step);

void
setFlag(self, name)
	Agar::Slider self
	const char * name
CODE:
	AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));

void
unsetFlag(self, name)
	Agar::Slider self
	const char * name
CODE:
	AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));

Uint
getFlag(self, name)
	Agar::Slider self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags, &RETVAL))
		{ XSRETURN_UNDEF; }
OUTPUT:
	RETVAL


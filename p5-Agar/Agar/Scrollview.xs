MODULE = Agar::Scrollview	PACKAGE = Agar::Scrollview	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Scrollview
new(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Scrollview->new(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), apScrollviewFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_ScrollviewNew(parent, flags);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
sizeHint(self, w, h)
	Agar::Scrollview self
	Uint w
	Uint h
CODE:
	AG_ScrollviewSizeHint(self, w, h);

void
setIncrement(self, pixels)
	Agar::Scrollview self
	int pixels
CODE:
	AG_ScrollviewSetIncrement(self, pixels);

void
setFlag(self, name)
	Agar::Scrollview self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, apScrollviewFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Scrollview self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, apScrollviewFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Scrollview self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apScrollviewFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL


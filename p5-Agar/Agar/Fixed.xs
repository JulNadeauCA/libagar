MODULE = Agar::Fixed	PACKAGE = Agar::Fixed	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Fixed
new(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Fixed->new(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), apFixedFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_FixedNew(parent, flags);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
size(self, child, w, h)
	Agar::Fixed self
	Agar::Widget child
	int w
	int h
CODE:
	AG_FixedSize(self, child, w, h);

void
move(self, child, x, y)
	Agar::Fixed self
	Agar::Widget child
	int x
	int y
CODE:
	AG_FixedMove(self, child, x, y);

void
setFlag(self, name)
	Agar::Fixed self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, apFixedFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Fixed self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, apFixedFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Fixed self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apFixedFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL


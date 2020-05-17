MODULE = Agar::Box	PACKAGE = Agar::Box	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Box
newHoriz(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Box->newHoriz(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), apBoxFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_BoxNewHoriz(parent, flags);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

Agar::Box
newVert(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Box->newVert(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), apBoxFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_BoxNewVert(parent, flags);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
setLabel(self, label)
	Agar::Box self
	const char * label
CODE:
	AG_BoxSetLabelS(self, label);

void
setHomogenous(self, flag)
	Agar::Box self
	int flag
CODE:
	AG_BoxSetHomogenous(self, flag);


void
setPadding(self, padding)
	Agar::Box self
	int padding
CODE:
	AG_SetStyleF(self, "padding", "%d", padding);

void
setSpacing(self, spacing)
	Agar::Box self
	int spacing
CODE:
	AG_SetStyleF(self, "spacing", "%d", spacing);

void
setHoriz(self)
	Agar::Box self
CODE:
	AG_BoxSetType(self, AG_BOX_HORIZ);

void
setVert(self)
	Agar::Box self
CODE:
	AG_BoxSetType(self, AG_BOX_HORIZ);

void
setDepth(self, depth)
	Agar::Box self
	int depth
CODE:
	AG_BoxSetDepth(self, depth);

void
setFlag(self, name)
	Agar::Box self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, apBoxFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Box self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, apBoxFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Box self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apBoxFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL


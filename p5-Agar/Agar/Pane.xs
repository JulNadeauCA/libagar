MODULE = Agar::Pane	PACKAGE = Agar::Pane	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Pane
newHoriz(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Pane->newHoriz(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), agPaneFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_PaneNewHoriz(parent, flags);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

Agar::Pane
newVert(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Pane->newVert(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), agPaneFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_PaneNewVert(parent, flags);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
setDividerWidth(self, pixels)
	Agar::Pane self
	int pixels
CODE:
	AG_PaneSetDividerWidth(self, pixels);

void
setDivisionMin(self, pixels)
	Agar::Pane self
	int pixels
CODE:
	AG_PaneSetDivisionMin(self, 0, pixels, pixels);
	AG_PaneSetDivisionMin(self, 1, pixels, pixels);

Agar::Box
leftPane(self)
	Agar::Pane self
CODE:
	RETVAL = self->div[0];
OUTPUT:
	RETVAL

Agar::Box
rightPane(self)
	Agar::Pane self
CODE:
	RETVAL = self->div[1];
OUTPUT:
	RETVAL

Agar::Box
topPane(self)
	Agar::Pane self
CODE:
	RETVAL = self->div[0];
OUTPUT:
	RETVAL

Agar::Box
bottomPane(self)
	Agar::Pane self
CODE:
	RETVAL = self->div[1];
OUTPUT:
	RETVAL

void
moveDivider(self, x)
	Agar::Pane self
	int x
CODE:
	AG_PaneMoveDivider(self, x);

void
moveDividerPct(self, pct)
	Agar::Pane self
	int pct
CODE:
	AG_PaneMoveDividerPct(self, pct);


void
setFlag(self, name)
	Agar::Pane self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, agPaneFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Pane self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, agPaneFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Pane self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, agPaneFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL


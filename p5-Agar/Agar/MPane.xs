MODULE = Agar::MPane	PACKAGE = Agar::MPane	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::MPane
new(package, parent, layout, ...)
	const char * package
	Agar::Widget parent
	char * layout
PREINIT:
	Uint flags = 0, wflags = 0;
	enum ag_mpane_layout layout_enum = AG_MPANE4;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::MPane->new(parent,layout,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), apMPaneFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(3)), apWidgetFlagNames, &wflags);
	}
	if (strlen(layout) > 1) {
		layout[1] = toLOWER(layout[1]);
		if (strlen(layout) > 3) {
			layout[3] = toLOWER(layout[3]);
		}
	}
	if (strEQ(layout, "1"))		{ layout_enum = AG_MPANE1; }
	else if (strEQ(layout, "2v"))	{ layout_enum = AG_MPANE2V; }
	else if (strEQ(layout, "2h"))	{ layout_enum = AG_MPANE2H; }
	else if (strEQ(layout, "2l1r"))	{ layout_enum = AG_MPANE2L1R; }
	else if (strEQ(layout, "1l2r"))	{ layout_enum = AG_MPANE1L2R; }
	else if (strEQ(layout, "2t1b"))	{ layout_enum = AG_MPANE2T1B; }
	else if (strEQ(layout, "1t2b"))	{ layout_enum = AG_MPANE1T2B; }
	else if (strEQ(layout, "3l1r"))	{ layout_enum = AG_MPANE3L1R; }
	else if (strEQ(layout, "1l3r"))	{ layout_enum = AG_MPANE1L3R; }
	else if (strEQ(layout, "3t1b"))	{ layout_enum = AG_MPANE3T1B; }
	else if (strEQ(layout, "1t3b"))	{ layout_enum = AG_MPANE1T3B; }
	RETVAL = AG_MPaneNew(parent, layout_enum, flags);
	if (RETVAL) { AGWIDGET(&(RETVAL->box))->flags |= wflags; }
OUTPUT:
	RETVAL

Agar::Box
pane(self, index)
	Agar::MPane self
	Uint index
CODE:
	if (index >= self->nPanes) {
		Perl_croak(aTHX_ "Pane index out of bounds");
	}
	RETVAL = self->panes[index];
OUTPUT:
	RETVAL

void
setFlag(self, name)
	Agar::MPane self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, apMPaneFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(&self->box)->flags));
	}

void
unsetFlag(self, name)
	Agar::MPane self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, apMPaneFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(&self->box)->flags));
	}

Uint
getFlag(self, name)
	Agar::MPane self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apMPaneFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(&self->box)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL


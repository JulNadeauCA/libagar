MODULE = Agar::Toolbar		PACKAGE = Agar::Toolbar		PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Toolbar
newHoriz(package, parent, numRows, ...)
	const char * package
	Agar::Widget parent
	int numRows
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Toolbar->newHoriz(parent,rows,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), apToolbarFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(3)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_ToolbarNew(parent, AG_TOOLBAR_HORIZ, numRows, flags);
	if (RETVAL) { AGWIDGET(&(RETVAL->box))->flags |= wflags; }
OUTPUT:
	RETVAL

Agar::Toolbar
newVert(package, parent, numRows, ...)
	const char * package
	Agar::Widget parent
	int numRows
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Toolbar->newVert(parent,rows,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), apToolbarFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(3)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_ToolbarNew(parent, AG_TOOLBAR_VERT, numRows, flags);
	if (RETVAL) { AGWIDGET(&(RETVAL->box))->flags |= wflags; }
OUTPUT:
	RETVAL

void
setActiveRow(self, row)
	Agar::Toolbar self
	int row
CODE:
	AG_ToolbarRow(self, row);

Agar::Button
addTextButton(self, text)
	Agar::Toolbar self
	const char * text
CODE:
	RETVAL = AG_ToolbarButton(self, text, 0, NULL, "");
OUTPUT:
	RETVAL

Agar::Button
addIconButton(self, surface)
	Agar::Toolbar self
	Agar::Surface surface
CODE:
	RETVAL = AG_ToolbarButtonIcon(self, AG_SurfaceDup(surface), 0, NULL, "");
OUTPUT:
	RETVAL

void
select(self, button)
	Agar::Toolbar self
	Agar::Button button
CODE:
	AG_ToolbarSelect(self, button);

void
deselect(self, button)
	Agar::Toolbar self
	Agar::Button button
CODE:
	AG_ToolbarDeselect(self, button);

void
selectOnly(self, button)
	Agar::Toolbar self
	Agar::Button button
CODE:
	AG_ToolbarSelectOnly(self, button);

void
selectAll(self)
	Agar::Toolbar self
CODE:
	AG_ToolbarSelectAll(self);

void
deselectAll(self)
	Agar::Toolbar self
CODE:
	AG_ToolbarDeselectAll(self);

Agar::Box
getRow(self, index)
	Agar::Toolbar self
	Uint index
CODE:
	if (index >= self->nRows) {
		Perl_croak(aTHX_ "Row index out of bounds");
	}
	RETVAL = self->rows[index];
OUTPUT:
	RETVAL

void
setFlag(self, name)
	Agar::Toolbar self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, apToolbarFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(&self->box)->flags));
	}

void
unsetFlag(self, name)
	Agar::Toolbar self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, apToolbarFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(&self->box)->flags));
	}

Uint
getFlag(self, name)
	Agar::Toolbar self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apToolbarFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(&self->box)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL


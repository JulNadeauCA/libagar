MODULE = Agar::Editable		PACKAGE = Agar::Editable	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Editable
new(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Editable->new(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), apEditableFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_EditableNew(parent, flags);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
sizeHint(self, text)
	Agar::Editable self
	const char * text
CODE:
	AG_EditableSizeHint(self, text);

void
sizeHintPixels(self, w, h)
	Agar::Editable self
	Uint w
	Uint h
CODE:
	AG_EditableSizeHintPixels(self, w, h);

void
sizeHintLines(self, numLines)
	Agar::Editable self
	Uint numLines
CODE:
	AG_EditableSizeHintLines(self, numLines);

int
getCursorPos(self)
	Agar::Editable self
CODE:
	RETVAL = AG_EditableGetCursorPos(self);
OUTPUT:
	RETVAL

void
setFlag(self, name)
	Agar::Editable self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, apEditableFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Editable self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, apEditableFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Editable self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apEditableFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL


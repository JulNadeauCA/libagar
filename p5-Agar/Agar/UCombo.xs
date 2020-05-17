MODULE = Agar::UCombo	PACKAGE = Agar::UCombo	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::UCombo
new(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::UCombo->new(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), apUcomboFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_UComboNew(parent, flags);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
sizeHint(self, text, numItems)
	Agar::UCombo self
	const char * text
	int numItems
CODE:
	AG_UComboSizeHint(self, text, numItems);

void
sizeHintPixels(self, w, h)
	Agar::UCombo self
	int w
	int h
CODE:
	AG_UComboSizeHintPixels(self, w, h);

Agar::Tlist
list(self)
	Agar::UCombo self
CODE:
	RETVAL = self->list;
OUTPUT:
	RETVAL

Agar::Button
button(self)
	Agar::UCombo self
CODE:
	RETVAL = self->button;
OUTPUT:
	RETVAL

void
setFlag(self, name)
	Agar::UCombo self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, apUcomboFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::UCombo self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, apUcomboFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::UCombo self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apUcomboFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL


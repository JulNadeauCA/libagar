MODULE = Agar::Combo	PACKAGE = Agar::Combo	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Combo
new(package, parent, label, ...)
	const char * package
	Agar::Widget parent
	const char * label
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Combo->new(parent,label,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), apComboFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(3)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_ComboNewS(parent, flags, label);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
sizeHint(self, text, numItems)
	Agar::Combo self
	const char * text
	int numItems
CODE:
	AG_ComboSizeHint(self, text, numItems);

void
sizeHintPixels(self, w, h)
	Agar::Combo self
	int w
	int h
CODE:
	AG_ComboSizeHintPixels(self, w, h);

void
select(self, item)
	Agar::Combo self
	Agar::TlistItem item
CODE:
	AG_ComboSelect(self, item);

void
selectText(self, text)
	Agar::Combo self
	const char * text
CODE:
	AG_ComboSelectText(self, text);

Agar::Tlist
list(self)
	Agar::Combo self
CODE:
	RETVAL = self->list;
OUTPUT:
	RETVAL

Agar::Textbox
tbox(self)
	Agar::Combo self
CODE:
	RETVAL = self->tbox;
OUTPUT:
	RETVAL

Agar::Button
button(self)
	Agar::Combo self
CODE:
	RETVAL = self->button;
OUTPUT:
	RETVAL

void
setFlag(self, name)
	Agar::Combo self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, apComboFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Combo self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, apComboFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Combo self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apComboFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL


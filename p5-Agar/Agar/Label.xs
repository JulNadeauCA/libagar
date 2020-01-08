MODULE = Agar::Label	PACKAGE = Agar::Label	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Label
new(package, parent, text, ...)
	const char * package
	Agar::Widget parent
	const char * text
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Label->new(parent,text,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), apLabelFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(3)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_LabelNewS(parent, flags, text);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
setPadding(self, left, right, top, bottom)
	Agar::Label self
	int left
	int right
	int top
	int bottom
CODE:
	AG_SetStyleF(self, "padding", "%i %i %i %i", top, right, bottom, left);

void
justify(self, mode)
	Agar::Label self
	const char * mode
CODE:
	switch (mode[0]) {
		case 'L': case 'l': AG_LabelJustify(self,AG_TEXT_LEFT); break;
		case 'R': case 'r': AG_LabelJustify(self,AG_TEXT_RIGHT); break;
		case 'C': case 'c': AG_LabelJustify(self,AG_TEXT_CENTER); break;
		default: Perl_croak(aTHX_ "Invalid justify mode");
	}

void
vAlign(self, mode)
	Agar::Label self
	const char * mode
CODE:
	switch (mode[0]) {
		case 'T': case 't': AG_LabelValign(self,AG_TEXT_TOP); break;
		case 'M': case 'm': AG_LabelValign(self,AG_TEXT_MIDDLE); break;
		case 'B': case 'b': AG_LabelValign(self,AG_TEXT_BOTTOM); break;
		default: Perl_croak(aTHX_ "Invalid vAlign mode");
	}

void
sizeHint(self, numLines, text)
	Agar::Label self
	Uint numLines
	const char * text
CODE:
	AG_LabelSizeHint(self, numLines, text);

void
setText(self, text)
	Agar::Label self
	const char * text
CODE:
	AG_LabelTextS(self, text);

void
setFlag(self, name)
	Agar::Label self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, apLabelFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Label self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, apLabelFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Label self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apLabelFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL


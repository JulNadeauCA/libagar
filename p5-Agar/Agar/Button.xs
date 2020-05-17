MODULE = Agar::Button	PACKAGE = Agar::Button	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Button
new(package, parent, label, ...)
	const char * package
	Agar::Widget parent
	const char * label
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Button->new(parent,label,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), apButtonFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(3)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_ButtonNewS(parent, flags, label);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
setPadding(self, left, right, top, bottom)
	Agar::Button self
	int left
	int right
	int top
	int bottom
CODE:
	AG_SetStyleF(self, "padding", "%i %i %i %i", top, right, bottom, left);

void
justify(self, mode)
	Agar::Button self
	const char * mode
CODE:
	switch (mode[0]) {
		case 'L':case 'l':AG_ButtonJustify(self,AG_TEXT_LEFT); break;
		case 'R':case 'r':AG_ButtonJustify(self,AG_TEXT_RIGHT); break;
		case 'C':case 'c':AG_ButtonJustify(self,AG_TEXT_CENTER); break;
		default: Perl_croak(aTHX_ "Invalid justify mode");
	}

void
vAlign(self, mode)
	Agar::Button self
	const char * mode
CODE:
	switch (mode[0]) {
		case 'T':case 't':AG_ButtonValign(self,AG_TEXT_TOP); break;
		case 'M':case 'm':AG_ButtonValign(self,AG_TEXT_MIDDLE); break;
		case 'B':case 'b':AG_ButtonValign(self,AG_TEXT_BOTTOM); break;
		default: Perl_croak(aTHX_ "Invalid vAlign mode");
	}

void
setRepeatMode(self, on)
	Agar::Button self
	int on
CODE:
	AG_ButtonSetRepeatMode(self, on);

void
surface(self, surface)
	Agar::Button self
	Agar::Surface surface
CODE:
	AG_ButtonSurface(self, surface);

void
setFlag(self, name)
	Agar::Button self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, apButtonFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Button self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, apButtonFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Button self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apButtonFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL


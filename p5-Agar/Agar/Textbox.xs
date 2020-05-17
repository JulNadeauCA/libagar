MODULE = Agar::Textbox		PACKAGE = Agar::Textbox		PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Textbox
new(package, parent, label, ...)
	const char * package
	Agar::Widget parent
	const char * label
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Textbox->new(parent,label,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), apTextboxFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(3)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_TextboxNewS(parent, flags, label);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
sizeHint(self, text)
	Agar::Textbox self
	const char * text
CODE:
	AG_TextboxSizeHint(self, text);

void
sizeHintPixels(self, w, h)
	Agar::Textbox self
	Uint w
	Uint h
CODE:
	AG_TextboxSizeHintPixels(self, w, h);

void
sizeHintLines(self, numLines)
	Agar::Textbox self
	Uint numLines
CODE:
	AG_TextboxSizeHintLines(self, numLines);

int
getCursorPos(self)
	Agar::Textbox self
CODE:
	RETVAL = AG_TextboxGetCursorPos(self);
OUTPUT:
	RETVAL

void
setFlag(self, name)
	Agar::Textbox self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, apTextboxFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Textbox self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, apTextboxFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Textbox self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apTextboxFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL

SV *
getString(self)
	Agar::Textbox self
CODE:
	RETVAL = newSVpv(AG_TextboxDupString(self), 0);
OUTPUT:
	RETVAL

void
setString(self, val)
	Agar::Textbox self
	const char * val
CODE:
	AG_TextboxSetString(self, val);

void
clearString(self, val)
	Agar::Textbox self
CODE:
	AG_TextboxClearString(self);

int
getInt(self)
	Agar::Textbox self
CODE:
	RETVAL = AG_TextboxInt(self);
OUTPUT:
	RETVAL

float
getFloat(self)
	Agar::Textbox self
CODE:
	RETVAL = AG_TextboxFloat(self);
OUTPUT:
	RETVAL

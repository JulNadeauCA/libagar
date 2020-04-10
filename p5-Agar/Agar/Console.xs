MODULE = Agar::Console		PACKAGE = Agar::Console		PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Console
new(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Console->new(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_ConsoleNew(parent, 0);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
setPadding(self, pixels)
	Agar::Console self
	int pixels
CODE:
	AG_SetStyleF(self, "padding", "%d", pixels);

void
msg(self, text)
	Agar::Console self
	const char * text
CODE:
	AG_ConsoleAppendLine(self, text);

Agar::Scrollbar
scrollBar(self)
	Agar::Console self
CODE:
	RETVAL = self->vBar;
OUTPUT:
	RETVAL

void
setFlag(self, name)
	Agar::Console self
	const char * name
CODE:
	AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));

void
unsetFlag(self, name)
	Agar::Console self
	const char * name
CODE:
	AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));

Uint
getFlag(self, name)
	Agar::Console self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags, &RETVAL))
		{ XSRETURN_UNDEF; }
OUTPUT:
	RETVAL


MODULE = Agar::Numerical	PACKAGE = Agar::Numerical	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Numerical
new(package, parent, label, ...)
	const char * package
	Agar::Widget parent
	const char * label
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Numerical->new(parent,label,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_NumericalNewS(parent, 0, NULL, label);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
setWriteable(self, on)
	Agar::Numerical self
	int on
CODE:
	AG_NumericalSetWriteable(self, on);

void
setRangeInt(self, min, max)
	Agar::Numerical self
	int min
	int max
CODE:
	AG_SetInt(self, "min", min);
	AG_SetInt(self, "max", max);

void
setRangeFloat(self, min, max)
	Agar::Numerical self
	float min
	float max
CODE:
	AG_SetFloat(self, "min", min);
	AG_SetFloat(self, "max", max);

void
setRangeDouble(self, min, max)
	Agar::Numerical self
	double min
	double max
CODE:
	AG_SetDouble(self, "min", min);
	AG_SetDouble(self, "max", max);

void
setValueInt(self, value)
	Agar::Numerical self
	int value
CODE:
	AG_SetInt(self, "value", value);

void
setValueFloat(self, value)
	Agar::Numerical self
	float value
CODE:
	AG_SetFloat(self, "value", value);

void
setValueDouble(self, value)
	Agar::Numerical self
	double value
CODE:
	AG_SetDouble(self, "value", value);

void
setPrecision(self, format, prec)
	Agar::Numerical self
	const char * format
	int prec
CODE:
	AG_NumericalSetPrecision(self, format, prec);

void
setFlag(self, name)
	Agar::Numerical self
	const char * name
CODE:
	AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));

void
unsetFlag(self, name)
	Agar::Numerical self
	const char * name
CODE:
	AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));

Uint
getFlag(self, name)
	Agar::Numerical self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags, &RETVAL))
		{ XSRETURN_UNDEF; }
OUTPUT:
	RETVAL


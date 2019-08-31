MODULE = Agar::Scrollbar	PACKAGE = Agar::Scrollbar	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Scrollbar
newHoriz(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Scrollbar->newHoriz(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_ScrollbarNew(parent, AG_SCROLLBAR_HORIZ, 0);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

Agar::Scrollbar
newVert(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Scrollbar->newVert(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_ScrollbarNew(parent, AG_SCROLLBAR_VERT, 0);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

void
setIncrementInt(self, step)
	Agar::Scrollbar self
	int step
CODE:
	AG_SetInt(self, "inc", step);

void
setIncrementFloat(self, step)
	Agar::Scrollbar self
	double step
CODE:
	AG_SetFloat(self, "inc", step);

void
incAction(self, coderef)
	Agar::Scrollbar self
	SV * coderef
CODE:
	if (SvTYPE(SvRV(coderef)) == SVt_PVCV) {
		SvREFCNT_inc(coderef);
		AP_DecRefEventPV(self->buttonIncFn);
		AG_ScrollbarSetIncFn(self, AP_EventHandler, "%p", coderef);
	} else {
		Perl_croak(aTHX_ "Usage: $scrollbar->incAction(codeRef)");
	}

void
decAction(self, coderef)
	Agar::Scrollbar self
	SV * coderef
CODE:
	if (SvTYPE(SvRV(coderef)) == SVt_PVCV) {
		SvREFCNT_inc(coderef);
		AP_DecRefEventPV(self->buttonDecFn);
		AG_ScrollbarSetDecFn(self, AP_EventHandler, "%p", coderef);
	} else {
		Perl_croak(aTHX_ "Usage: $scrollbar->decAction(codeRef)");
	}

void
setFlag(self, name)
	Agar::Scrollbar self
	const char * name
CODE:
	AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));

void
unsetFlag(self, name)
	Agar::Scrollbar self
	const char * name
CODE:
	AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));

Uint
getFlag(self, name)
	Agar::Scrollbar self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags, &RETVAL))
		{ XSRETURN_UNDEF; }
OUTPUT:
	RETVAL


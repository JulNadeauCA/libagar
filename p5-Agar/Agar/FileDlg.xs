MODULE = Agar::FileDlg		PACKAGE = Agar::FileDlg		PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::FileDlg
new(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::FileDlg->new(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), apFileDlgFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_FileDlgNew(parent, flags);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

int
setDirectory(self, path)
	Agar::FileDlg self
	const char * path
CODE:
	RETVAL = AG_FileDlgSetDirectoryS(self, path);
OUTPUT:
	RETVAL

void
setFilename(self, name)
	Agar::FileDlg self
	const char * name
CODE:
	AG_FileDlgSetFilenameS(self, name);

void
okAction(self, coderef)
	Agar::FileDlg self
	SV * coderef
CODE:
	if (SvTYPE(SvRV(coderef)) == SVt_PVCV) {
		SvREFCNT_inc(coderef);
		AP_DecRefEventPV(self->okAction);
		AG_FileDlgOkAction(self, AP_EventHandler, "%p", coderef);
	} else {
		Perl_croak(aTHX_ "Usage: $dialog->okAction(codeRef)");
	}

void
cancelAction(self, coderef)
	Agar::FileDlg self
	SV * coderef
CODE:
	if (SvTYPE(SvRV(coderef)) == SVt_PVCV) {
		SvREFCNT_inc(coderef);
		AP_DecRefEventPV(self->cancelAction);
		AG_FileDlgCancelAction(self, AP_EventHandler, "%p", coderef);
	} else {
		Perl_croak(aTHX_ "Usage: $dialog->cancelAction(codeRef)");
	}

void
setFlag(self, name)
	Agar::FileDlg self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, apFileDlgFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::FileDlg self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, apFileDlgFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::FileDlg self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, apFileDlgFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL


MODULE = Agar::Notebook		PACKAGE = Agar::Notebook	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Notebook
new(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Notebook->new(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), agNotebookFlagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_NotebookNew(parent, flags);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

Agar::NotebookTab
addHorizTab(self, label)
	Agar::Notebook self
	const char * label
CODE:
	RETVAL = AG_NotebookAdd(self, label, AG_BOX_HORIZ);
OUTPUT:
	RETVAL

Agar::NotebookTab
addVertTab(self, label)
	Agar::Notebook self
	const char * label
CODE:
	RETVAL = AG_NotebookAdd(self, label, AG_BOX_VERT);
OUTPUT:
	RETVAL

void
delTab(self, tab)
	Agar::Notebook self
	Agar::NotebookTab tab
CODE:
	AG_NotebookDel(self, tab);

void
selectTab(self, tab)
	Agar::Notebook self
	Agar::NotebookTab tab
CODE:
	AG_NotebookSelect(self, tab);

void
setFlag(self, name)
	Agar::Notebook self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, agNotebookFlagNames, &(self->flags))) {
		AP_SetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Notebook self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, agNotebookFlagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, apWidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Notebook self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, agNotebookFlagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, apWidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL

MODULE = Agar::Notebook		PACKAGE = Agar::NotebookTab	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Box
box(self)
	Agar::NotebookTab self
CODE:
	RETVAL = &(self->box);
OUTPUT:
	RETVAL


MODULE = Agar::Menu	PACKAGE = Agar::Menu	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Menu
new(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Menu->new(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_MenuNew(parent, 0);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

Agar::Menu
newGlobal(package, ...)
	const char * package
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 2 && SvTYPE(SvRV(ST(1))) != SVt_PVHV) || items > 2) {
		Perl_croak(aTHX_ "Usage: Agar::Menu->newGlobal([{opts}])");
	}
	if (items == 2) {
		AP_MapHashToFlags(SvRV(ST(1)), apWidgetFlagNames, &wflags);
	}
	RETVAL = AG_MenuNewGlobal(0);
	if (RETVAL) { AGWIDGET(RETVAL)->flags |= wflags; }
OUTPUT:
	RETVAL

Agar::MenuItem
rootItem(self)
	Agar::Menu self
CODE:
	RETVAL = self->root;
OUTPUT:
	RETVAL

void
expandItem(self, item, x, y)
	Agar::Widget self
	Agar::MenuItem item
	int x
	int y
PREINIT:
	AG_Widget *parent;
CODE:
	AG_MenuExpand(self, item, x, y);

void
collapseItem(item)
	Agar::MenuItem item
CODE:
	AG_MenuCollapse(item);

void
setPadding(self, l, r, t, b)
	Agar::Menu self
	int l
	int r
	int t
	int b
CODE:
	AG_SetStyleF(self, "padding", "%d %d %d %d", l,r,t,b);

void
setLabelPadding(self, l, r, t, b)
	Agar::Menu self
	int l
	int r
	int t
	int b
CODE:
	AG_MenuSetLabelPadding(self, l, r, t, b);

MODULE = Agar::Menu	PACKAGE = Agar::PopupMenu	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::PopupMenu
new(package, parent)
	const char * package
	Agar::Widget parent
CODE:
	RETVAL = AG_PopupNew(parent);
OUTPUT:
	RETVAL

void
show(self)
	Agar::PopupMenu self
CODE:
	AG_PopupShow(self);

void
showAt(self, x, y)
	Agar::PopupMenu self
	int x
	int y
CODE:
	AG_PopupShowAt(self, x,y);

void
hide(self)
	Agar::PopupMenu self
CODE:
	AG_PopupHide(self);

void
destroy(self)
	Agar::PopupMenu self
CODE:
	AG_PopupDestroy(self);

Agar::MenuItem
rootItem(self)
	Agar::PopupMenu self
CODE:
	RETVAL = self->root;
OUTPUT:
	RETVAL

MODULE = Agar::Menu	PACKAGE = Agar::MenuItem	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::MenuItem
nodeItem(self, text)
	Agar::MenuItem self
	const char * text
CODE:
	RETVAL = AG_MenuNode(self, text, NULL);
OUTPUT:
	RETVAL

Agar::MenuItem
actionItem(self, text, coderef)
	Agar::MenuItem self
	const char * text
	SV * coderef
CODE:
	SvREFCNT_inc(coderef);
	RETVAL = AG_MenuAction(self, text, NULL, AP_EventHandler, "%p", coderef);
OUTPUT:
	RETVAL

Agar::MenuItem
actionItemKbd(self, text, key, mod, coderef)
	Agar::MenuItem self
	const char * text
	int key
	int mod
	SV * coderef
CODE:
	SvREFCNT_inc(coderef);
	RETVAL = AG_MenuActionKb(self, text, NULL, key, mod, AP_EventHandler, "%p", coderef);
OUTPUT:
	RETVAL

void
separator(self)
	Agar::MenuItem self
CODE:
	AG_MenuSeparator(self);

void
section(self, text)
	Agar::MenuItem self
	const char * text
CODE:
	AG_MenuSectionS(self, text);

void
setIcon(self, surface)
	Agar::MenuItem self
	Agar::Surface surface
CODE:
	AG_MenuSetIcon(self, surface);

void
setLabel(self, text)
	Agar::MenuItem self
	const char * text
CODE:
	AG_MenuSetLabelS(self, text);

void
enable(self)
	Agar::MenuItem self
CODE:
	AG_MenuEnable(self);

void
disable(self)
	Agar::MenuItem self
CODE:
	AG_MenuDisable(self);

Agar::MenuItem
selected(self)
	Agar::MenuItem self
CODE:
	if ((RETVAL = self->sel_subitem) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

Agar::MenuItem
parentItem(self)
	Agar::MenuItem self
CODE:
	if ((RETVAL = self->parent) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

Agar::Menu
parentMenu(self)
	Agar::MenuItem self
CODE:
	RETVAL = self->pmenu;
OUTPUT:
	RETVAL


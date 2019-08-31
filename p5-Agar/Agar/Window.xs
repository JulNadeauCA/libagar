MODULE = Agar::Window	PACKAGE = Agar::Window	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Window
new(package, ...)
	const char *package
PREINIT:
	Uint flags = 0;
CODE:
	if ((items == 2 && SvTYPE(SvRV(ST(1))) != SVt_PVHV) || items > 2) {
		Perl_croak(aTHX_ "Usage: Agar::Window->new([{opts}])");
	}
	if (items == 2) {
		AP_MapHashToFlags(SvRV(ST(1)), agWindowFlagNames, &flags);
	}
	RETVAL = AG_WindowNew(flags);
OUTPUT:
	RETVAL

Agar::Window
newNamed(package, name, ...)
	const char *package
	const char *name
PREINIT:
	Uint flags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Window->newNamed(name,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), agWindowFlagNames, &flags);
	}
	if ((RETVAL = AG_WindowNewNamedS(flags, name)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

SV *
caption(win, ...)
	Agar::Window win
CODE:
	if (items > 1) {
		AG_WindowSetCaptionS(win, (char *)SvPV_nolen(ST(1)));
	}
	RETVAL = newSVpv(win->caption,0);
OUTPUT:
	RETVAL

Agar::Surface
icon(win, ...)
	Agar::Window win
CODE:
	if (items > 1) {
		Agar__Surface s;
		if (sv_derived_from(ST(1), "Agar::Surface")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			s = INT2PTR(Agar__Surface,tmp);
		} else {
	    		Perl_croak(aTHX_ "icon is not of type Agar::Surface");
		}
		AG_WindowSetIcon(win, s);
	}
	RETVAL = AGWIDGET_SURFACE(win->icon,win->icon->surface);
OUTPUT:
	RETVAL

void
setGeometry(win, x, y, w, h)
	Agar::Window win
	int x
	int y
	int w
	int h
CODE:
	AG_WindowSetGeometry(win, x, y, w, h);

void
setMinSize(win, w, h)
	Agar::Window win
	int w
	int h
CODE:
	AG_WindowSetMinSize(win, w, h);

void
show(win)
	Agar::Window win
CODE:
	AG_WindowShow(win);

void
hide(win)
	Agar::Window win
CODE:
	AG_WindowHide(win);

void
draw(win)
	Agar::Window win
CODE:
	AG_WindowDraw(win);

void
attach(win, chld)
	Agar::Window win
	Agar::Window chld
CODE:
	AG_WindowAttach(win, chld);

void
detach(win, chld)
	Agar::Window win
	Agar::Window chld
CODE:
	AG_WindowDetach(win, chld);

Agar::Widget
findFocused(win)
	Agar::Window win
CODE:
	if ((RETVAL = AG_WidgetFindFocused(win)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

void
DESTROY(win)
	Agar::Window win
CODE:
	AG_ObjectDetach(win);


MODULE = Agar::Widget	PACKAGE = Agar::Widget	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

void
draw(self)
	Agar::Widget self
CODE:
	AG_WidgetDraw(self);

void
enable(self)
	Agar::Widget self
CODE:
	AG_WidgetEnable(self);

void
disable(self)
	Agar::Widget self
CODE:
	AG_WidgetDisable(self);

int
isEnabled(self)
	Agar::Widget self
CODE:
	RETVAL = AG_WidgetEnabled(self);
OUTPUT:
	RETVAL

int
isDisabled(self)
	Agar::Widget self
CODE:
	RETVAL = AG_WidgetDisabled(self);
OUTPUT:
	RETVAL

void
setFocusable(self, isFocusable)
	Agar::Widget self
	int isFocusable
CODE:
	AG_WidgetSetFocusable(self, isFocusable);

int
isFocused(self)
	Agar::Widget self
CODE:
	RETVAL = AG_WidgetIsFocused(self);
OUTPUT:
	RETVAL

int
isFocusedInWindow(self)
	Agar::Widget self
CODE:
	RETVAL = AG_WidgetIsFocusedInWindow(self);
OUTPUT:
	RETVAL

void
focus(self)
	Agar::Widget self
CODE:
	AG_WidgetFocus(self);

void
unfocus(self)
	Agar::Widget self
CODE:
	AG_WidgetUnfocus(self);

Agar::Window
window(self)
	Agar::Widget self
CODE:
	RETVAL = AG_ParentWindow(self);
OUTPUT:
	RETVAL

void
requestSize(self, w, h)
	Agar::Widget self
	int w
	int h
PREINIT:
	static AG_SizeReq sizereq;
CODE:
	sizereq.w = w;
	sizereq.h = h;
	AG_WidgetSizeReq(self, &sizereq);

void
setStyle(self, attr, value)
	Agar::Widget self
	const char *attr
	const char *value
CODE:
	AG_SetStyle(self, attr, value);

void
setSize(self, w, h)
	Agar::Widget self
	int w
	int h
CODE:
	AG_WidgetSetSize(self, w, h);

int
x(self)
	Agar::Widget self
CODE:
	RETVAL = self->x;
OUTPUT:
	RETVAL

int
y(self)
	Agar::Widget self
CODE:
	RETVAL = self->y;
OUTPUT:
	RETVAL

int
w(self)
	Agar::Widget self
CODE:
	RETVAL = self->w;
OUTPUT:
	RETVAL

int
h(self)
	Agar::Widget self
CODE:
	RETVAL = self->h;
OUTPUT:
	RETVAL

void
expandHoriz(self)
	Agar::Widget self
CODE:
	AG_ExpandHoriz(self);

void
expandVert(self)
	Agar::Widget self
CODE:
	AG_ExpandVert(self);

void
expand(self)
	Agar::Widget self
CODE:
	AG_Expand(self);

void
redraw(self)
	Agar::Widget self
CODE:
	AG_Redraw(self);

void
redrawOnChange(self, refresh_ms, name)
	Agar::Widget self
	int refresh_ms
	const char *name
CODE:
	AG_RedrawOnChange(self, refresh_ms, name);

void
redrawOnTick(self, refresh_ms)
	Agar::Widget self
	int refresh_ms
CODE:
	AG_RedrawOnTick(self, refresh_ms);


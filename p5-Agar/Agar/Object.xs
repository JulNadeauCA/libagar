MODULE = Agar::Object	PACKAGE = Agar::Object	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Event
setEvent(self, event_name, coderef)
	Agar::Object self
	const char * event_name
	SV * coderef
CODE:
	if (SvTYPE(SvRV(coderef)) == SVt_PVCV) {
		AP_DecRefEventPV(AG_FindEventHandler(self, event_name));
		RETVAL = AG_SetEvent(self, event_name, AP_EventHandler, "");
		AP_StoreEventPV(RETVAL, coderef);
	} else {
		Perl_croak(aTHX_ "Usage: $object->setEvent(eventName, codeRef)");
	}
OUTPUT:
	RETVAL

void
unsetEvent(self, event_name)
	Agar::Object self
	const char * event_name
CODE:
	AP_DecRefEventPV(AG_FindEventHandler(self, event_name));
	AG_UnsetEvent(self, event_name);

SV *
findEventHandler(self, event_name)
	Agar::Object self
	const char * event_name
PREINIT:
	AG_Event *event;
CODE:
	event = AG_FindEventHandler(self, event_name);
	if (event == NULL || event->fn == NULL) {
		XSRETURN_UNDEF;
	} else if ((RETVAL = AP_RetrieveEventPV(event)) == NULL) {
		XSRETURN_NO;
	}
OUTPUT:
	RETVAL

void
lock(self)
	Agar::Object self
CODE:
	AG_ObjectLock(self);

void
unlock(self)
	Agar::Object self
CODE:
	AG_ObjectUnlock(self);

void
attachTo(self, newParent)
	Agar::Object self
	Agar::Object newParent
CODE:
	AG_ObjectAttach(newParent, self);

void
detach(self)
	Agar::Object self
CODE:
	AG_ObjectDetach(self);

Agar::Object
root(self)
	Agar::Object self
CODE:
	RETVAL = AG_ObjectRoot(self);
OUTPUT:
	RETVAL

Agar::Object
parent(self)
	Agar::Object self
CODE:
	RETVAL = AG_ObjectParent(self);
OUTPUT:
	RETVAL

Agar::Object
findChild(self, name)
	Agar::Object self
	const char * name
CODE:
	if ((RETVAL = AG_ObjectFindChild(self, name)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

void
setName(self, name)
	Agar::Object self
	const char * name
CODE:
	AG_ObjectSetNameS(self, name);

SV *
getClassName(self)
	Agar::Object self
CODE:
	RETVAL = newSVpv(AGOBJECT_CLASS(self)->name, 0);
OUTPUT:
	RETVAL

SV *
getClassHier(self)
	Agar::Object self
CODE:
	RETVAL = newSVpv(AGOBJECT_CLASS(self)->hier, 0);
OUTPUT:
	RETVAL

Uint
getUint(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetUint(self, name);
OUTPUT:
	RETVAL

void
setUint(self, name, val)
	Agar::Widget self
	const char * name
	Uint val
CODE:
	AG_SetUint(self, name, val);

int
getInt(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetInt(self, name);
OUTPUT:
	RETVAL

void
setInt(self, name, val)
	Agar::Widget self
	const char * name
	int val
CODE:
	AG_SetInt(self, name, val);

int
getBool(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetBool(self, name);
OUTPUT:
	RETVAL

void
setBool(self, name, val)
	Agar::Widget self
	const char * name
	int val
CODE:
	AG_SetBool(self, name, val);

Uint8
getUint8(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetUint8(self, name);
OUTPUT:
	RETVAL

void
setUint8(self, name, val)
	Agar::Widget self
	const char * name
	Uint8 val
CODE:
	AG_SetUint8(self, name, val);

Sint8
getSint8(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetSint8(self, name);
OUTPUT:
	RETVAL

void
setSint8(self, name, val)
	Agar::Widget self
	const char * name
	Sint8 val
CODE:
	AG_SetSint8(self, name, val);

Uint16
getUint16(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetUint16(self, name);
OUTPUT:
	RETVAL

void
setUint16(self, name, val)
	Agar::Widget self
	const char * name
	Uint16 val
CODE:
	AG_SetUint16(self, name, val);

Sint16
getSint16(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetSint16(self, name);
OUTPUT:
	RETVAL

void
setSint16(self, name, val)
	Agar::Widget self
	const char * name
	Sint16 val
CODE:
	AG_SetSint16(self, name, val);

Uint32
getUint32(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetUint32(self, name);
OUTPUT:
	RETVAL

void
setUint32(self, name, val)
	Agar::Widget self
	const char * name
	Uint32 val
CODE:
	AG_SetUint32(self, name, val);

Sint32
getSint32(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetSint32(self, name);
OUTPUT:
	RETVAL

void
setSint32(self, name, val)
	Agar::Widget self
	const char * name
	Sint32 val
CODE:
	AG_SetSint32(self, name, val);

float
getFloat(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetFloat(self, name);
OUTPUT:
	RETVAL

void
setFloat(self, name, val)
	Agar::Widget self
	const char * name
	float val
CODE:
	AG_SetFloat(self, name, val);

double
getDouble(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetDouble(self, name);
OUTPUT:
	RETVAL

void
setDouble(self, name, val)
	Agar::Widget self
	const char * name
	double val
CODE:
	AG_SetDouble(self, name, val);

SV *
getString(self, name)
	Agar::Widget self
	const char * name
CODE:
	if (AG_Defined(self, name)) {
		RETVAL = newSVpv(AG_GetStringDup(self, name), 0);
	} else {
		RETVAL = newSVpv(AG_Strdup(""), 0);
	}
OUTPUT:
	RETVAL

void
setString(self, name, val)
	Agar::Widget self
	const char * name
	const char * val
CODE:
	AG_SetString(self, name, val);

MODULE = Agar::Event	PACKAGE = Agar::Event	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

const char *
name(event)
	Agar::Event event
CODE:
	RETVAL = event->name;
OUTPUT:
	RETVAL

Agar::Object
receiver(event)
	Agar::Event event
CODE:
	RETVAL = AG_SELF();
OUTPUT:
	RETVAL

IV
ptr(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_POINTER;
CODE:
	index += AP_ARGS_MAX;
	if (index <= AP_ARGS_MAX || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = PTR2IV(AG_PTR(index));
OUTPUT:
	RETVAL

IV
ptrNamed(event, name)
	Agar::Event event
	const char *name
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_POINTER;
CODE:
	RETVAL = PTR2IV(AG_PTR_NAMED(name));
OUTPUT:
	RETVAL

Agar::Object
object(event, index, tag)
	Agar::Event event
	int index
	const char * tag
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_POINTER;
CODE:
	index += AP_ARGS_MAX;
	if (index <= AP_ARGS_MAX || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_OBJECT(index, tag);
OUTPUT:
	RETVAL

Agar::Object
object_named(event, name, hier)
	Agar::Event event
	const char * name
	const char * hier
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_POINTER;
CODE:
	RETVAL = AG_OBJECT_NAMED(name);
OUTPUT:
	RETVAL

SV *
string(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_STRING;
CODE:
	index += AP_ARGS_MAX;
	if (index <= AP_ARGS_MAX || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = newSVpv(AG_STRING(index), 0);
OUTPUT:
	RETVAL

int
int(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_INT;
CODE:
	index += AP_ARGS_MAX;
	if (index <= AP_ARGS_MAX || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_INT(index);
OUTPUT:
	RETVAL

Uint
Uint(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_UINT;
CODE:
	index += AP_ARGS_MAX;
	if (index <= AP_ARGS_MAX || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_UINT(index);
OUTPUT:
	RETVAL

long
long(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_SINT32;
CODE:
	index += AP_ARGS_MAX;
	if (index <= AP_ARGS_MAX || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_LONG(index);
OUTPUT:
	RETVAL

Ulong
Ulong(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_UINT32;
CODE:
	index += AP_ARGS_MAX;
	if (index <= AP_ARGS_MAX || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_ULONG(index);
OUTPUT:
	RETVAL

float
float(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_FLOAT;
CODE:
	index += AP_ARGS_MAX;
	if (index <= AP_ARGS_MAX || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_FLOAT(index);
OUTPUT:
	RETVAL

double
double(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_DOUBLE;
CODE:
	index += AP_ARGS_MAX;
	if (index <= AP_ARGS_MAX || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_DOUBLE(index);
OUTPUT:
	RETVAL


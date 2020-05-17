MODULE = Agar::Font	PACKAGE = Agar::Font	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Font
new(package, face, points)
	const char * package
	const char * face
	float points;
CODE:
	RETVAL = AG_FetchFont(face, points, 0);
OUTPUT:
	RETVAL

void
setDefault(font)
	Agar::Font font
CODE:
	agDefaultFont = font; /* could leak memory */

void
DESTROY(font)
	Agar::Font font
CODE:
	if (agDefaultFont != font) {
		AG_ObjectDestroy(font);
	}


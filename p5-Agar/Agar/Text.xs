MODULE = Agar::Text	PACKAGE = Agar::Text	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

void
PushState()
CODE:
	AG_PushTextState();

void
PopState()
CODE:
	AG_PopTextState();

void
Justify(mode)
	const char * mode
CODE:
	switch (mode[0]) {
		case 'l': case 'L': AG_TextJustify(AG_TEXT_LEFT); break;
		case 'r': case 'R': AG_TextJustify(AG_TEXT_RIGHT); break;
		case 'c': case 'C': AG_TextJustify(AG_TEXT_CENTER); break;
	}

void
Valign(mode)
	const char * mode
CODE:
	switch (mode[0]) {
		case 't': case 'T': AG_TextValign(AG_TEXT_TOP); break;
		case 'm': case 'M': AG_TextValign(AG_TEXT_MIDDLE); break;
		case 'b': case 'B': AG_TextValign(AG_TEXT_BOTTOM); break;
	}

void
ColorRGB(r, g, b)
	Uint8 r
	Uint8 g
	Uint8 b
CODE:
	AG_TextColorRGB(r, g, b);

void
ColorRGBA(r, g, b, a)
	Uint8 r
	Uint8 g
	Uint8 b
	Uint8 a
CODE:
	AG_TextColorRGBA(r, g, b, a);

void
BGColorRGB(r, g, b)
	Uint8 r
	Uint8 g
	Uint8 b
CODE:
	AG_TextBGColorRGB(r, g, b);

void
BGColorRGBA(r, g, b, a)
	Uint8 r
	Uint8 g
	Uint8 b
	Uint8 a
CODE:
	AG_TextBGColorRGBA(r, g, b, a);

void
SetFont(font)
	Agar::Font font
CODE:
	AG_TextFont(font);

int
Width(text)
	const char * text
CODE:
	AG_TextSize(text, &RETVAL, NULL);
OUTPUT:
	RETVAL

int
Height(text)
	const char * text
CODE:
	AG_TextSize(text, NULL, &RETVAL);
OUTPUT:
	RETVAL


/*	Public domain	*/

#ifndef _AGAR_GUI_TEXT_H_
#define _AGAR_GUI_TEXT_H_

#include <agar/gui/drv.h>
#include <agar/gui/font.h>

#include <agar/gui/begin.h>

/* Maximum height of AG_{Push,Pop}TextState() stack. */
#ifndef AG_TEXT_STATES_MAX
#define AG_TEXT_STATES_MAX (AG_MODEL >> 1)
#endif

/* Maximum length of AG_TextParseFontSpec() font specification strings. */
#ifndef AG_TEXT_FONTSPEC_MAX
#define AG_TEXT_FONTSPEC_MAX AG_MODEL
#endif

/*
 * Maximum length of ANSI control sequences. Longer sequences are ignored.
 * Note: Increasing this impacts the performance of backward ansi-scanning
 * routines such as AG_Editable's CursorLeft().
 */
#ifndef AG_TEXT_ANSI_SEQ_MAX
#define AG_TEXT_ANSI_SEQ_MAX 64
#endif

/* Maximum ANSI parameter bytes (for those sequences we care about). */
#ifndef AG_TEXT_ANSI_PARAM_MAX
#define AG_TEXT_ANSI_PARAM_MAX 32
#endif

/* Way to justify text horizontally in a space. */
enum ag_text_justify {
	AG_TEXT_LEFT,
	AG_TEXT_CENTER,
	AG_TEXT_RIGHT
};

/* Way to align text vertically in a space. */
enum ag_text_valign {
	AG_TEXT_TOP,
	AG_TEXT_MIDDLE,
	AG_TEXT_BOTTOM
};

/* Canned dialog types. */
enum ag_text_msg_title {
	AG_MSG_ERROR,
	AG_MSG_WARNING,
	AG_MSG_INFO
};

/* ANSI 3-bit and 4-bit colors. */
enum ag_ansi_color {
	AG_ANSI_BLACK,		/* fg=30 bg=40 */
	AG_ANSI_RED,
	AG_ANSI_GREEN,
	AG_ANSI_YELLOW,
	AG_ANSI_BLUE,
	AG_ANSI_MAGENTA,
	AG_ANSI_CYAN,
	AG_ANSI_WHITE,		/* fg=37 bg=47 */
	AG_ANSI_BRIGHT_BLACK,	/* fg=90 bg=100 */
	AG_ANSI_BRIGHT_RED,
	AG_ANSI_BRIGHT_GREEN,
	AG_ANSI_BRIGHT_YELLOW,
	AG_ANSI_BRIGHT_BLUE,
	AG_ANSI_BRIGHT_MAGENTA,
	AG_ANSI_BRIGHT_CYAN,
	AG_ANSI_BRIGHT_WHITE,	/* fg=97 bg=107 */
	AG_ANSI_COLOR_LAST
};

/* Raw ANSI escape codes. */
enum ag_text_ansi_control {
	AG_ANSI_SS2,			/* G2 character set (xterm) */
	AG_ANSI_SS3,			/* G3 character set (xterm) */
	AG_ANSI_DCS,			/* User-defined keys/termcap (xterm) */
	AG_ANSI_CSI_CUU,		/* Cursor Up */
	AG_ANSI_CSI_CUD,		/* Cursor Down */
	AG_ANSI_CSI_CUF,		/* Cursor Forward */
	AG_ANSI_CSI_CUB,		/* Cursor Back */
	AG_ANSI_CSI_CNL,		/* Cursor Next Line */
	AG_ANSI_CSI_CPL,		/* Cursor Prev Line */
	AG_ANSI_CSI_CHA,		/* Cursor Horizontal Absolute */
	AG_ANSI_CSI_CUP,		/* Cursor Position */
	AG_ANSI_CSI_ED,			/* Erase in Display */
	AG_ANSI_CSI_EL,			/* Erase in Line */
	AG_ANSI_CSI_SU,			/* Scroll Up */
	AG_ANSI_CSI_SD,			/* Scroll Down */
	AG_ANSI_CSI_SGR,		/* Select Graphic Rendition */
	AG_ANSI_CSI_DSR,		/* Device Status Report */
	AG_ANSI_CSI_SCP,		/* Save Cursor Position */
	AG_ANSI_CSI_RCP,		/* Restore Cursor Position */
	AG_ANSI_ST,			/* String Terminator */
	AG_ANSI_OSC,			/* Operating System Command */
	AG_ANSI_PM,			/* Ignore (ST-terminated) */
	AG_ANSI_APC,			/* Ignore (ST-terminated) */
	AG_ANSI_SOS,			/* Ignore (ST-terminated) */
	AG_ANSI_RIS,			/* Reset to initial state */
	AG_ANSI_PRIVATE,
	AG_ANSI_LAST
};
enum ag_text_sgr_parameter {
	AG_SGR_RESET           = 0,	/* Reset all attributes */
	AG_SGR_BOLD            = 1,	/* Bold / increase intensity */
	AG_SGR_FAINT           = 2,	/* Faint / decrease intensity */
	AG_SGR_ITALIC          = 3,	/* Italic (or sometimes inverse) */
	AG_SGR_UNDERLINE       = 4,	/* Single underline */
	AG_SGR_BLINK_SLOW      = 5,	/* Under 400ms */
	AG_SGR_BLINK_FAST      = 6,	/* Over 400ms */
	AG_SGR_REVERSE         = 7,	/* Reverse video */
	AG_SGR_CONCEAL         = 8,	/* Hidden */
	AG_SGR_CROSSED_OUT     = 9,	/* Marked for deletion */
	AG_SGR_PRI_FONT        = 10,	/* Switch to primary font */
	AG_SGR_ALT_FONT_1      = 11,	/* Switch to alt font #1 */
	AG_SGR_ALT_FONT_2      = 12,	/* Switch to alt font #2 */
	AG_SGR_ALT_FONT_3      = 13,	/* Switch to alt font #3 */
	AG_SGR_ALT_FONT_4      = 14,	/* Switch to alt font #4 */
	AG_SGR_ALT_FONT_5      = 15,	/* Switch to alt font #5 */
	AG_SGR_ALT_FONT_6      = 16,	/* Switch to alt font #6 */
	AG_SGR_ALT_FONT_7      = 17,	/* Switch to alt font #7 */
	AG_SGR_ALT_FONT_8      = 18,	/* Switch to alt font #8 */
	AG_SGR_ALT_FONT_9      = 19,	/* Switch to alt font #9 */
	AG_SGR_FRAKTUR         = 20,	/* Switch to Fraktur */
	AG_SGR_UNDERLINE_2     = 21,	/* Double underline (or bold off) */
	AG_SGR_NO_INTENSITY    = 22,	/* Neither bold nor faint */
	AG_SGR_NO_FONT_STYLE   = 23,	/* Not italic, not Fraktur */
	AG_SGR_NO_UNDERLINE    = 24,	/* No single or double underline */
	AG_SGR_NO_BLINK        = 25,	/* Blink off */
	AG_SGR_NO_INVERSE      = 27,	/* Inverse video off */
	AG_SGR_REVEAL          = 28,	/* Not concealed */
	AG_SGR_NOT_CROSSED_OUT = 29,	/* Not crossed-out */
	AG_SGR_FG1             = 30,	/* Set FG color #1 */
	AG_SGR_FG2             = 31,	/* Set FG color #2 */
	AG_SGR_FG3             = 32,	/* Set FG color #3 */
	AG_SGR_FG4             = 33,	/* Set FG color #4 */
	AG_SGR_FG5             = 34,	/* Set FG color #5 */
	AG_SGR_FG6             = 35,	/* Set FG color #6 */
	AG_SGR_FG7             = 36,	/* Set FG color #7 */
	AG_SGR_FG8             = 37,	/* Set FG color #8 */
	AG_SGR_FG              = 38,	/* Set FG color (256-color) */
	AG_SGR_NO_FG           = 39,	/* Set default FG color */
	AG_SGR_BG1             = 40,	/* Set BG color #1 */
	AG_SGR_BG2             = 41,	/* Set BG color #2 */
	AG_SGR_BG3             = 42,	/* Set BG color #3 */
	AG_SGR_BG4             = 43,	/* Set BG color #4 */
	AG_SGR_BG5             = 44,	/* Set BG color #5 */
	AG_SGR_BG6             = 45,	/* Set BG color #6 */
	AG_SGR_BG7             = 46,	/* Set BG color #7 */
	AG_SGR_BG8             = 47,	/* Set BG color #8 */
	AG_SGR_BG              = 48,	/* Set BG color (256-color) */
	AG_SGR_NO_BG           = 49,	/* Set default BG color */
	AG_SGR_FRAMED          = 51,	/* Square frame */
	AG_SGR_ENCIRCLED       = 52,	/* Circular frame */
	AG_SGR_OVERLINED       = 53,	/* Overlined */
	AG_SGR_NO_FRAMES       = 54,	/* Not framed or encircled */
	AG_SGR_NOT_OVERLINED   = 55,	/* Not overlined */
					/* (56-59 unused) */
	AG_SGR_NO_FG_NO_BG     = 56,	/* Both NO_FG and NO_BG (internal) */
	AG_SGR_IDEOGRAM_1      = 60,    /* Ideogram underline or right side line */
	AG_SGR_IDEOGRAM_2      = 61,    /* Ideogram double underline, or
	                                   double line on the right side */
	AG_SGR_IDEOGRAM_3      = 62,    /* Ideogram overline or left side line */
	AG_SGR_IDEOGRAM_4      = 63,    /* Ideogram double overline, or
	                                   double line on the left side */
	AG_SGR_IDEOGRAM_5      = 64,    /* Ideogram stress marking */
	AG_SGR_IDEOGRAM_6      = 65,    /* No Ideogram attributes */
	AG_SGR_ALT_FONT_11     = 66,    /* Switch to alt font #11 (Agar ext) */
	AG_SGR_ALT_FONT_12     = 67,    /* Switch to alt font #12 (Agar ext) */
	AG_SGR_ALT_FONT_13     = 68,    /* Switch to alt font #13 (Agar ext) */
	AG_SGR_ALT_FONT_14     = 69,    /* Switch to alt font #14 (Agar ext) */
	AG_SGR_ALT_FONT_15     = 70,    /* Switch to alt font #15 (Agar ext) */
	AG_SGR_ALT_FONT_16     = 71,    /* Switch to alt font #16 (Agar ext) */
	AG_SGR_ALT_FONT_17     = 72,    /* Switch to alt font #17 (Agar ext) */
	AG_SGR_SUPERSCRIPT     = 73,    /* Superscript */
	AG_SGR_SUBSCRIPT       = 74,    /* Subscript */
	AG_SGR_NO_SUPSUBSCRIPT = 75,    /* No superscript or subscript */
	AG_SGR_BRIGHT_FG_1     = 90,
	AG_SGR_BRIGHT_FG_2     = 91,
	AG_SGR_BRIGHT_FG_3     = 92,
	AG_SGR_BRIGHT_FG_4     = 93,
	AG_SGR_BRIGHT_FG_5     = 94,
	AG_SGR_BRIGHT_FG_6     = 95,
	AG_SGR_BRIGHT_FG_7     = 96,
	AG_SGR_BRIGHT_FG_8     = 97,
					/* (98-99 unused) */
	AG_SGR_BRIGHT_BG_1     = 100,
	AG_SGR_BRIGHT_BG_2     = 101,
	AG_SGR_BRIGHT_BG_3     = 102,
	AG_SGR_BRIGHT_BG_4     = 103,
	AG_SGR_BRIGHT_BG_5     = 104,
	AG_SGR_BRIGHT_BG_6     = 105,
	AG_SGR_BRIGHT_BG_7     = 106,
	AG_SGR_BRIGHT_BG_8     = 107,
	AG_SGR_LAST            = 108
};

/* Normalized ANSI escape code. */
typedef struct ag_text_ansi {
	enum ag_text_ansi_control ctrl;   /* Type of control sequence */
	Uint len;                         /* Number of chars in sequence */
	int  n;                           /* Parameter count */
	enum ag_text_sgr_parameter sgr;   /* SGR parameter */
	AG_Color color;	                  /* For BG_COLOR / FG_COLOR */
} AG_TextANSI;

/*
 * An element of the rendering attribute stack.
 * Sync with CompareTextStates() in gui/text_cache.c.
 */
typedef struct ag_text_state {
	AG_Font *_Nonnull font;		/* Font face */
	AG_Color color;			/* Foreground text color */
	AG_Color colorBG;		/* Background color */
	AG_Color *_Nullable colorANSI;	/* ANSI color palette (3/4-bit mode) */
	enum ag_text_justify justify;	/* Justification mode */
	enum ag_text_valign valign;	/* Vertical alignment mode */
	int tabWd;			/* Width of \t in pixels */
	Uint32 _pad;
} AG_TextState;

/* Measures of rendered text. */
typedef struct ag_text_metrics {
	int w, h;			/* Dimensions in pixels */
	Uint *_Nullable wLines;		/* Width of each line */
	Uint            nLines;		/* Total line count */
	Uint32 _pad;
} AG_TextMetrics;

#ifdef AG_DEBUG
# define AG_TEXT_STATE_CUR() \
  (((agTextStateCur >= 0 && \
    agTextStateCur < AG_TEXT_STATES_MAX)) ? &agTextStateStack[agTextStateCur] : \
   (AG_TextState *)AG_GenericMismatch("AG_TEXT_STATE"))
#else
# define AG_TEXT_STATE_CUR() (&agTextStateStack[agTextStateCur])
#endif

__BEGIN_DECLS
/* text.c */
extern int agTextFontHeight;    /* Default font height (px) */
extern int agTextFontAscent;    /* Default font ascent (px) */
extern int agTextFontLineSkip;  /* Default font y-increment (px) */
extern int agFontconfigInited;  /* Fontconfig library is initialized */

extern _Nonnull_Mutex AG_Mutex agTextLock;
extern AG_FontQ                agFontCache;
extern AG_TextState            agTextStateStack[AG_TEXT_STATES_MAX];
extern int                     agTextStateCur;
extern AG_Font *_Nullable      agDefaultFont;

extern const AG_FontAdjustment agFontAdjustments[];

extern const char *agCoreFonts[];
extern const char *agTextMsgTitles[];
extern const char *agTextJustifyNames[];
extern const char *agTextValignNames[];

void               AG_PushTextState(void);
void               AG_CopyTextState(AG_TextState *);
void               AG_TextColorANSI(enum ag_ansi_color, const AG_Color *_Nonnull);
AG_Font *_Nullable AG_TextFontLookup(const char *_Nullable, float, Uint);
AG_Font *_Nullable AG_TextFontPts(float);
AG_Font *_Nullable AG_TextFontPct(int);
AG_Font *_Nullable AG_TextFontPctFlags(int, Uint);
void               AG_PopTextState(void);
void               AG_TextClearGlyphCache(AG_Driver *_Nonnull);

void AG_TextSize(const char *_Nullable, int *_Nullable, int *_Nullable);
void AG_TextSizeMulti(const char *_Nonnull, int *_Nonnull, int *_Nonnull,
                      Uint *_Nullable *_Nonnull, Uint *_Nullable);

void AG_TextSizeInternal(const AG_Char *_Nullable, int *_Nullable, int *_Nullable);
void AG_TextSizeMultiInternal(const AG_Char *_Nonnull, int *_Nullable,
                              int *_Nullable, Uint *_Nullable *_Nonnull, Uint *_Nonnull);
#ifdef AG_UNICODE
#define AG_TextSizeUCS4(s,w,h)            AG_TextSizeInternal((s),(w),(h))
#define AG_TextSizeMultiUCS4(s,w,h,wL,nL) AG_TextSizeMultiInternal((s),(w),(h),(wL),(nL))
#endif

void AG_TextMsgS(enum ag_text_msg_title, const char *_Nonnull);
void AG_TextMsg(enum ag_text_msg_title, const char *_Nonnull, ...)
               FORMAT_ATTRIBUTE(printf,2,3);
void AG_TextMsgFromError(void);

void AG_TextTmsgS(enum ag_text_msg_title, Uint32, const char *_Nonnull);
void AG_TextTmsg(enum ag_text_msg_title, Uint32, const char *_Nonnull, ...)
                FORMAT_ATTRIBUTE(printf,3,4);

void AG_TextInfoS(const char *_Nullable, const char *_Nonnull);
void AG_TextInfo(const char *_Nullable, const char *_Nonnull, ...)
                FORMAT_ATTRIBUTE(printf,2,3);

void AG_TextWarningS(const char *_Nonnull, const char *_Nonnull);
void AG_TextWarning(const char *_Nonnull, const char *_Nonnull, ...)
                   FORMAT_ATTRIBUTE(printf,2,3);
void AG_TextErrorS(const char *_Nonnull);
void AG_TextError(const char *_Nonnull, ...)
                 FORMAT_ATTRIBUTE(printf,1,2);

void AG_TextAlign(int *_Nonnull, int *_Nonnull, int,int, int,int,
                  int,int,int, int, enum ag_text_justify, enum ag_text_valign);

AG_Surface *_Nonnull AG_TextRenderF(const char *_Nonnull, ...) _Warn_Unused_Result;
AG_Surface *_Nonnull AG_TextRender(const char *_Nonnull) _Warn_Unused_Result;
AG_Surface *_Nonnull AG_TextRenderRTL(const char *_Nonnull) _Warn_Unused_Result;
AG_Surface *_Nonnull AG_TextRenderInternal(const AG_Char *_Nonnull, AG_Font *_Nonnull,
					   const AG_Color *_Nonnull,
					   const AG_Color *_Nonnull)
                                          _Warn_Unused_Result;
#ifdef AG_UNICODE
# define AG_TextRenderUCS4(s) AG_TextRenderInternal((s), AG_TEXT_STATE_CUR()->font, \
                                                        &AG_TEXT_STATE_CUR()->colorBG, \
                                                        &AG_TEXT_STATE_CUR()->color)
#endif

AG_Glyph *_Nonnull AG_TextRenderGlyph(AG_Driver *_Nonnull, AG_Font *_Nonnull,
                                      const AG_Color *_Nonnull, const AG_Color *_Nonnull,
				      AG_Char)
                                     _Warn_Unused_Result;

int  AG_TextParseANSI(const AG_TextState *_Nonnull, AG_TextANSI *_Nonnull,
                      const AG_Char *_Nonnull);

int  AG_TextExportUnicode_StripANSI(const char *, char *, const AG_Char *, AG_Size);

void AG_TextColor(const AG_Color *_Nonnull);
void AG_TextColorRGB(Uint8, Uint8, Uint8);
void AG_TextColorRGBA(Uint8, Uint8, Uint8, Uint8);
void AG_TextColorHex(Uint32);
void AG_TextBGColor(const AG_Color *_Nonnull);
void AG_TextBGColorRGB(Uint8, Uint8, Uint8);
void AG_TextBGColorRGBA(Uint8, Uint8, Uint8, Uint8);
void AG_TextBGColorHex(Uint32);
void AG_TextFont(AG_Font *_Nonnull);
void AG_TextJustify(enum ag_text_justify);
void AG_TextValign(enum ag_text_valign);
void AG_TextTabWidth(int);

AG_Font *AG_SetDefaultFont(AG_Font *_Nullable);
void AG_TextParseFontSpec(const char *_Nullable);

int  AG_InitTextSubsystem(void);
void AG_DestroyTextSubsystem(void);

/*
 * Test whether the given character should be considered a space
 * (for word wrapping and word selection).
 */
static __inline__ _Const_Attribute int
AG_CharIsSpace(AG_Char c)
{
	switch (c) {
	case ' ':		/* SPACE */
	case '\t':		/* TAB */
		return (1);
#ifdef AG_UNICODE
	case 0x00a0:		/* NO-BREAK SPACE */
	case 0x1680:		/* OGHAM SPACE MARK */
	case 0x180e:		/* MONGOLIAN VOWEL SEPARATOR */
	case 0x202f:		/* NARROW NO-BREAK SPACE */
	case 0x205f:		/* MEDIUM MATHEMATICAL SPACE */
	case 0x3000:		/* IDEOGRAPHIC SPACE */
	case 0xfeff:		/* ZERO WIDTH NO-BREAK SPACE */
		return (1);
#endif
	}
#ifdef AG_UNICODE
	if (c >= 0x2000 && c <= 0x200b)	/* EN/EM SPACES */
		return (1);
#endif
	return (0);
}

/*
 * Test whether the given character should be considered punctuation
 * (for word selection).
 */
static __inline__ _Const_Attribute int
AG_CharIsPunct(AG_Char c)
{
	switch (c) {
	case '!':
	case '"':
	case '#':
	case '$':
	case '%':
	case '&':
	case '\'':
	case '(':
	case ')':
	case '*':
	case '+':
	case ',':
	case '-':
	case '.':
	case '/':
	case ':':
	case ';':
	case '<':
	case '=':
	case '>':
	case '?':
	case '@':
	case '[':
	case '\\':
	case ']':
	case '^':
	case '_':
	case '`':
	case '{':
	case '|':
	case '}':
	case '~':
		return (1);
#ifdef AG_UNICODE
	case 0x037e:		/* GREEK QUESTION MARK */
	case 0x0589:		/* ARMENIAN FULL STOP */
	case 0x058a:		/* ARMENIAN HYPHEN */
	case 0x061b:		/* ARABIC SEMICOLON */
	case 0x061f:		/* ARABIC QUESTION MARK */
	case 0x07f8:		/* NKO COMMA */
	case 0x07f9:		/* NKO EXCLAMATION MARK */
	case 0x10fb:		/* GEORGIAN PARAGRAPH SEPARATOR */
	case 0x2e18:		/* INVERTED INTERROBANG */
	case 0x2e2e:		/* REVERSED QUESTION MARK */
	case 0xa4fe:		/* LISU PUNCTUATION COMMA */
	case 0xa4ff:		/* LISU PUNCTUATION FULL STOP */
		return (1);
#endif
	}
#ifdef AG_UNICODE
	if ((c >= 0x00a1 &&	/* INVERTED EXCLAMATION MARK- */
	     c <= 0x00bf) ||	/* INVERTED QUESTION MARK. */
	    (c >= 0x055a &&	/* ARMENIAN APOSTROPHE- */
	     c <= 0x055f) ||	/* ARMENIAN ABBREVIATION MARK. */
	    (c >= 0x2010 &&	/* HYPHEN- */
	     c <= 0x205e) ||	/* VERTICAL FOUR DOTS. */
	    (c >= 0x3008 &&	/* LEFT ANGLE BRACKET- */
	     c <= 0x301f))	/* LOW DOUBLE PRIME QUOTATION MARK. */
		return (1);
#endif
	return (0);
}

#define AG_CharIsSpaceOrPunct(ch) (AG_CharIsSpace(ch) || AG_CharIsPunct(ch))
#define AG_CharIsSpaceOrLF(ch) (AG_CharIsSpace(ch) || (ch == '\n'))
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_GUI_TEXT_H_ */

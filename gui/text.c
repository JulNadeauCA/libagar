/*
 * Copyright (c) 2001-2020 Julien Nadeau Carriere <vedge@csoft.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* TextSizeFT() and TextRenderFT() are based on code from SDL_ttf : */
/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002  Sam Lantinga
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author, nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Agar font engine. We handle high-level rendering and sizing of UTF-8 text
 * (internally converted to UCS-4) using either FreeType or an internal bitmap
 * font engine (for platforms without FreeType).
 */

#include <agar/config/have_freetype.h>
#include <agar/config/have_fontconfig.h>
#include <agar/config/ttfdir.h>

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/core/win32.h>

#ifdef HAVE_FREETYPE
# include <agar/gui/ttf.h>
#endif
#ifdef HAVE_FONTCONFIG
# include <fontconfig/fontconfig.h>
#endif

#include <agar/gui/text.h>
#include <agar/gui/window.h>
#ifdef AG_WIDGETS
# include <agar/gui/box.h>
# include <agar/gui/label.h>
# include <agar/gui/textbox.h>
# include <agar/gui/button.h>
# include <agar/gui/ucombo.h>
# include <agar/gui/numerical.h>
# include <agar/gui/checkbox.h>
#endif
#include <agar/gui/keymap.h>
#include <agar/gui/load_xcf.h>
#include <agar/gui/iconmgr.h>
#include <agar/gui/icons.h>
#include <agar/gui/fonts.h>
#include <agar/gui/packedpixel.h>
#include <agar/gui/gui_math.h>

#include <agar/gui/fonts_data.h>

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

/* #define DEBUG_FONTS */
/* #define DEBUG_ANSI */

/* Default fonts */
const char *agDefaultFaceFT = "_agFontAlgue";
const char *agDefaultFaceBitmap = "minimal.xcf";

/* Statically compiled fonts */
AG_StaticFont *agBuiltinFonts[] = {
	&agFontAlgue,
	&agFontAlgue_Bold,
	&agFontAlgue_Italic,
	&agFontAlgue_BoldItalic,
	NULL
};

int agTextFontHeight = 0;		/* Default font height (px) */
int agTextFontAscent = 0;		/* Default font ascent (px) */
int agTextFontDescent = 0;		/* Default font descent (px) */
int agTextFontLineSkip = 0;		/* Default font line skip (px) */
int agFreetypeInited = 0;		/* Initialized Freetype library */
int agFontconfigInited = 0;		/* Initialized Fontconfig library */

_Nonnull_Mutex AG_Mutex agTextLock;

AG_FontQ agFontCache;                                        /* Loaded fonts */

AG_TextState agTextStateStack[AG_TEXT_STATES_MAX];
Uint         agTextStateCur = 0;

/*
 * Small adjustments to the sizes and ascents of core fonts.
 * Indicate where Bold is also the Regular style.
 */
const AG_FontAdjustment agFontAdjustments[] = {
/*                                                 0.0 10.4 14.0 21.0 23.8 35.0 to- */
/*                                                10.4 14.0 21.0 23.8 35.0 inf  pts */
/*	{ "bedstead",        AG_FONT_BOLD, 0.9f, { +1,  +3,  +3,  +4,  +5,  +7 } }, */
	{ "cm-sans",         0,            1.1f, { -4,  -4,  -6,  -7,  -9, -16 } },
	{ "cm-serif",        0,            1.1f, { -3,  -4,  -5,  -5,  -6, -10 } },
	{ "cm-typewriter",   0,            1.1f, { -2,  -4,  -5,  -5,  -7,  -8 } },
	{ "league-spartan",  AG_FONT_BOLD, 0.9f, { -1,  -1,  -3,  -4,  -5,  -7 } },
	{ "league-gothic",   AG_FONT_BOLD, 1.1f, { -1,  -1,  -1,  -2,  -1,  -3 } },
	{ "fraktur",         AG_FONT_BOLD, 1.1f, { +1,  +1,  +1,  +1,  +1,  +1 } },
	{ "source-han-sans", 0,            1.0f, { -8, -12, -15, -20, -28, -35 } },
	{ "unialgue",        0,            1.0f, { -3,  -4,  -5,  -7, -11, -12 } },
	{ NULL,              0,            0.0f, {  0,   0,   0,   0,   0,   0 } }
};

/* ANSI color scheme (may be overridden by AG_TextState) */
AG_Color agTextColorANSI[] = {
#if AG_MODEL == AG_LARGE
	{ 0x0000, 0x0000, 0x0000, 0xffff },	/* Black */
	{ 0xdede, 0x2d2d, 0x2b2b, 0xffff },	/* Red */
	{ 0x0000, 0xcdcd, 0x0000, 0xffff },	/* Green */
	{ 0xcdcd, 0xcdcd, 0x0000, 0xffff },	/* Yellow */
	{ 0x0000, 0x0000, 0xeeee, 0xffff },	/* Blue */
	{ 0xcdcd, 0x0000, 0xcdcd, 0xffff },	/* Magenta */
	{ 0x0000, 0xcdcd, 0xcdcd, 0xffff },	/* Cyan */
	{ 0xe5e5, 0xe5e5, 0xe5e5, 0xffff },	/* White */
	{ 0x7f7f, 0x7f7f, 0x7f7f, 0xffff },	/* Br.Black */
	{ 0xffff, 0x1717, 0x0000, 0xffff },	/* Br.Red */
	{ 0x0000, 0xffff, 0x0000, 0xffff },	/* Br.Green */
	{ 0xffff, 0xffff, 0x0000, 0xffff },	/* Br.Yellow */
	{ 0x5c5c, 0x5c5c, 0xffff, 0xffff },	/* Br.Blue */
	{ 0xffff, 0x0000, 0xffff, 0xffff },	/* Br.Magenta */
	{ 0x0000, 0xffff, 0xffff, 0xffff },	/* Br.Cyan */
	{ 0xffff, 0xffff, 0xffff, 0xffff },	/* Br.White */
#else
	{ 0,   0,     0, 255 },			/* Black */
	{ 205, 45,    0, 255 },			/* Red */
	{ 0,   205,   0, 255 },			/* Green */
	{ 205, 205,   0, 255 },			/* Yellow */
	{ 0,   0,   238, 255 },			/* Blue */
	{ 205, 0,   205, 255 },			/* Magenta */
	{ 0,   205, 205, 255 },			/* Cyan */
	{ 229, 229, 229, 255 },			/* White */
	{ 127, 127, 127, 255 },			/* Br.Black */
	{ 255, 23,    0, 255 },			/* Br.Red */
	{ 0,   255,   0, 255 },			/* Br.Green */
	{ 255, 255,   0, 255 },			/* Br.Yellow */
	{ 92,  92,  255, 255 },			/* Br.Blue */
	{ 255, 0,   255, 255 },			/* Br.Magenta */
	{ 0,   255, 255, 255 },			/* Br.Cyan */
	{ 255, 255, 255, 255 },			/* Br.White */
#endif
};	

AG_Font *_Nullable agDefaultFont = NULL;

const char *agFontTypeNames[] = {		/* For enum ag_font_type */
	N_("Vector"),
	N_("Bitmap"),
	N_("Dummy")
};
const char *agCoreFonts[] = {
	"unialgue",                             /* Unialgue (not RFN) */
	"",                                     /* (unused) */
	"cm-sans",                              /* CMU Sans */
	"cm-serif",                             /* CMU Serif */
	"cm-typewriter",                        /* CMU Typewriter */
	"charter",                              /* Charter */
	"courier-prime",                        /* Courier Prime */
	"source-han-sans",                      /* Source Han Sans */
	"league-spartan",                       /* League Spartan */
	"league-gothic",                        /* League Gothic */
	"fraktur",                              /* Unifraktur Maguntia */
	NULL
};
const char *agFontFileExts[] = {                /* Recognized font extensions */
	".otf", ".ttf", ".ttc",
	".woff2", ".woff",
	".dfont", ".fnt",
	".xcf", ".bmp", ".png",
	NULL
};

const char *agTextMsgTitles[] = {		/* For enum ag_text_msg_title */
	N_("Error"),
	N_("Warning"),
	N_("Information"),
	NULL
};
const char *agTextJustifyNames[] = {            /* For enum ag_text_justify */
	N_("Left"),
	N_("Center"),
	N_("Right"),
	NULL
};
const char *agTextValignNames[] = {             /* For enum ag_text_valign */
	N_("Top"),
	N_("Middle"),
	N_("Bottom"),
	NULL
};

static int agTextInitedSubsystem = 0;

#ifdef HAVE_FREETYPE
static void TextRenderFT(const AG_Char *_Nonnull, AG_Surface *_Nonnull, const AG_TextMetrics *_Nonnull, AG_Font *_Nonnull, const AG_Color *_Nonnull, const AG_Color *_Nonnull);
static void TextRenderFT_Underline(AG_TTFFont *_Nonnull, AG_Surface *_Nonnull, int);
#endif
static AG_Glyph *_Nonnull TextRenderGlyph_Miss(AG_Driver *_Nonnull, AG_Font *_Nonnull, const AG_Color *_Nonnull, const AG_Color *_Nonnull, AG_Char);
static AG_Surface *_Nonnull GetBitmapGlyph(const AG_Font *_Nonnull, AG_Char);

#if !defined(HAVE_FREETYPE)
# define TextSizeFT       TextSizeDummy
# define TextRenderFT     TextRenderDummy
#endif

#ifdef DEBUG_FOCUS
# define Debug_Focus AG_Debug
#else
# if defined(__GNUC__)
#  define Debug_Focus(obj, arg...) ((void)0)
# elif defined(__CC65__)
#  define Debug_Focus
# else
#  define Debug_Focus AG_Debug
# endif
#endif /* DEBUG_FOCUS */

/* Load an individual glyph from a bitmap font file. */
static void
LoadBitmapGlyph(AG_Surface *_Nonnull S, const char *_Nonnull lbl,
    void *_Nonnull pFont)
{
	AG_Font *font = pFont;

	if (font->data.bmp.nGlyphs == 0) {   /* Expect "MAP:x-y" label first */
		Strlcpy(font->data.bmp.spec, lbl, AG_FONT_BITMAP_SPEC_MAX);
	}
	font->data.bmp.glyphs = Realloc(font->data.bmp.glyphs,
	    (font->data.bmp.nGlyphs + 1) * sizeof(AG_Surface *));
	font->data.bmp.glyphs[font->data.bmp.nGlyphs++] = S;
}

/* Load a bitmap font. */
static int
BitmapOpenFont(AG_Font *_Nonnull font, const char *path)
{
	char *s, *msig, *c0, *c1;
	AG_DataSource *ds;

	switch (font->spec.sourceType) {
	case AG_FONT_SOURCE_FILE:
		ds = AG_OpenFile(path, "rb");
		break;
	case AG_FONT_SOURCE_MEMORY:
		ds = AG_OpenConstCore(font->spec.source.mem.data,
		                      font->spec.source.mem.size);
		break;
	default:
		ds = NULL;
		break;
	}
	if (ds == NULL)
		return (-1);

	if ((font->data.bmp.glyphs = TryMalloc(sizeof(AG_Surface *))) == NULL) {
		AG_CloseDataSource(ds);
		return (-1);
	}
	font->data.bmp.nGlyphs = 0;
	font->data.bmp.spec = Malloc(AG_FONT_BITMAP_SPEC_MAX);
	font->data.bmp.spec[0] = '\0';

	/* Load the glyphs from the XCF layers. */
	if (AG_XCFLoad(ds, 0, LoadBitmapGlyph, font) == -1) {
		AG_CloseDataSource(ds);
		goto fail;
	}
	AG_CloseDataSource(ds);

	/* Get the range of characters from the "MAP:x-y" string. */
	s = font->data.bmp.spec;
	msig = AG_Strsep(&s, ":");
	c0 = AG_Strsep(&s, "-");
	c1 = AG_Strsep(&s, "-");
	if (font->data.bmp.nGlyphs < 1 ||
	    msig == NULL || strcmp(msig, "MAP") != 0 ||
	    c0 == NULL || c1 == NULL ||
	    c0[0] == '\0' || c1[0] == '\0') {
		AG_SetErrorS("XCF is missing \"MAP:x-y\" label");
		goto fail;
	}
	font->data.bmp.c0 = (AG_Char)strtol(c0, NULL, 10);
	font->data.bmp.c1 = (AG_Char)strtol(c1, NULL, 10);
	if (font->data.bmp.nGlyphs < (font->data.bmp.c1 - font->data.bmp.c0)) {
		AG_SetErrorS("XCF has inconsistent bitmap fontspec");
		goto fail;
	}
	font->height = font->data.bmp.glyphs[0]->h;
	font->ascent = font->height;
	font->descent = 0;
	font->lineskip = font->height;

	free(font->data.bmp.spec);
	font->data.bmp.spec = NULL;
	return (0);
fail:
	free(font->data.bmp.spec);
	font->data.bmp.spec = NULL;
	return (-1);
}

/*
 * Save the current font-engine rendering state.
 *
 * Must be called from Widget draw(), size_alloc(), size_request() or
 * event-handling context. Widget must have the USE_TEXT flag set.
 */
void
AG_PushTextState(void)
{
	const AG_TextState *tsPrev = AG_TEXT_STATE_CUR();
	AG_TextState *ts;

	if ((agTextStateCur+1) >= AG_TEXT_STATES_MAX) {
#ifdef AG_VERBOSITY
		AG_FatalErrorF("PushTextState Overflow (%d+1 >= %d)",
		    agTextStateCur, AG_TEXT_STATES_MAX);
#else
		AG_FatalError("PushTextState Overflow");
#endif
	}
	ts = &agTextStateStack[++agTextStateCur];
	memcpy(ts, tsPrev, sizeof(AG_TextState));
	if (tsPrev->colorANSI != NULL) {
		const AG_Size size = AG_ANSI_COLOR_LAST*sizeof(AG_Color);
		ts->colorANSI = Malloc(size);
		memcpy(ts->colorANSI, tsPrev->colorANSI, size);
	}
	ts->font = AG_FetchFont(OBJECT(tsPrev->font)->name,
	                        tsPrev->font->spec.size,
		                tsPrev->font->flags);
#ifdef AG_DEBUG
	Snprintf(ts->name, sizeof(ts->name), "TS%d", agTextStateCur);
#endif
}

/* Create a copy of the current text state in dst. */
void
AG_CopyTextState(AG_TextState *dst)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();

	memcpy(dst, ts, sizeof(AG_TextState));
}

/*
 * Override an entry in the 16-color (4-bit) ANSI color palette.
 * Duplicate the default palette into the current state if necessary.
 */
void
AG_TextColorANSI(enum ag_ansi_color colorANSI, const AG_Color *c)
{
	AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_Color *tgt;

	if (colorANSI >= AG_ANSI_COLOR_LAST) {
		AG_FatalError("colorANSI");
	}
	if (ts->colorANSI == NULL) {
		const AG_Size size = AG_ANSI_COLOR_LAST * sizeof(AG_Color);

		ts->colorANSI = Malloc(size);
		memcpy(ts->colorANSI, agTextColorANSI, size);
	}
	tgt = &ts->colorANSI[colorANSI];
	memcpy(tgt, c, sizeof(AG_Color));
}

/*
 * Select the font face to use in rendering text.
 *
 * Must be called from Widget draw(), size_alloc(), size_request() or
 * event-handling context. Widget must have the USE_TEXT flag set.
 */
AG_Font *
AG_TextFontLookup(const char *face, float size, Uint flags)
{
	AG_Font *newFont;
	AG_TextState *ts;

	if ((newFont = AG_FetchFont(face, size, flags)) == NULL) {
		return (NULL);
	}
	ts = AG_TEXT_STATE_CUR();
	ts->font = newFont;
	return (newFont);
}

/*
 * Set font size in points.
 *
 * Must be called from Widget draw(), size_alloc(), size_request() or
 * event-handling context. Widget must have the USE_TEXT flag set.
 */
AG_Font *
AG_TextFontPts(float size)
{
	AG_TextState *ts = AG_TEXT_STATE_CUR();
	const AG_Font *fontCur = ts->font;
	AG_Font *font;

	if (!(font = AG_FetchFont(OBJECT(fontCur)->name, size, fontCur->flags))) {
		return (NULL);
	}
	ts->font = font;
	return (font);
}

/*
 * Set font size as % of the current font size.
 *
 * Must be called from Widget draw(), size_alloc(), size_request() or
 * event-handling context. Widget must have the USE_TEXT flag set.
 */
AG_Font *
AG_TextFontPct(int pct)
{
	AG_TextState *ts = AG_TEXT_STATE_CUR();
	const AG_Font *font = ts->font;

	return AG_FetchFont(OBJECT(font)->name,
	                    font->spec.size * pct / 100.0f,
	                    font->flags);
}
AG_Font *
AG_TextFontPctFlags(int pct, Uint flags)
{
	AG_TextState *ts = AG_TEXT_STATE_CUR();
	const AG_Font *font = ts->font;

	return AG_FetchFont(OBJECT(font)->name,
	                    font->spec.size * pct / 100.0f,
	                    flags);
}

/*
 * Load the given font (or return a pointer to an existing one), from
 * a specified font face, size (in points), and option flags.
 *
 * If face=NULL or fontSize=0, use agConfig `font.face' and `font.size'.
 *
 * For fontconfig-managed fonts, face is case-insensitive. For Agar core fonts
 * (and user-installed fonts), face is case-sensitive and corresponds to a file
 * in agConfig `font-path' (e.g., "fraktur" loads fraktur.ttf).
 *
 * A face with leading underscore (e.g., "_agFontAlgue") specifies a built-in
 * font which will be loaded from memory.
 */
AG_Font *
AG_FetchFont(const char *face, float fontSize, Uint flags)
{
	char fontPath[AG_PATHNAME_MAX];
	char name[AG_OBJECT_NAME_MAX], nameBase[AG_OBJECT_NAME_MAX];
	AG_Config *cfg = AG_ConfigObject();
	AG_StaticFont *builtin = NULL;
	AG_Font *font;
	AG_FontSpec *spec;
	const AG_FontAdjustment *fa;
	int isInFontPath;

	if (face == NULL) {
		AG_GetString(cfg, "font.face", name, sizeof(name));
	} else {
		if (face[0] != '_') {          /* Convert name to lowercase */
			const char *pFace;
			char *pDst;

			for (pFace=face, pDst=name;
			    *pFace != '\0' && pDst < &name[sizeof(name)-1];
			     pFace++) {
				*pDst = tolower(*pFace);
				pDst++;
			}
			*pDst = '\0';
		} else {
			Strlcpy(name, face, sizeof(name));
		}
	}

	if (fontSize < AG_FONT_PTS_EPSILON) {
		fontSize = (float)AG_GetInt(cfg, "font.size");
	}
	for (fa = &agFontAdjustments[0]; fa->face != NULL; fa++) {
		if (strcmp(name, fa->face) == 0) {
			fontSize *= fa->size_factor;
			break;
		}
	}
	
	Strlcpy(nameBase, name, sizeof(nameBase));

	AG_MutexLock(&agTextLock);

	TAILQ_FOREACH(font, &agFontCache, fonts) {
		if (Strcasecmp(OBJECT(font)->name, name) == 0 &&
		    (font->flags == flags) &&
		    Fabs(font->spec.size - fontSize) < AG_FONT_PTS_EPSILON) {
			break;
		}
	}
	if (font != NULL)
		goto out;

#ifdef DEBUG_FONTS
	Debug(NULL, "FetchFont(\"%s\" [=> %s],%.02f,0x%x)\n",
	    (face != NULL) ? face : "<default>", name, fontSize, flags);
#endif
	if ((font = TryMalloc(sizeof(AG_Font))) == NULL) {
		AG_MutexUnlock(&agTextLock);
		return (NULL);
	}
	AG_ObjectInit(font, &agFontClass);
	AG_ObjectSetNameS(font, name);
	spec = &font->spec;
	spec->size = fontSize;
	font->flags = flags;

	if (name[0] == '_') {                                   /* Built-in */
		AG_StaticFont **pbf;
	
		if ((flags & AG_FONT_BOLD) && (flags & AG_FONT_ITALIC)) {
			Strlcat(name, "_BoldItalic", sizeof(name));
		} else if (flags & AG_FONT_BOLD) {
			Strlcat(name, "_Bold", sizeof(name));
		} else if (flags & AG_FONT_ITALIC) {
			Strlcat(name, "_Italic", sizeof(name));
		}
		for (pbf = &agBuiltinFonts[0]; *pbf != NULL; pbf++) {
			if (strcmp((*pbf)->name, &name[1]) == 0) {
				builtin = *pbf;
				break;
			}
		}
		if (builtin == NULL) {
			AG_SetError(_("No such builtin font: %s"), name);
			goto fail;
		}
		spec->type = builtin->type;
		spec->sourceType = AG_FONT_SOURCE_MEMORY;
		spec->source.mem.data = builtin->data;
		spec->source.mem.size = builtin->size;
		goto init_font;
	}

	spec->type = AG_FONT_VECTOR;
	spec->sourceType = AG_FONT_SOURCE_FILE;
	fontPath[0] = '\0';
	isInFontPath = 0;

	if (AG_ConfigFind(AG_CONFIG_PATH_FONTS, name,   /* Exact file match */
	    fontPath, sizeof(fontPath)) == 0) {
		const char *pExt = strrchr(fontPath, '.');
		/*
		 * Non-FreeType bitmap fonts must be referred to by their
		 * full filename and extension.
		 */
		if (pExt && (Strcasecmp(pExt,".bmp") == 0 ||
		             Strcasecmp(pExt,".png") == 0 ||
		             Strcasecmp(pExt,".xcf") == 0)) {
			spec->type = AG_FONT_BITMAP;
		} else {
			/* Is a vector font */
		}
		isInFontPath = 1;
	} else {                                    /* <name>.{ttf,otf,...} */
		char path[AG_FILENAME_MAX];
		const char **ffe;

		for (ffe = &agFontFileExts[0]; *ffe != NULL; ffe++) {
			Strlcpy(path, name, sizeof(path));

			if (flags & AG_FONT_CONDENSED)
				Strlcat(path, "-condensed", sizeof(path));
			if (flags & AG_FONT_MONOSPACE)
				Strlcat(path, "-mono", sizeof(path));

			if (flags & AG_FONT_BOLD) {
				const AG_FontAdjustment *fa;

				/* Is Regular a Bold? */
				for (fa = &agFontAdjustments[0];
				     fa->face != NULL; fa++) {
					if (strcmp(name, fa->face) != 0) {
						continue;
					}
					if (fa->flags & AG_FONT_BOLD)
						break;
				}
				if (fa->face == NULL) {    /* Regular != Bold */
					if (flags & AG_FONT_ITALIC) {
						Strlcat(path, "-bold-italic", sizeof(path));
					} else {
						Strlcat(path, "-bold", sizeof(path));
					}
				} else {
					if (flags & AG_FONT_ITALIC)
						Strlcat(path, "-italic", sizeof(path));
				}
			} else {
				if (flags & AG_FONT_UPRIGHT_ITALIC) {
					Strlcat(path, "-upright-italic", sizeof(path));
				} else if (flags & AG_FONT_ITALIC) {
					Strlcat(path, "-italic", sizeof(path));
				}
			}
			Strlcat(path, *ffe, sizeof(path));
			if (AG_ConfigFind(AG_CONFIG_PATH_FONTS, path,
			    fontPath, sizeof(fontPath)) == 0) {
				isInFontPath = 1;
				break;
			}
		}
	}

#ifdef HAVE_FONTCONFIG
	if (agFontconfigInited && !isInFontPath) {
		FcPattern *pattern, *fpat;
		FcResult fres = FcResultMatch;
		FcChar8 *filename;
		FcMatrix *mat;
		char *s;
		const char *condensed = (flags & AG_FONT_CONDENSED) ? "Condensed " : NULL;
		AG_Size len;
/*		double sizeDbl; */

		len = strlen(nameBase)+64;
		s = Malloc(len);
		if (flags & AG_FONT_MONOSPACE) {
			if ((fontSize - floorf(fontSize)) > 0.0) {
				Snprintf(s,len, "%s Mono-%.2f", nameBase, fontSize);
			} else {
				Snprintf(s,len, "%s Mono-%.0f", nameBase, fontSize);
			}
		} else {
			if ((fontSize - floorf(fontSize)) > 0.0) {
				Snprintf(s,len, "%s-%.2f", nameBase, fontSize);
			} else {
				Snprintf(s,len, "%s-%.0f", nameBase, fontSize);
			}
		}
		if (flags & AG_FONT_BOLD) {
			Strlcat(s, ":style=", len);
			if (flags & AG_FONT_ITALIC) {
				if (condensed) { Strlcat(s, condensed, len); }
				Strlcat(s, "Bold Italic,", len);
				if (condensed) { Strlcat(s, condensed, len); }
				Strlcat(s, "Bold Oblique", len);
			} else if (flags & AG_FONT_OBLIQUE) {
				if (condensed) { Strlcat(s, condensed, len); }
				Strlcat(s, "Bold Oblique,", len);
				if (condensed) { Strlcat(s, condensed, len); }
				Strlcat(s, "Bold Italic", len);
			} else {
				Strlcat(s, ":style=", len);
				if (condensed) { Strlcat(s, condensed, len); }
				Strlcat(s, "Bold", len);
			}
		} else if (flags & AG_FONT_ITALIC) {
			Strlcat(s, ":style=", len);
			if (condensed) { Strlcat(s, condensed, len); }
			Strlcat(s, "Italic,", len);
			if (condensed) { Strlcat(s, condensed, len); }
			Strlcat(s, "Oblique", len);
		} else if (flags & AG_FONT_OBLIQUE) {
			Strlcat(s, ":style=", len);
			if (condensed) { Strlcat(s, condensed, len); }
			Strlcat(s, "Oblique,", len);
			if (condensed) { Strlcat(s, condensed, len); }
			Strlcat(s, "Italic", len);
		} else if (condensed) {
			Strlcat(s, ":style=Condensed", len);
		}
/*		Debug(NULL, "FcNameParse(%s, 0x%x)\n", s, flags); */

		if ((pattern = FcNameParse((FcChar8 *)s)) == NULL ||
		    !FcConfigSubstitute(NULL, pattern, FcMatchPattern)) {
			AG_SetError(_("Fontconfig failed to parse: %s"), name);
			free(s);
			goto fail;
		}
		free(s);

		FcDefaultSubstitute(pattern);
		if ((fpat = FcFontMatch(NULL, pattern, &fres)) == NULL ||
		    fres != FcResultMatch) {
			AG_SetError(_("Fontconfig failed to match: %s"), name);
			goto fail;
		}
		if (FcPatternGetString(fpat, FC_FILE, 0, &filename) != FcResultMatch) {
			AG_SetErrorS("No FC_FILE");
			goto fail;
		}
		Strlcpy(fontPath, (const char *)filename, sizeof(fontPath));
	
		if (FcPatternGetInteger(fpat, FC_INDEX, 0, &spec->index)  != FcResultMatch) {
			AG_SetErrorS("No FC_INDEX");
			goto fail;
		}
#if 0
		if (FcPatternGetDouble(fpat, FC_SIZE, 0, &sizeDbl) != FcResultMatch) {
			AG_SetErrorS("No FC_SIZE");
			goto fail;
		}
		spec->size = (float)sizeDbl;
#endif
		if (FcPatternGetMatrix(fpat, FC_MATRIX, 0, &mat) == FcResultMatch) {
			spec->matrix.xx = mat->xx;
			spec->matrix.yy = mat->yy;
			spec->matrix.xy = mat->xy;
			spec->matrix.yx = mat->yx;
		}
		spec->type = AG_FONT_VECTOR;
		FcPatternDestroy(fpat);
		FcPatternDestroy(pattern);
	}
#endif /* HAVE_FONTCONFIG */

init_font:
	switch (spec->type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		if (!agFreetypeInited) {
			AG_SetErrorS("FreeType is not initialized");
			goto fail;
		}
		if (AG_TTFOpenFont(font, fontPath) == -1) {
			goto fail;
		}
		break;
#endif
	case AG_FONT_BITMAP:
		if (BitmapOpenFont(font, fontPath) == -1) {
			goto fail;
		}
		break;
	case AG_FONT_DUMMY:
		break;
	default:
		AG_SetErrorS("Unsupported font type");
		goto fail;
	}
	TAILQ_INSERT_HEAD(&agFontCache, font, fonts);
out:
	font->nRefs++;
	AG_MutexUnlock(&agTextLock);
	return (font);
fail:
#ifdef DEBUG_FONTS
	Debug(NULL, "Font (\"%s\":%.02f:%x): %s\n", face, fontSize, flags,
	    AG_GetError());
#endif
	AG_MutexUnlock(&agTextLock);
	AG_ObjectDestroy(font);
	return (NULL);
}

/*
 * Restore the previous font-engine rendering state.
 *
 * Must be called from Widget draw(), size_alloc(), size_request() or
 * event-handling context. Widget must have the USE_TEXT flag set.
 */
void
AG_PopTextState(void)
{
#ifdef AG_DEBUG
	AG_TextState *ts = AG_TEXT_STATE_CUR();

	if (agTextStateCur == 0)
		AG_FatalError("PopTextState without Push");

	if (ts->name[0] != 'T' ||
	    ts->name[1] != 'S' || atoi(&ts->name[2]) != agTextStateCur)
		AG_FatalErrorF("PopTextState: Bad state #%d", agTextStateCur);
#endif
/*	if (ts->font != NULL) */
/*		AG_UnusedFont(ts->font); */

	--agTextStateCur;
}

/*
 * Decrement reference count on a font, delete if it reaches zero.
 * TODO make this work asynchronously.
 */
void
AG_UnusedFont(AG_Font *font)
{
	AG_OBJECT_ISA(font, "AG_Font:*");

	AG_MutexLock(&agTextLock);
	if (font != agDefaultFont) {
		if (--font->nRefs == 0) {
			TAILQ_REMOVE(&agFontCache, font, fonts);
			AG_ObjectDestroy(font);
		}
	}
	AG_MutexUnlock(&agTextLock);
}

/* Clear the glyph cache. */
void
AG_TextClearGlyphCache(AG_Driver *drv)
{
	int i;
	AG_Glyph *G, *Gnext;

	for (i = 0; i < AG_GLYPH_NBUCKETS; i++) {
		for (G = SLIST_FIRST(&drv->glyphCache[i].glyphs);
		     G != SLIST_END(&drv->glyphCache[i].glyphs);
		     G = Gnext) {
			Gnext = SLIST_NEXT(G, glyphs);
			AG_SurfaceFree(G->su);
			free(G);
		}
		SLIST_INIT(&drv->glyphCache[i].glyphs);
	}
}

static __inline__ void
InitMetrics(AG_TextMetrics *_Nonnull tm)
{
	tm->w = 0;
	tm->h = 0;
	tm->wLines = NULL;
	tm->nLines = 0;
}

static __inline__ void
FreeMetrics(AG_TextMetrics *_Nonnull tm)
{
	Free(tm->wLines);
}

/* For DUMMY font engine */
static void
TextSizeDummy(const AG_Char *_Nonnull ucs, AG_TextMetrics *_Nonnull tm,
    int extended)
{
	tm->w = 0;
	tm->h = 0;
	tm->wLines = NULL;
	tm->nLines = 0;
}

#ifdef HAVE_FREETYPE
/*
 * Compute the rendered size of UCS-4 text with a FreeType font. If the
 * string is multiline and nLines is non-NULL, the width of individual lines
 * is returned into wLines, and the number of lines into nLines.
 */
static void
TextSizeFT(const AG_Char *_Nonnull ucs, AG_TextMetrics *_Nonnull tm, int extended)
{
	AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_Font *fontOrig = ts->font, *fontCur = fontOrig;
	AG_TTFGlyph *G;
	const AG_Char *ch;
	const int lineskip = fontCur->lineskip;
	int xMin=0, xMax=0, yMin=0, yMax;
	int xMinLine=0, xMaxLine=0;
	int x, z;

	/* Compute the sum of the bounding box of the characters. */
	yMax = fontCur->lineskip;
	x = 0;
	for (ch = &ucs[0]; *ch != '\0'; ch++) {
		AG_TTFFont *ttf;

		if (*ch == '\n') {
			if (extended) {
				tm->wLines = Realloc(tm->wLines,
				    (tm->nLines+2)*sizeof(Uint));
				tm->wLines[tm->nLines++] = (xMaxLine-xMinLine);
				xMinLine = 0;
				xMaxLine = 0;
			}
			yMax += lineskip;
			x = 0;
			continue;
		}
		if (*ch == '\t') {
			x += ts->tabWd;  /* XXX TODO */
			continue;
		}
		if (ch[0] == 0x1b &&
		    ch[1] >= 0x40 && ch[1] <= 0x5f && ch[2] != '\0') {
			AG_TextANSI ansi;
			
			if (AG_TextParseANSI(ts, &ansi, &ch[1]) != 0) {
				continue;
			}
			if (ansi.ctrl == AG_ANSI_CSI_SGR) {
				switch (ansi.sgr) {
				case AG_SGR_RESET:
				case AG_SGR_NO_FG_NO_BG:
					if (fontCur != fontOrig) {
						fontCur = fontOrig;
					}
					break;
				case AG_SGR_BOLD:
					fontCur = AG_FetchFont(
					    OBJECT(fontOrig)->name,
					    fontOrig->spec.size,
					    fontOrig->flags | AG_FONT_BOLD);
					if (fontCur == NULL) {
						fontCur = fontOrig;
					}
					break;
				case AG_SGR_ITALIC:
					fontCur = AG_FetchFont(
					    OBJECT(fontOrig)->name,
					    fontOrig->spec.size,
					    fontOrig->flags | AG_FONT_ITALIC);
					if (fontCur == NULL) {
						fontCur = fontOrig;
					}
					break;
				case AG_SGR_PRI_FONT:
				case AG_SGR_ALT_FONT_1:
				case AG_SGR_ALT_FONT_2:
				case AG_SGR_ALT_FONT_3:
				case AG_SGR_ALT_FONT_4:
				case AG_SGR_ALT_FONT_5:
				case AG_SGR_ALT_FONT_6:
				case AG_SGR_ALT_FONT_7:
				case AG_SGR_ALT_FONT_8:
				case AG_SGR_ALT_FONT_9:
				case AG_SGR_FRAKTUR:
					fontCur = AG_FetchFont(
					    agCoreFonts[ansi.sgr-10],
					    fontOrig->spec.size,
					    fontOrig->flags);
					if (fontCur == NULL) {
						fontCur = fontOrig;
					}
					break;
				default:
					break;
				}
			}
			ch += ansi.len;
			continue;
		}

		ttf = fontCur->data.vec.ttf;

		if (AG_TTFFindGlyph(ttf, *ch, TTF_CACHED_METRICS) != 0) {
			continue;
		}
		G = ttf->current;

		z = x + G->minx;
		if (xMin > z) { xMin = z; }
		if (xMinLine > z) { xMinLine = z; }

		if (ttf->style & AG_FONT_SW_BOLD) {        /* Software Bold */
			x += ttf->glyph_overhang;
		}
		z = x + MAX(G->advance, G->maxx);
		if (xMax < z) { xMax = z; }
		if (xMaxLine < z) { xMaxLine = z; }
		x += G->advance;

		if (G->miny < yMin) { yMin = G->miny; }
		if (G->maxy > yMax) { yMax = G->maxy; }
	}
	if (*ch != '\n' && extended) {
		if (tm->nLines > 0) {
			tm->wLines = Realloc(tm->wLines,
			    (tm->nLines+2)*sizeof(Uint));
			tm->wLines[tm->nLines] = (xMaxLine-xMinLine);
		}
		tm->nLines++;
	}
	tm->w = (xMax-xMin);
	tm->h = (yMax-yMin);
}
#endif /* !HAVE_FREETYPE */

/*
 * Compute the rendered size of UCS-4 text with a bitmap font.
 */
static void
TextSizeBitmap(const AG_Char *_Nonnull ucs, AG_TextMetrics *_Nonnull tm,
    int extended)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	const AG_Char *c;
	AG_Surface *Sglyph;
	const int tabWd = ts->tabWd;
	const int lineskip = ts->font->lineskip;
	int wLine=0;

	for (c = &ucs[0]; *c != '\0'; c++) {
		if (*c == '\n') {
			if (extended) {
				tm->wLines = Realloc(tm->wLines,
				    (tm->nLines+2)*sizeof(Uint));
				tm->wLines[tm->nLines++] = wLine;
				wLine = 0;
			}
			tm->h += lineskip;
			continue;
		}
		if (*c == '\t') {
			wLine += tabWd;
			tm->w += tabWd;
			continue;
		}
		if (*c == 0x1b &&
		    c[1] >= 0x40 && c[1] <= 0x5f && c[2] != '\0') {
			AG_TextANSI ansi;
			
			if (AG_TextParseANSI(ts, &ansi, &c[1]) == 0) {
				c += ansi.len;
				continue;
			}
		}
		Sglyph = GetBitmapGlyph(ts->font, *c);
		wLine += Sglyph->w;
		tm->w += Sglyph->w;
		tm->h = MAX(tm->h, Sglyph->h);
	}
	if (*c != '\n' && extended) {
		if (tm->nLines > 0) {
			tm->wLines = Realloc(tm->wLines,
			    (tm->nLines+2)*sizeof(Uint));
			tm->wLines[tm->nLines] = wLine;
		}
		tm->nLines++;
	}
}

/* Render UCS-4 text to a new surface using a bitmap font. */
static void
TextRenderBitmap(const AG_Char *_Nonnull ucs, AG_Surface *_Nonnull S,
    const AG_TextMetrics *_Nonnull tm, AG_Font *_Nonnull font,
    const AG_Color *_Nonnull cBgOrig, const AG_Color *_Nonnull cFgOrig)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	const int lineskip = font->lineskip;
	AG_Surface *Sglyph;
	const AG_Char *c;
	AG_Rect rd;
	int line;

	/* TODO colorize mode */
	(void)cBgOrig;
	(void)cFgOrig;

	rd.x = (tm->nLines > 1) ? AG_TextJustifyOffset(tm->w, tm->wLines[0]) : 0;
	rd.y = 0;
	
	for (c=&ucs[0], line=0; *c != '\0'; c++) {
		if (*c == '\n') {
			rd.y += lineskip;
			rd.x = AG_TextJustifyOffset(tm->w, tm->wLines[++line]);
			continue;
		}
		if (*c == '\t') {
			rd.x += ts->tabWd;
			continue;
		}
		if (*c == 0x1b &&
		    c[1] >= 0x40 && c[1] <= 0x5f && c[2] != '\0') {
			AG_TextANSI ansi;
			
			if (AG_TextParseANSI(ts, &ansi, &c[1]) == 0) {
				c += ansi.len;
				continue;
			}
		}
		Sglyph = GetBitmapGlyph(font, *c);
		if (*c != ' ') {
			AG_SurfaceBlit(Sglyph, NULL, S, rd.x, rd.y);
		}
		rd.x += Sglyph->w;
	}

	AG_SurfaceSetColorKey(S, AG_SURFACE_COLORKEY,
	    AG_MapPixel_RGBA(&S->format, 0,0,0,0));

	AG_SurfaceSetAlpha(S, AG_SURFACE_ALPHA,
	    font->data.bmp.glyphs[0]->alpha);
}

static AG_Surface *_Nonnull
GetBitmapGlyph(const AG_Font *_Nonnull font, AG_Char c)
{
	if ((font->flags & AG_FONT_UPPERCASE) &&
	    (isalpha((int)c) && islower((int)c))) {
		c = (AG_Char)toupper((int)c);
	}
	if (c < font->data.bmp.c0 || c > font->data.bmp.c1) {
		return (AG_TEXT_STATE_CUR()->font->data.bmp.glyphs[0]);
	}
	return (font->data.bmp.glyphs[c - font->data.bmp.c0 + 1]);
}

static void
TextRenderDummy(const AG_Char *_Nonnull ucs, AG_Surface *_Nonnull S,
    const AG_TextMetrics *_Nonnull tm, AG_Font *_Nonnull font,
    const AG_Color *_Nonnull cBgOrig, const AG_Color *_Nonnull cFgOrig)
{
	/* no-op */
}

/*
 * Return the expected size in pixels of a rendered C string.
 * If Unicode is supported, the string may contain UTF-8.
 */
void
AG_TextSize(const char *text, int *w, int *h)
{
#ifdef AG_UNICODE
	AG_Char *s;
#endif
	if (text == NULL || text[0] == '\0') {
		if (w != NULL) { *w = 0; }
		if (h != NULL) { *h = 0; }
		return;
	}
#ifdef AG_UNICODE
	if ((s = AG_ImportUnicode("UTF-8", text, NULL, NULL)) != NULL) {
		AG_TextSizeInternal(s, w,h);
		free(s);
	}
#else
	AG_TextSizeInternal((const Uint8 *)text, w,h);
#endif
}

/*
 * Return the rendered size in pixels of a string of internal AG_Char (which
 * may be internal UCS-4 or ASCII).
 */
void
AG_TextSizeInternal(const AG_Char *s, int *w, int *h)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	void (*pfSize[])(const AG_Char *_Nonnull, AG_TextMetrics *_Nonnull, int) = {
		TextSizeFT,     /* VECTOR */
		TextSizeBitmap, /* BITMAP */
		TextSizeDummy   /* DUMMY */
	};
	AG_TextMetrics tm;

	InitMetrics(&tm);

	if (s != NULL && (char)(s[0]) != '\0') {
		const enum ag_font_type engine = ts->font->spec.type;
#ifdef AG_DEBUG
		if (engine >= AG_FONT_TYPE_LAST) {
			AG_FatalError("Bad font type");
		}
		AG_OBJECT_ISA(ts->font, "AG_Font:*");
#endif
		pfSize[engine](s, &tm, 0);
	}
	if (w != NULL) { *w = tm.w; }
	if (h != NULL) { *h = tm.h; }
	FreeMetrics(&tm);
}

/*
 * Return the rendered size in pixels of a C string (which may contain UTF-8).
 * Return a line count and an array of line widths.
 */
void
AG_TextSizeMulti(const char *text, int *w, int *h, Uint **wLines, Uint *nLines)
{
#ifdef AG_UNICODE
	AG_Char *s;

	if ((s = AG_ImportUnicode("UTF-8", text, NULL, NULL)) != NULL) {
		AG_TextSizeMultiInternal(s, w, h, wLines, nLines);
		free(s);
		return;
	}
#else
	AG_TextSizeMultiInternal((const Uint8 *)text, w, h, wLines, nLines);
#endif
}

/*
 * Compute the rendered size of a string of internal AG_Char (which may
 * be UCS-4 or ASCII). Return a line count and an array of line widths.
 */
void
AG_TextSizeMultiInternal(const AG_Char *s, int *w, int *h, Uint **wLines,
    Uint *nLines)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	void (*pfSize[])(const AG_Char *_Nonnull, AG_TextMetrics *_Nonnull, int) = {
		TextSizeFT,     /* VECTOR */
		TextSizeBitmap, /* BITMAP */
		TextSizeDummy   /* DUMMY */
	};
	AG_TextMetrics tm;
	const enum ag_font_type engine = ts->font->spec.type;

#ifdef AG_DEBUG
	if (engine >= AG_FONT_TYPE_LAST) {
		AG_FatalError("Bad font type");
	}
	AG_OBJECT_ISA(ts->font, "AG_Font:*");
#endif
	InitMetrics(&tm);
	if (s != NULL && (char)(s[0]) != '\0') {
		pfSize[engine](s, &tm, 1);
	}
	if (w != NULL) { *w = tm.w; }
	if (h != NULL) { *h = tm.h; }

	if (tm.nLines == 1) {
		tm.wLines = Realloc(tm.wLines, sizeof(Uint));
		tm.wLines[0] = tm.w;
	}
	if (wLines != NULL) { *wLines = tm.wLines; }
	if (nLines != NULL) { *nLines = tm.nLines; }
}

/*
 * Canned notification and dialog windows.
 * TODO move this elsewhere
 */
#ifdef AG_WIDGETS

/* Display an informational message window. */
void
AG_TextMsg(enum ag_text_msg_title title, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextMsgS(title, s);
	free(s);
}
void
AG_TextMsgS(enum ag_text_msg_title title, const char *s)
{
	AG_Window *win;
	AG_Box *vb;

	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_NOTIFICATION;
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[title]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_BoxNewVert(win, 0);
	AG_SetStyle(vb, "font-size", "120%");
	AG_LabelNewS(vb, 0, s);

	vb = AG_BoxNewVert(win, AG_BOX_HOMOGENOUS | AG_BOX_EXPAND);
	AG_WidgetFocus( AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win)) );
	AG_WindowShow(win);
}

/* Display the last error message from AG_GetError(). */
void
AG_TextMsgFromError(void)
{
	AG_TextMsgS(AG_MSG_ERROR, AG_GetError());
}

static Uint32
TextTmsgExpire(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Window *win = AGWINDOW(event->argv[1].data.p);

	if (!AG_OBJECT_VALID(win) || !AG_OfClass(win, "AG_Widget:AG_Window:*")) {
		AG_Verbose("Ignoring a TextTmsg() expiration\n");
		return (0);
	}
	AG_ObjectDetach(win);
	return (0);
}

/* Display a message for a given period of time (format string). */
void
AG_TextTmsg(enum ag_text_msg_title title, Uint32 expire, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextTmsgS(title, expire, s);
	free(s);
}
void
AG_TextTmsgS(enum ag_text_msg_title title, Uint32 ticks, const char *s)
{
	AG_Window *win;
	AG_Box *vb;
	AG_Timer *to;

	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_NOTIFICATION;
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[title]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_BoxNewVert(win, 0);
	AG_SetStyle(vb, "font-size", "120%");
	AG_LabelNewS(vb, 0, s);
	AG_WindowShow(win);

	to = AG_AddTimerAuto(NULL, ticks, TextTmsgExpire, "%p", win);
	if (to != NULL)
		Strlcpy(to->name, "textTmsg", sizeof(to->name));
}

/*
 * Display an informational message with a "Don't tell me again" option.
 * The user preference is preserved in a persistent table. Unlike warnings,
 * the dialog window is not modal (format string).
 */
void
AG_TextInfo(const char *key, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextInfoS(key, s);
	free(s);
}
void
AG_TextInfoS(const char *key, const char *s)
{
	AG_Config *cfg = AG_ConfigObject();
	char disableSw[64];
	AG_Variable *Vdisable;
	AG_Window *win;
	AG_Box *vb;
	AG_Checkbox *cb;
	
	if (key != NULL) {
		Strlcpy(disableSw, "info.", sizeof(disableSw));
		Strlcat(disableSw, key, sizeof(disableSw));
		AG_ObjectLock(cfg);
		if (AG_Defined(cfg,disableSw) && AG_GetInt(cfg,disableSw) == 1)
			goto out;
	}
	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_DIALOG;

	AG_WindowSetCaptionS(win, _(agTextMsgTitles[AG_MSG_INFO]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_BoxNewVert(win, 0);
	AG_SetStyle(vb, "font-size", "120%");
	AG_LabelNewS(vb, 0, s);

	vb = AG_BoxNewVert(win, AG_BOX_HOMOGENOUS | AG_BOX_EXPAND);
	AG_WidgetFocus( AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win)) );
	if (key != NULL) {
		cb = AG_CheckboxNewS(win, AG_CHECKBOX_HFILL,
		    _("Don't tell me again"));
		Vdisable = AG_SetInt(cfg, disableSw, 0);
		AG_BindInt(cb, "state", &Vdisable->data.i);
	}
	AG_WindowShow(win);
out:
	if (key != NULL)
		AG_ObjectUnlock(cfg);
}

/*
 * Display a warning message with a "Don't tell me again" option.
 * The user preference is preserved in a persistent table.
 */
void
AG_TextWarning(const char *key, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextWarningS(key, s);
	free(s);
}
void
AG_TextWarningS(const char *key, const char *s)
{
	AG_Config *cfg = AG_ConfigObject();
	char disableSw[64];
	AG_Variable *Vdisable;
	AG_Window *win;
	AG_Box *vb;
	AG_Checkbox *cb;

	if (key != NULL) {
		Strlcpy(disableSw, "warn.", sizeof(disableSw));
		Strlcat(disableSw, key, sizeof(disableSw));
		AG_ObjectLock(cfg);
		if (AG_Defined(cfg,disableSw) && AG_GetInt(cfg,disableSw) == 1)
			goto out;
	}
	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_DIALOG;

	AG_WindowSetCaptionS(win, _(agTextMsgTitles[AG_MSG_WARNING]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_BoxNewVert(win, 0);
	AG_SetStyle(vb, "font-size", "120%");
	AG_LabelNewS(vb, 0, s);

	vb = AG_BoxNewVert(win, AG_BOX_HOMOGENOUS | AG_BOX_EXPAND);
	AG_WidgetFocus( AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win)) );

	if (key != NULL) {
		cb = AG_CheckboxNewS(win, AG_CHECKBOX_HFILL,
		    _("Don't tell me again"));
		Vdisable = AG_SetInt(cfg, disableSw, 0);
		AG_BindInt(cb, "state", &Vdisable->data.i);
	}
	AG_WindowShow(win);

out:
	if (key != NULL)
		AG_ObjectUnlock(cfg);
}

/* Display an error message. */
void
AG_TextError(const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextErrorS(s);
	free(s);
}
void
AG_TextErrorS(const char *s)
{
	AG_Window *win;
	AG_Box *vb;

	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_DIALOG;
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[AG_MSG_ERROR]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_BoxNewVert(win, 0);
	AG_SetStyle(vb, "font-size", "120%");
	AG_LabelNewS(vb, 0, s);

	vb = AG_BoxNewVert(win, AG_BOX_HOMOGENOUS | AG_BOX_EXPAND);
	AG_WidgetFocus( AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win)) );
	AG_WindowShow(win);
}
#endif /* AG_WIDGETS */

/* Align a text surface inside a given space. */
void
AG_TextAlign(int *x, int *y, int wArea, int hArea, int wText, int hText,
    int lPad, int rPad, int tPad, int bPad, enum ag_text_justify justify,
    enum ag_text_valign valign)
{
	switch (justify) {
	case AG_TEXT_LEFT:
		*x = lPad;
		break;
	case AG_TEXT_CENTER:
		*x = ((wArea + lPad + rPad) >> 1) - (wText >> 1);
		break;
	case AG_TEXT_RIGHT:
		*x = wArea - rPad - wText;
		break;
	}
	switch (valign) {
	case AG_TEXT_TOP:
		*y = tPad;
		break;
	case AG_TEXT_MIDDLE:
		*y = ((hArea + tPad + bPad) >> 1) - (hText >> 1);
		break;
	case AG_TEXT_BOTTOM:
		*y = hArea - bPad - wText;
		break;
	}
}

/*
 * Return the offset in pixels needed to align text based on the current
 * justification mode.
 */
int
AG_TextJustifyOffset(int w, int wLine)
{
	switch (AG_TEXT_STATE_CUR()->justify) {
	case AG_TEXT_LEFT:	return (0);
	case AG_TEXT_CENTER:	return ((w >> 1) - (wLine >> 1));
	case AG_TEXT_RIGHT:	return (w - wLine);
	}
	return (0);
}

/*
 * Return the offset in pixels needed to align text based on the current
 * vertical alignment mode.
 */
int
AG_TextValignOffset(int h, int hLine)
{
	switch (AG_TEXT_STATE_CUR()->valign) {
	case AG_TEXT_TOP:	return (0);
	case AG_TEXT_MIDDLE:	return ((h >> 1) - (hLine >> 1));
	case AG_TEXT_BOTTOM:	return (h - hLine);
	}
	return (0);
}

/*
 * Render text (UTF-8 encoded) onto a newly-allocated surface.
 * Inherit font, FG and BG colors from current text state.
 */
AG_Surface *
AG_TextRenderF(const char *fmt, ...)
{
	AG_Surface *S;
	char *text;
	va_list args;

	va_start(args, fmt);
	Vasprintf(&text, fmt, args);
	va_end(args);
	S = AG_TextRender(text);
	free(text);
	return (S);
}

/*
 * Render text (UTF-8 encoded) onto a newly-allocated surface.
 * Inherit font, FG and BG colors from current text state.
 */
AG_Surface *
AG_TextRender(const char *text)
{
	AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_Surface *S;
#ifdef AG_UNICODE
	AG_Char *us;

	if ((us = AG_ImportUnicode("UTF-8", text, NULL, NULL)) == NULL) {
		AG_FatalError(NULL);
	}
	S = AG_TextRenderInternal(us, ts->font, &ts->colorBG, &ts->color);
	free(us);
#else
	S = AG_TextRenderInternal((const Uint8 *)text, ts->font,
	                          &ts->colorBG, &ts->color);
#endif
	return (S);
}

/*
 * Render text in native internal format (AG_Char, which is UCS-4 or ASCII)
 * onto a newly allocated surface.
 */
AG_Surface *
AG_TextRenderInternal(const AG_Char *text, AG_Font *font, const AG_Color *cBg,
    const AG_Color *cFg)
{
	void (*pfSize[])(const AG_Char *_Nonnull, AG_TextMetrics *_Nonnull, int) = {
		TextSizeFT,       /* VECTOR */
		TextSizeBitmap,   /* BITMAP */
		TextSizeDummy     /* DUMMY */
	};
	void (*pfRender[])(const AG_Char *_Nonnull, AG_Surface *_Nonnull,
	                   const AG_TextMetrics *_Nonnull, AG_Font *_Nonnull,
			   const AG_Color *_Nonnull, const AG_Color *_Nonnull) = {
		TextRenderFT,     /* VECTOR */
		TextRenderBitmap, /* BITMAP */
		TextRenderDummy   /* DUMMY */
	};
	const enum ag_font_type engine = font->spec.type;
	AG_TextMetrics tm;
	AG_Surface *S;

	InitMetrics(&tm);
#ifdef AG_DEBUG
	if (engine >= AG_FONT_TYPE_LAST)
		AG_FatalError("Bad font type");
#endif
	pfSize[engine](text, &tm, 1);

	S = AG_SurfaceNew(agSurfaceFmt, tm.w, tm.h, 0);
	AG_FillRect(S, NULL, cBg);

	if (cBg->a == AG_OPAQUE)
		S->format.Amask = 0;                  /* No blending needed */
#if 0
	if (cBg->a == AG_TRANSPARENT)
		AG_SurfaceSetColorKey(S,              /* Basic transparency */
		    AG_SURFACE_COLORKEY,
		    AG_MapPixel(&S->format, cBg));
#endif
	if (tm.w > 0 && tm.h > 0)
		pfRender[engine](text, S, &tm, font, cBg, cFg);

	FreeMetrics(&tm);
	return (S);
}

#ifdef HAVE_FREETYPE
/* Render text using FreeType. */
static void
TextRenderFT(const AG_Char *_Nonnull ucs, AG_Surface *_Nonnull S,
    const AG_TextMetrics *_Nonnull tm, AG_Font *_Nonnull fontOrig,
    const AG_Color *_Nonnull cBgOrig, const AG_Color *_Nonnull cFgOrig)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_Font *fontCur = fontOrig;
	AG_TTFGlyph *G;
	const AG_Char *ch;
	Uint8 *src, *dst;
	AG_TTFFont *ttf;
	AG_Color cBg = *cBgOrig;
	AG_Color cFg = *cFgOrig;
	FT_UInt prev_index = 0;
	const int BytesPerPixel = S->format.BytesPerPixel;
	const int lineSkip = fontOrig->lineskip;
	int xStart, yStart, line, x,y, w;

 	xStart = (tm->nLines>1) ? AG_TextJustifyOffset(tm->w, tm->wLines[0]) : 0;
 	yStart = 0;

	for (ch=&ucs[0], line=0; *ch != '\0'; ch++) {
		switch (*ch) {
		case '\n':
			yStart += lineSkip;
			xStart = AG_TextJustifyOffset(tm->w, tm->wLines[++line]);
			continue;
		case '\r':
			xStart = 0;
			continue;
		case '\t':
			xStart += ts->tabWd;  /* XXX TODO */
			continue;
		default:
			break;
			
		}
		if (ch[0] == 0x1b &&
		    ch[1] >= 0x40 && ch[1] <= 0x5f && ch[2] != '\0') {
			AG_TextANSI ansi;

			if (AG_TextParseANSI(ts, &ansi, &ch[1]) != 0) {
			/* 	AG_Verbose("%s; ignoring\n", AG_GetError()); */
				continue;
			}
			if (ansi.ctrl == AG_ANSI_CSI_SGR) {
				switch (ansi.sgr) {
				case AG_SGR_RESET:
				case AG_SGR_NO_FG_NO_BG:
					if (fontCur != fontOrig) {
						fontCur = fontOrig;
					}
					cFg = *cFgOrig;
					cBg = *cBgOrig;
					break;
				case AG_SGR_FG:
					cFg = ansi.color;
					break;
				case AG_SGR_BG:
					cBg = ansi.color;
					break;
				case AG_SGR_BOLD:
					fontCur = AG_FetchFont(
					    OBJECT(fontCur)->name,
					    fontCur->spec.size,
					    fontCur->flags | AG_FONT_BOLD);
					if (fontCur == NULL) {
						fontCur = fontOrig;
					}
					break;
				case AG_SGR_ITALIC:
					fontCur = AG_FetchFont(
					    OBJECT(fontCur)->name,
					    fontCur->spec.size,
					    fontCur->flags | AG_FONT_ITALIC);
					if (fontCur == NULL) {
						fontCur = fontOrig;
					}
					break;
				case AG_SGR_FAINT:
					/* TODO */
					break;
				case AG_SGR_UNDERLINE:
					/* TODO */
					break;
				case AG_SGR_PRI_FONT:
				case AG_SGR_ALT_FONT_1:
				case AG_SGR_ALT_FONT_2:
				case AG_SGR_ALT_FONT_3:
				case AG_SGR_ALT_FONT_4:
				case AG_SGR_ALT_FONT_5:
				case AG_SGR_ALT_FONT_6:
				case AG_SGR_ALT_FONT_7:
				case AG_SGR_ALT_FONT_8:
				case AG_SGR_ALT_FONT_9:
				case AG_SGR_FRAKTUR:
					fontCur = AG_FetchFont(
					    agCoreFonts[ansi.sgr-10],
					    fontCur->spec.size,
					    fontCur->flags);
					if (fontCur == NULL) {
						fontCur = fontOrig;
					}
					break;
				default:
					break;
				}
			}
			ch += ansi.len;
			continue;
		}

		ttf = fontCur->data.vec.ttf;

		if (AG_TTFFindGlyph(ttf, *ch, TTF_CACHED_METRICS |
		                              TTF_CACHED_PIXMAP)) {
			return;
		}
		G = ttf->current;

		/*
		 * Ensure the width of the pixmap is correct. On some cases,
		 * freetype may report a larger pixmap than possible.
		 */
		if ((w = G->pixmap.width) > G->maxx - G->minx)
			w = G->maxx - G->minx;

		if (FT_HAS_KERNING(ttf->face) && prev_index && G->index) {
			FT_Vector delta; 

			FT_Get_Kerning(ttf->face, prev_index, G->index,
			    ft_kerning_default, &delta);

			xStart += delta.x >> 6;
		}
	
		/* Prevent texture wrapping with first glyph. */
		if ((ch == &ucs[0]) && (G->minx < 0)) {
			xStart -= G->minx;
		}
		if ((xStart + G->minx) < 0 ||
		    (xStart + G->minx) >= S->w) {
			continue;
		}
		if (cBg.a == AG_TRANSPARENT) {       /* Put on transparent BG */
			for (y = 0; y < G->pixmap.rows; y++) {
				if ((yStart + y + G->yoffset) < 0 ||
				    (yStart + y + G->yoffset) >= S->h)
					continue;

				src = (Uint8 *)(G->pixmap.buffer +
				                G->pixmap.pitch*y);
				dst = S->pixels +
				    (yStart + y + G->yoffset)*S->pitch +
				    (xStart + G->minx)*BytesPerPixel;
	
				for (x = 0; x < w; x++) {
					if ((cFg.a = AG_8toH(*src++)) > 0) {
						AG_SurfacePut_At(S, dst,
						    AG_MapPixel(&S->format, &cFg));
					}
					dst += BytesPerPixel;
				}
			}
		} else {                            /* Blend against color BG */
			for (y = 0; y < G->pixmap.rows; y++) {
				if (y+G->yoffset < 0 || y+G->yoffset >= S->h)
					continue;

				src = (Uint8 *)(G->pixmap.buffer +
				                G->pixmap.pitch*y);
				dst = S->pixels +
				    (yStart + y + G->yoffset)*S->pitch +
				    (xStart + G->minx)*BytesPerPixel;
	
				for (x = 0; x < w; x++) {
					if ((cFg.a = AG_8toH(*src++)) > 0) {
						AG_SurfaceBlend_At(S, dst, &cFg,
						    AG_ALPHA_DST);
					}
					dst += BytesPerPixel;
				}
			}
		}

		xStart += G->advance;

		if (ttf->style & AG_FONT_SW_BOLD) {        /* Software Bold */
			xStart += ttf->glyph_overhang;
		}
		prev_index = G->index;
	}

	ttf = fontOrig->data.vec.ttf;
	if (ttf->style & AG_FONT_UNDERLINE)
		TextRenderFT_Underline(ttf, S, tm->nLines);
}
#endif /* HAVE_FREETYPE */

/*
 * Interpret a possible ANSI escape sequence within a native string and write
 * a normalized description into *ansi. Return 0 on success, -1 on failure.
 */
int
AG_TextParseANSI(const AG_TextState *ts, AG_TextANSI *_Nonnull ansi,
    const AG_Char *_Nonnull s)
{
	const AG_Char *c = &s[0];
	int len;

	switch (*c) {
	case '[':						/* CSI */
		break;
	case 'N':						/* SS2 */
	case 'O':						/* SS3 */
	case 'P':						/* DCS */
		ansi->ctrl = AG_ANSI_SS2 + (*c - (AG_Char)'N');
		ansi->len = 1;
		return (0);
	case '\\':						/* ST */
	case ']':						/* OSC */
	case '^':						/* PM */
	case '_':						/* APC */
		ansi->ctrl = AG_ANSI_ST + (*c - (AG_Char)'\\');
		ansi->len = 1;
		return (0);
	case 'X':						/* SOS */
		ansi->ctrl = AG_ANSI_SOS;
		ansi->len = 1;
		return (0);
	case 'c':						/* RIS */
		ansi->ctrl = AG_ANSI_RIS;
		ansi->len = 1;
		return (0);
	default:
		goto fail;
	}

	for (c = &s[1]; (len = (int)(c-s)) <= AG_TEXT_ANSI_SEQ_MAX; c++) {
		char buf[AG_TEXT_ANSI_PARAM_MAX], *pBuf;
		const AG_Char *pSrc;
		char *tok;
		int sgr, i;

		switch (*c) {
		case 'm':					/* SGR */
			ansi->ctrl = AG_ANSI_CSI_SGR;
			if (len >= sizeof(buf)) {
				goto fail;
			} else if (len == 0) {
				break;
			}
			for (i=0, pSrc=&s[1], pBuf=buf;
			     i < len;
			     i++) {
				*pBuf = (char)(*pSrc);    /* AG_Char -> ASCII */
				pBuf++;
				pSrc++;
			}
			pBuf[-1] = '\0';
#ifdef DEBUG_ANSI
			Debug(NULL, "SGR (%d bytes): \"%s\"\n", len, buf);
#endif
			pBuf = buf;
			if ((tok = Strsep(&pBuf, ":;")) == NULL) {
				break;
			}
			if ((sgr = atoi(tok)) >= AG_SGR_LAST || sgr < 0) {
				goto fail;
			}
			ansi->sgr = (enum ag_text_sgr_parameter) sgr;
#ifdef DEBUG_ANSI
			Debug(NULL, "SGR parameter %d\n", ansi->sgr);
#endif
			if (sgr == 0) {
				break;
			}
			/*
			 * 3/4-bit color.
			 */
			if (sgr >= 30 && sgr <= 37) {          /* Normal FG */
				ansi->sgr = AG_SGR_FG;
				memcpy(&ansi->color, (ts->colorANSI) ?
				                     &ts->colorANSI[sgr-30] :
				                   &agTextColorANSI[sgr-30],
				       sizeof(AG_Color));
				break;
			} else if (sgr >= 40 && sgr <= 47) {    /* Normal BG */
				ansi->sgr = AG_SGR_BG;
				memcpy(&ansi->color, (ts->colorANSI) ?
				                     &ts->colorANSI[sgr-40] :
				                   &agTextColorANSI[sgr-40],
				       sizeof(AG_Color));
				break;
			} else if (sgr >= 90 && sgr <= 97) {    /* Bright FG */
				ansi->sgr = AG_SGR_FG;
				memcpy(&ansi->color, (ts->colorANSI) ?
				                     &ts->colorANSI[8+sgr-90] :
				                   &agTextColorANSI[8+sgr-90],
				       sizeof(AG_Color));
				break;
			} else if (sgr >= 100 && sgr <= 107) {  /* Bright BG */
				ansi->sgr = AG_SGR_BG;
				memcpy(&ansi->color, (ts->colorANSI) ?
				                     &ts->colorANSI[8+sgr-90] :
				                   &agTextColorANSI[8+sgr-90],
				       sizeof(AG_Color));
				break;
			}
			switch (ansi->sgr) {
			case AG_SGR_FG:
			case AG_SGR_BG:
				if (!(tok = Strsep(&pBuf, ":;")) ||
				    tok[0] == '\0' || tok[1] != '\0') {
					goto fail_param;
				}
				switch (tok[0]) {
				case '2':                      /* 24-bit RGB */
					ansi->color.r = (tok = Strsep(&pBuf, ":;")) ? AG_8toH(atoi(tok)) : 0;
					ansi->color.g = (tok = Strsep(&pBuf, ":;")) ? AG_8toH(atoi(tok)) : 0;
					ansi->color.b = (tok = Strsep(&pBuf, ":;")) ? AG_8toH(atoi(tok)) : 0;
					ansi->color.a = AG_OPAQUE;
					break;
				case '5':                   /* 8-bit indexed */
					if (!(tok = Strsep(&pBuf, ":;")) || tok[0] == '\0') {
						goto fail_param;
					}
					if ((i = atoi(tok)) >= 0 && i <= 15) {
						memcpy(&ansi->color,
						    (ts->colorANSI) ?
						    &ts->colorANSI[i] :
						    &agTextColorANSI[i],
						    sizeof(AG_Color));
					}
					break;
				}
				break;
			case AG_SGR_NO_FG:
				if ((tok = Strsep(&pBuf, ":;")) &&
				    tok[0] == '4' && tok[1] == '9' &&
				    tok[2] == '\0') {
					ansi->sgr = AG_SGR_NO_FG_NO_BG;
				}
				break;
			case AG_SGR_NO_BG:
				if ((tok = Strsep(&pBuf, ":;")) &&
				    tok[0] == '3' && tok[1] == '9' &&
				    tok[2] == '\0') {
					ansi->sgr = AG_SGR_NO_FG_NO_BG;
				}
				break;
			default:
				break;
			}
			break;
		case 'A':					/* CUU */
		case 'B':					/* CUD */
		case 'C':					/* CUF */
		case 'D':					/* CUB */
		case 'E':					/* CNL */
		case 'F':					/* CPL */
		case 'G':					/* CHA */
			ansi->ctrl = AG_ANSI_CSI_CUU + (*c - (AG_Char)'A');
			break;
		case 'H':					/* CUP */
		case 'f':
			ansi->ctrl = AG_ANSI_CSI_CUP;
			break;
		case 'J':					/* ED */
		case 'K':					/* EL */
			ansi->ctrl = AG_ANSI_CSI_ED + (*c - (AG_Char)'J');
			break;
		case 'S':					/* SU */
		case 'T':					/* SD */
			ansi->ctrl = AG_ANSI_CSI_SU + (*c - (AG_Char)'S');
			break;
		case 'h':					/* Private */
		case 'i':
		case 'l':
			ansi->ctrl = AG_ANSI_PRIVATE;
			break;
		case 'n':
			ansi->ctrl = AG_ANSI_CSI_DSR;
			break;
		case 's':
			ansi->ctrl = AG_ANSI_CSI_SCP;
			break;
		case 'u':
			ansi->ctrl = AG_ANSI_CSI_RCP;
			break;
		case '\0':
			goto fail_param;
		default:
			if ((*c >= 0x70 && *c < 0x7e) || /* Private final byte */
			    strchr("<=>?", *c)) {        /* Private parameter */
				ansi->ctrl = AG_ANSI_PRIVATE;
				break;
			}
			if ((*c >= 0x30 && *c <= 0x3f) || /* Parameter byte */
			    (*c >= 0x20 && *c <= 0x2f)) { /* Intermediate byte */
				continue;
			}
			goto fail_param;
		}
		break;
	}
	ansi->len = len+1;
	return (0);
fail:
	AG_SetError("Bad ANSI code (before `%c')", *c);
	return (-1);
fail_param:
	AG_SetError("Bad ANSI parameter (before `%c')", *c);
	return (-1);
}

#ifdef HAVE_FREETYPE
/*
 * Render underline style.
 * Surface must be at least ftFont->underline_height pixels high.
 */
static void
TextRenderFT_Underline(AG_TTFFont *_Nonnull ftFont, AG_Surface *_Nonnull S,
    int nLines)
{
	AG_Color c = AG_TEXT_STATE_CUR()->color;
	AG_Pixel px = AG_MapPixel(&S->format, &c);
	Uint8 *pDst;
	const int pad = 2;	/* TODO */
	int w = S->w - pad;
	int lh = ftFont->underline_height;
	int incr = ftFont->ascent - ftFont->underline_offset - lh;
	int line, y0, lineskip = ftFont->lineskip;

	for (line=0, y0=incr; line < nLines; line++) {
		int x, y;

		if (y0 >= S->h) {
			y0 = S->h - lh;
		}
		pDst = S->pixels + y0*S->pitch;
		for (y = 0; y < lh; y++) {
			for (x = pad; x < w; x++) {
				AG_SurfacePut_At(S, pDst, px);
				pDst += S->format.BytesPerPixel;
			}
			pDst += S->padding;
		}
		y0 += lineskip;
	}
}
#endif /* HAVE_FREETYPE */

/*
 * Lookup/insert a glyph in the glyph cache.
 * Must be called from GUI rendering context.
 */
AG_Glyph *
AG_TextRenderGlyph(AG_Driver *drv, AG_Font *font,
    const AG_Color *cBg, const AG_Color *cFg, AG_Char ch)
{
	AG_Glyph *G;
	const Uint h = (Uint)(ch % AG_GLYPH_NBUCKETS);

	SLIST_FOREACH(G, &drv->glyphCache[h].glyphs, glyphs) {
		if (ch == G->ch &&
		    font == G->font &&
		    AG_ColorCompare(cBg, &G->colorBG) == 0 &&
		    AG_ColorCompare(cFg, &G->color) == 0)
			break;
	}
	if (G == NULL) {
		G = TextRenderGlyph_Miss(drv, font, cBg, cFg, ch);
		SLIST_INSERT_HEAD(&drv->glyphCache[h].glyphs, G, glyphs);
	}
	return (G);
}

/* Render a glyph following a cache miss; called from AG_TextRenderGlyph(). */
static AG_Glyph *_Nonnull
TextRenderGlyph_Miss(AG_Driver *_Nonnull drv, AG_Font *_Nonnull font,
    const AG_Color *_Nonnull cBg, const AG_Color *_Nonnull cFg, AG_Char ch)
{
	AG_Glyph *G;
	AG_Char s[2];

	G = Malloc(sizeof(AG_Glyph));
	G->font = (AG_Font *)font;
	G->colorBG = *cBg;
	G->color = *cFg;
	G->ch = ch;

	s[0] = ch;
	s[1] = '\0';
	G->su = AG_TextRenderInternal(s, font, cBg, cFg);

	switch (font->spec.type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		{
			AG_TTFGlyph *Gttf;

			if (AG_TTFFindGlyph(font->data.vec.ttf, ch,
			    TTF_CACHED_METRICS | TTF_CACHED_BITMAP) == 0) {
				Gttf = ((AG_TTFFont *)font->data.vec.ttf)->current;
				G->advance = Gttf->advance;
			} else {
				G->advance = G->su->w;
			}
		}
		break;
#endif
	case AG_FONT_BITMAP:
		G->advance = G->su->w;
		break;
	case AG_FONT_DUMMY:
	default:
		break;
	}
	AGDRIVER_CLASS(drv)->updateGlyph(drv, G);
	return (G);
}

/* Set active text color. */
void
AG_TextColor(const AG_Color *c)
{
	memcpy(&AG_TEXT_STATE_CUR()->color, c, sizeof(AG_Color));
}
void
AG_TextColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_ColorRGB_8(&AG_TEXT_STATE_CUR()->color, r,g,b);
}
void
AG_TextColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_ColorRGBA_8(&AG_TEXT_STATE_CUR()->color, r,g,b,a);
}

/* Set text color from 0xRRGGBBAA format. */
void
AG_TextColorHex(Uint32 c)
{
	AG_ColorHex32(&AG_TEXT_STATE_CUR()->color, c);
}

/* Set active text background color. */
void
AG_TextBGColor(const AG_Color *c)
{
	memcpy(&AG_TEXT_STATE_CUR()->colorBG, c, sizeof(AG_Color));
}
void
AG_TextBGColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_ColorRGB_8(&AG_TEXT_STATE_CUR()->colorBG, r,g,b);
}
void
AG_TextBGColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_ColorRGBA_8(&AG_TEXT_STATE_CUR()->colorBG, r,g,b,a);
}

/* Set text BG color from 0xRRGGBBAA format. */
void
AG_TextBGColorHex(Uint32 c)
{
	AG_ColorHex(&AG_TEXT_STATE_CUR()->colorBG, c);
}

/* Select a specific font face to use in rendering text. */
void
AG_TextFont(AG_Font *font)
{
	AG_TEXT_STATE_CUR()->font = font;
}

/* Select the justification mode to use in rendering text. */
void
AG_TextJustify(enum ag_text_justify mode)
{
	AG_TEXT_STATE_CUR()->justify = mode;
}

/* Select the vertical alignment mode to use in rendering text. */
void
AG_TextValign(enum ag_text_valign mode)
{
	AG_TEXT_STATE_CUR()->valign = mode;
}

/* Select the tab width in pixels for rendering text. */
void
AG_TextTabWidth(int px)
{
	AG_TEXT_STATE_CUR()->tabWd = px;
}

/*
 * Set the default font to the specified font.
 *
 * Return a pointer to the previous default font (the caller may or may
 * not wish to call AG_UnusedFont() on it).
 */
AG_Font *
AG_SetDefaultFont(AG_Font *font)
{
	AG_Font *prevDefaultFont;
	AG_Config *cfg;

	AG_MutexLock(&agTextLock);

	prevDefaultFont = agDefaultFont;
	agDefaultFont = font;

	agTextFontHeight = font->height;
	agTextFontAscent = font->ascent;
	agTextFontDescent = font->descent;
	agTextFontLineSkip = font->lineskip;

	AG_TEXT_STATE_CUR()->font = font;

	cfg = AG_ConfigObject();
	AG_SetString(cfg, "font.face", OBJECT(font)->name);
	AG_SetInt(cfg, "font.size", (int)font->spec.size);
	AG_SetInt(cfg, "font.flags", font->flags);

	AG_MutexUnlock(&agTextLock);

	return (prevDefaultFont);
}

/*
 * Set the default font from a string "<Face>,<Size>,<Flags>".
 * Size is in points (integer). Flags may include:
 *
 *   'b'  =>  bold
 *   'i'  =>  italic
 *   'U'  =>  uppercase
 *   'I'  =>  upright italic
 */
void
AG_TextParseFontSpec(const char *spec)
{
	char buf[AG_TEXT_FONTSPEC_MAX];
	AG_Config *cfg = AG_ConfigObject();
	char *fs, *s, *c;

	if (spec == NULL) {
		buf[0] = '\0';
	} else {
		Strlcpy(buf, spec, sizeof(buf));
	}
	fs = &buf[0];

	if ((s = AG_Strsep(&fs, ":,/")) != NULL && s[0] != '\0') {
		AG_SetString(cfg, "font.face", s);
	}
	if ((s = AG_Strsep(&fs, ":,/")) != NULL && s[0] != '\0') {
		AG_SetInt(cfg, "font.size", atoi(s));
	}
	if ((s = AG_Strsep(&fs, ":,/")) != NULL && s[0] != '\0') {
		Uint flags = 0;

		for (c = &s[0]; *c != '\0'; c++) {
			switch (*c) {
			case 'b': flags |= AG_FONT_BOLD;           break;
			case 'i': flags |= AG_FONT_ITALIC;         break;
			case 'U': flags |= AG_FONT_UPPERCASE;      break;
			case 'I': flags |= AG_FONT_UPRIGHT_ITALIC; break;
			}
		}
		AG_SetUint(cfg, "font.flags", flags);
	}
}

/* Initialize an AG_Font object. */
static void
AG_Font_Init(void *_Nonnull obj)
{
	AG_Font *font = obj;

	memset(&font->spec, 0, sizeof(AG_FontSpec) + /* spec */
	                       sizeof(Uint) +        /* flags */
	                       sizeof(int) +         /* height */
	                       sizeof(int) +         /* ascent */
	                       sizeof(int) +         /* descent */
	                       sizeof(int) +         /* lineskip */
	                       sizeof(Uint));        /* nRefs */

	/* font->spec.type = AG_FONT_VECTOR; */
	/* font->spec.source = AG_FONT_SOURCE_FILE; */
	font->spec.matrix.xx = 1.0;
	font->spec.matrix.yy = 1.0;

	font->data.vec.ttf = NULL;
}

static void
AG_Font_Destroy(void *_Nonnull obj)
{
	AG_Font *font = obj;

	switch (font->spec.type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		AG_TTFCloseFont(font);
		break;
#endif
	case AG_FONT_BITMAP:
		{
			int i;

			for (i = 0; i < font->data.bmp.nGlyphs; i++) {
				AG_SurfaceFree(font->data.bmp.glyphs[i]);
			}
			free(font->data.bmp.glyphs);
		}
		break;
	case AG_FONT_DUMMY:
	default:
		break;
	}
}

#ifdef AG_WIDGETS
/*
 * Prompt the user with a choice of options.
 * TODO move this elsewhere
 */
AG_Window *
AG_TextPromptOptions(AG_Button **bOpts, Uint nbOpts, const char *fmt, ...)
{
	char *text;
	AG_Window *win;
	AG_Box *bo;
	va_list ap;
	Uint i;

	va_start(ap, fmt);
	Vasprintf(&text, fmt, ap);
	va_end(ap);

	if ((win = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_NOTITLE |
	                        AG_WINDOW_NORESIZE)) == NULL) {
		AG_FatalError(NULL);
	}
	win->wmType = AG_WINDOW_WM_DIALOG;
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_SetStyle(win, "spacing", "8");

	AG_LabelNewS(win, 0, text);
	free(text);

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS | AG_BOX_HFILL);
	for (i = 0; i < nbOpts; i++) {
		bOpts[i] = AG_ButtonNewS(bo, 0, "XXXXXXXXXXX");
	}
	AG_WindowShow(win);
	return (win);
}
#endif /* AG_WIDGETS */

/* Initialize the font engine and configure the default font. */
int
AG_InitTextSubsystem(void)
{
	AG_Config *cfg = AG_ConfigObject();
	AG_User *sysUser;
	AG_TextState *ts;

	if (agTextInitedSubsystem++ > 0)
		return (0);

	AG_MutexInitRecursive(&agTextLock);
	TAILQ_INIT(&agFontCache);

	/* Set the default font search path. */
	AG_ObjectLock(cfg);
	sysUser = AG_GetRealUser();

	if (strcmp(TTFDIR, "NONE") != 0)
		AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, TTFDIR);

#if defined(__APPLE__)
	if (sysUser != NULL &&
	    sysUser->home != NULL) {
		AG_ConfigAddPath(AG_CONFIG_PATH_FONTS, "%s/Library/Fonts",
		    sysUser->home);
	}
	AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, "/Library/Fonts");
	AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, "/System/Library/Fonts");
#elif defined(_WIN32)
	{
		char windir[AG_PATHNAME_MAX];

		if (sysUser != NULL && sysUser->home != NULL) {
			AG_ConfigAddPath(AG_CONFIG_PATH_FONTS, "%s\\Fonts",
			    sysUser->home);
		}
		if (GetWindowsDirectoryA(windir, sizeof(windir)) > 0) {
			AG_ConfigAddPath(AG_CONFIG_PATH_FONTS, "%s\\Fonts",
			    windir);
		}
		AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, ".");
		AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, "..\\share\\agar\\fonts");
	}
#else /* !WIN32 & !APPLE */
	if (sysUser != NULL && sysUser->home != NULL) {
		AG_ConfigAddPath(AG_CONFIG_PATH_FONTS, "%s%s.fonts",
		    sysUser->home, AG_PATHSEP);
	}
#endif
	if (sysUser != NULL)
		AG_UserFree(sysUser);

#ifdef HAVE_FREETYPE
	if (AG_TTFInit() == 0) {		/* Initialize FreeType */
		agFreetypeInited = 1;
	} else {
		AG_Verbose("Failed to initialize FreeType (%s); falling back "
		           "to monospace font engine\n", AG_GetError());
	}
#endif
#ifdef HAVE_FONTCONFIG
	if (FcInit()) {
		agFontconfigInited = 1;
	} else {
		AG_Verbose("Failed to initialize fontconfig; ignoring\n");
	}
#endif
	{
#ifdef AG_DEBUG
		int debugLvlSave = agDebugLvl;

		agDebugLvl = 0;
#endif

		/* Load the default font. */
		if (agFreetypeInited) {
			if (!AG_Defined(cfg,"font.face"))
				AG_SetString(cfg, "font.face", agDefaultFaceFT);
			if (!AG_Defined(cfg,"font.size"))
				AG_SetInt(cfg, "font.size", 14);
		} else {
			if (!AG_Defined(cfg,"font.face"))
				AG_SetString(cfg, "font.face", agDefaultFaceBitmap);
			if (!AG_Defined(cfg,"font.size"))
				AG_SetInt(cfg, "font.size", 16);
		}
		if (!AG_Defined(cfg,"font.flags"))
			AG_SetUint(cfg, "font.flags", 0);
#ifdef AG_DEBUG
		agDebugLvl = debugLvlSave;
#endif
	}
	AG_ObjectUnlock(cfg);

	/* Load the default font. */
	if ((agDefaultFont = AG_FetchFont(NULL, 0.0f, 0)) == NULL) {
		goto fail;
	}
	agTextFontHeight = agDefaultFont->height;
	agTextFontAscent = agDefaultFont->ascent;
	agTextFontDescent = agDefaultFont->descent;
	agTextFontLineSkip = agDefaultFont->lineskip;

	/* Initialize the rendering state. */
	agTextStateCur = 0;
	ts = &agTextStateStack[0];
	ts->font = agDefaultFont;
	AG_ColorWhite(&ts->color);
	AG_ColorNone(&ts->colorBG);
	ts->colorANSI = NULL;
	ts->justify = AG_TEXT_LEFT;
	ts->valign = AG_TEXT_TOP;
	ts->tabWd = agTextTabWidth;
#ifdef AG_DEBUG
	AG_Strlcpy(ts->name, "TS0", sizeof(ts->name));
#endif
	return (0);
fail:
#ifdef HAVE_FREETYPE
	if (agFreetypeInited) {
		AG_TTFDestroy();
		agFreetypeInited = 0;
	}
#endif
	return (-1);
}

void
AG_DestroyTextSubsystem(void)
{
	AG_Font *font, *fontNext;
	
	if (--agTextInitedSubsystem > 0) {
		return;
	}
	for (font = TAILQ_FIRST(&agFontCache);
	     font != TAILQ_END(&agFontCache);
	     font = fontNext) {
		fontNext = TAILQ_NEXT(font, fonts);
		AG_ObjectDestroy(font);
	}
#ifdef HAVE_FREETYPE
	if (agFreetypeInited) {
		AG_TTFDestroy();
		agFreetypeInited = 0;
	}
#endif
#ifdef HAVE_FONTCONFIG
	if (agFontconfigInited) {
		FcFini();
		agFontconfigInited = 0;
	}
#endif
	AG_MutexDestroy(&agTextLock);
}

AG_ObjectClass agFontClass = {
	"AG_Font",
	sizeof(AG_Font),
	{ 0, 0 },
	AG_Font_Init,
	NULL,		/* reset */
	AG_Font_Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL,		/* edit */
};

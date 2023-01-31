/*
 * Copyright (c) 2001-2023 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Base Font class for the AG_Text(3) typography engine.
 */

#include <agar/config/have_freetype.h>
#include <agar/config/have_fontconfig.h>

#include <agar/core/core.h>
#include <agar/core/config.h>

#include <agar/gui/text.h>
#include <agar/gui/fonts_data.h>
#include <agar/gui/font.h>
#include <agar/gui/font_ft.h>
#include <agar/gui/font_bf.h>
#include <agar/gui/gui_math.h>

#ifdef HAVE_FONTCONFIG
# include <fontconfig/fontconfig.h>
#endif

#include <string.h>
#include <ctype.h>

/* #define DEBUG_FONTS */

/*
 * Adjustments to the scaling and baselines of core fonts (relative to Algue).
 * Option flags: AG_FONT_BOLD (bold style is also the Regular style).
 */
const AG_FontAdjustment agFontAdjustments[] = {
/*                                                 0.0 10.4 14.0 21.0 23.8 35.0 to- */
/*                                                10.4 14.0 21.0 23.8 35.0 inf  pts */
	{ "cm-sans",         0,            1.1f, { -4,  -4,  -6,  -7,  -9, -16 } },
	{ "cm-serif",        0,            1.1f, { -1,  -2,   0,  -2,  -4,  -5 } },
	{ "league-spartan",  0,            1.0f, { -2,  -2,  -2,  -3,  -5,  -6 } },
	{ "league-gothic",   AG_FONT_BOLD, 1.1f, { -1,  -1,  -1,  -2,  -1,  -3 } },
	{ "fraktur",         AG_FONT_BOLD, 1.1f, { +1,  +1,  +1,  +1,  +1,  +1 } },
	{ "source-han-sans", 0,            1.0f, { -8, -12, -15, -20, -28, -35 } },
	{ "unialgue",        0,            1.0f, { -3,  -4,  -5,  -7, -11, -12 } },
	{ NULL,              0,            0.0f, {  0,   0,   0,   0,   0,   0 } }
};

/* Recognized font file extensions. */
const char *agFontFileExts[] = {
	".ttf",   /* TrueType Font */
	".otf",   /* OpenType Font */
	".agbf",  /* Agar Bitmap Font */
	".ttc",   /* TrueType Font Collection */
	".woff2", /* Web Open Font Format 2.0 File */
	".woff",  /* Web Open Font Format File */
	".dfont", /* Mac OS X Data Fork Font */
	".fnt",   /* Windows Font File */
	NULL
};

/* Fonts baked into the data segment of the library. */
AG_StaticFont *agBuiltinFonts[] = {
	&agFontAlgue,                    /* Algue Regular */
	&agFontAlgue_Bold,               /* Algue Bold */
	&agFontAlgue_Italic,             /* Algue Italic */
	&agFontAlgue_BoldItalic,         /* Algue Bold Italic */
	NULL
};

/*
 * Append suffix to a core font filename based on Weight and Style.
 *
 * Check in the adjustments table whether this weight happens to be the
 * Regular weight for this font (in which case, we do not append the
 * suffix to the filename).
 */
#undef CAT_WEIGHT_SUFFIX
#define CAT_WEIGHT_SUFFIX(weight, suffix)                                      \
	/* Is this weight the Regular weight for this font? */                 \
	for (fa = &agFontAdjustments[0]; fa->face != NULL; fa++) {             \
		if (strcmp(name, fa->face) == 0 &&                             \
		    (fa->flags & (weight)))                                    \
			break;                                                 \
	}                                                                      \
	if (fa->face == NULL) {                                                \
		if (flags & AG_FONT_ITALIC) {                                  \
			Strlcat(path, suffix "-italic", sizeof(path));         \
		} else if (flags & AG_FONT_OBLIQUE) {                          \
			Strlcat(path, suffix "-oblique", sizeof(path));        \
		} else if (flags & AG_FONT_UPRIGHT_ITALIC) {                   \
			Strlcat(path, suffix "-upright-italic", sizeof(path)); \
		} else {                                                       \
			Strlcat(path, suffix, sizeof(path));                   \
		}                                                              \
	} else {                                                               \
		if (flags & AG_FONT_ITALIC) {                                  \
			Strlcat(path, "-italic", sizeof(path));                \
		} else if (flags & AG_FONT_OBLIQUE) {                          \
			Strlcat(path, "-oblique", sizeof(path));               \
		} else if (flags & AG_FONT_UPRIGHT_ITALIC) {                   \
			Strlcat(path, "-upright-italic", sizeof(path));        \
		}                                                              \
	}

/*
 * Load the given font (or return a pointer to an existing one), from
 * a specified font face, size (in points), and option flags.
 *
 * If face is NULL or fontSize is 0.0, use the default `font.face' and
 * `font.size' from AG_Config(3).
 *
 * Font faces are case-insensitive and may correspond to fontconfig-managed
 * font names, or font files installed in the AG_Config(3) `font-path'.
 *
 * Face names with a leading underscore (e.g., "_agFontAlgue") corresponds
 * to built-in fonts embedded into the library.
 */
AG_Font *
AG_FetchFont(const char *face, float fontSize, Uint flags)
{
	char fontPath[AG_PATHNAME_MAX];
	char name[AG_OBJECT_NAME_MAX];
	char nameBase[AG_OBJECT_NAME_MAX];
	AG_FontSpec spec;
	AG_Config *cfg = AG_ConfigObject();
	AG_Font *font;
	const AG_FontAdjustment *fa;
	int isInFontPath;

	if (face == NULL) {                                  /* Use default */
		AG_GetString(cfg, "font.face", name, sizeof(name));
	} else {
		if (face[0] == '_') {
			/* Builtins match case-sensitively. */
			Strlcpy(name, face, sizeof(name));
		} else {
			const char *pFace;
			char *pDst;

			/* Convert to lowercase for case-insensitive matching. */
			for (pFace=face, pDst=name;
			    *pFace != '\0' && pDst < &name[sizeof(name)-1];
			     pFace++) {
				*pDst = tolower(*pFace);
				pDst++;
			}
			*pDst = '\0';
		}
	}

	if (fontSize < AG_FONT_PTS_EPSILON) {                   /* Default? */
		fontSize = (float)AG_GetInt(cfg, "font.size");
	}
	for (fa = &agFontAdjustments[0]; fa->face != NULL; fa++) {
		/* TODO cache lowercase name so we can case-sensitive compare */
		if (Strcasecmp(name, fa->face) == 0) {
			fontSize *= fa->size_factor;  /* Scaling correction */
			break;
		}
	}

	Strlcpy(nameBase, name, sizeof(nameBase));
	fontPath[0] = '\0';
	memset(&spec, 0, sizeof(spec));
	spec.size = fontSize;
	spec.matrix.xx = 1.0;
	spec.matrix.yy = 1.0;

	AG_MutexLock(&agTextLock);

	TAILQ_FOREACH(font, &agFontCache, fonts) {
		/* TODO cache lowercase name so we can case-sensitive compare */
		if (Strcasecmp(font->name, name) == 0 &&
		    (font->flags == flags) &&
		    Fabs(font->spec.size - fontSize) < AG_FONT_PTS_EPSILON)
			break;
	}
	if (font != NULL)                                       /* In cache */
		goto out;

#ifdef DEBUG_FONTS
	Debug(NULL, "FetchFont(\"%s\" -> \"" AGSI_YEL "%s" AGSI_RST "\", "
	            AGSI_RED "%.02f" AGSI_RST ", "
		    AGSI_RED "0x%x" AGSI_RST ")\n",
		    (face) ? face : "<default>",
	            name, fontSize, flags);
#endif
	if (name[0] == '_') {                           /* Load from memory */
		AG_StaticFont *builtin, **pBuiltin;

		if ((flags & AG_FONT_BOLD) && (flags & AG_FONT_ITALIC)) {
			Strlcat(name, "_BoldItalic", sizeof(name));
		} else if (flags & AG_FONT_BOLD) {
			Strlcat(name, "_Bold", sizeof(name));
		} else if (flags & AG_FONT_ITALIC) {
			Strlcat(name, "_Italic", sizeof(name));
		}
		for (pBuiltin = &agBuiltinFonts[0], builtin = NULL;
		     *pBuiltin != NULL;
		     pBuiltin++) {
			if (strcmp((*pBuiltin)->name, &name[1]) == 0) {
				builtin = *pBuiltin;
				break;
			}
		}
		if (builtin == NULL) {
			AG_SetError(_("No such built-in font: %s"), name);
			goto fail;
		}
		spec.type = builtin->type;
		spec.sourceType = AG_FONT_SOURCE_MEMORY;
		spec.source.mem.data = builtin->data;
		spec.source.mem.size = builtin->size;
		goto open_font;
	} else {                                          /* Load from file */
		spec.sourceType = AG_FONT_SOURCE_FILE;
	}

	isInFontPath = 0;

	if (AG_ConfigFind(AG_CONFIG_PATH_FONTS, name,   /* Exact file match */
	    fontPath, sizeof(fontPath)) == 0) {
		const char *pExt = strrchr(fontPath, '.');
		
		if (pExt && Strcasecmp(pExt,".agbf") == 0) {
			spec.type = AG_FONT_BITMAP;
		} else {
			spec.type = AG_FONT_FREETYPE;
		}
		isInFontPath = 1;
	} else {                             /* Case-insensitive `file.ext' */
		char path[AG_FILENAME_MAX];
		const char **ffe;
		const AG_FontAdjustment *fa;
	
		spec.type = AG_FONT_FREETYPE;

		for (ffe = &agFontFileExts[0]; *ffe != NULL; ffe++) {
			Strlcpy(path, name, sizeof(path));

			if (flags & AG_FONT_ULTRACONDENSED) {
				Strlcat(path, "-ultracondensed", sizeof(path));
			} else if (flags & AG_FONT_CONDENSED) {
				Strlcat(path, "-condensed", sizeof(path));
			} else if (flags & AG_FONT_SEMICONDENSED) {
				Strlcat(path, "-semicondensed", sizeof(path));
			} else if (flags & AG_FONT_SEMIEXPANDED) {
				Strlcat(path, "-semiexpanded", sizeof(path));
			} else if (flags & AG_FONT_EXPANDED) {
				Strlcat(path, "-expanded", sizeof(path));
			} else if (flags & AG_FONT_ULTRAEXPANDED) {
				Strlcat(path, "-ultraexpanded", sizeof(path));
			}

			if (flags & AG_FONT_MONOSPACE)
				Strlcat(path, "-mono", sizeof(path));

			if (flags & AG_FONT_BOLD) {
				CAT_WEIGHT_SUFFIX(AG_FONT_BOLD, "-bold");
			} else if (flags & AG_FONT_LIGHT) {
				CAT_WEIGHT_SUFFIX(AG_FONT_LIGHT, "-light");
			} else if (flags & AG_FONT_SEMIBOLD) {
				CAT_WEIGHT_SUFFIX(AG_FONT_SEMIBOLD, "-semibold");
			} else if (flags & AG_FONT_EXTRALIGHT) {
				CAT_WEIGHT_SUFFIX(AG_FONT_EXTRALIGHT, "-extralight");
			} else if (flags & AG_FONT_THIN) {
				CAT_WEIGHT_SUFFIX(AG_FONT_THIN, "-thin");
			} else if (flags & AG_FONT_BLACK) {
				CAT_WEIGHT_SUFFIX(AG_FONT_BLACK, "-black");
			}
			if (flags & AG_FONT_ITALIC) {
				Strlcat(path, "-italic", sizeof(path));
			} else if (flags & AG_FONT_UPRIGHT_ITALIC) {
				Strlcat(path, "-upright-italic", sizeof(path));
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
	if (agFontconfigInited && !isInFontPath) {      /* Query fontconfig */
		FcPattern *pattern, *fpat;
		FcResult fres = FcResultMatch;
		FcChar8 *filename;
		FcMatrix *mat;
		char *s;
		AG_Size len;
/*		double sizeDbl; */

		len = strlen(nameBase)+64;
		s = Malloc(len);
		if (flags & AG_FONT_MONOSPACE) {       /* Family preference */
			if ((fontSize - floorf(fontSize)) > 0.0) {
				Snprintf(s,len, "%s Mono-%.2f",
				    nameBase, fontSize);
			} else {
				Snprintf(s,len, "%s Mono-%.0f",
				    nameBase, fontSize);
			}
		} else {
			if ((fontSize - floorf(fontSize)) > 0.0) {
				Snprintf(s,len, "%s-%.2f", nameBase, fontSize);
			} else {
				Snprintf(s,len, "%s-%.0f", nameBase, fontSize);
			}
		}

		if ((flags & AG_FONT_WEIGHTS) || (flags & AG_FONT_STYLES) ||
		    (flags & AG_FONT_WD_VARIANTS)) {
			Strlcat(s, ":style=", len);

			if (flags & AG_FONT_CONDENSED) {
				Strlcat(s, "Condensed ", len);
			} else if (flags & AG_FONT_SEMICONDENSED) {
				Strlcat(s, "SemiCondensed ", len);
			}
			if (flags & AG_FONT_BOLD) {
				if (flags & AG_FONT_ITALIC) {
					Strlcat(s, "Bold Italic,"
					           "Bold Oblique", len);
				} else if (flags & AG_FONT_OBLIQUE) {
					Strlcat(s, "Bold Oblique,"
					           "Bold Italic", len);
				} else {
					Strlcat(s, "Bold", len);
				}
			} else if (flags & AG_FONT_SEMIBOLD) {
				if (flags & AG_FONT_ITALIC) {
					Strlcat(s, "SemiBold Italic,"
					           "SemiBold Oblique", len);
				} else if (flags & AG_FONT_OBLIQUE) {
					Strlcat(s, "SemiBold Oblique,"
					           "SemiBold Italic", len);
				} else {
					Strlcat(s, "SemiBold", len);
				}
			} else if (flags & AG_FONT_ITALIC) {
				Strlcat(s, "Italic,Oblique", len);
			} else if (flags & AG_FONT_OBLIQUE) {
				Strlcat(s, "Oblique,Italic", len);
			}
		}

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
	
		if (FcPatternGetInteger(fpat, FC_INDEX, 0, &spec.index) != FcResultMatch) {
			AG_SetErrorS("No FC_INDEX");
			goto fail;
		}
#if 0
		if (FcPatternGetDouble(fpat, FC_SIZE, 0, &sizeDbl) != FcResultMatch) {
			AG_SetErrorS("No FC_SIZE");
			goto fail;
		}
		spec.size = (float)sizeDbl;
#endif
		if (FcPatternGetMatrix(fpat, FC_MATRIX, 0, &mat) == FcResultMatch) {
			spec.matrix.xx = mat->xx;
			spec.matrix.yy = mat->yy;
			spec.matrix.xy = mat->xy;
			spec.matrix.yx = mat->yx;
		}
		spec.type = AG_FONT_FREETYPE;
		FcPatternDestroy(fpat);
		FcPatternDestroy(pattern);
	}
#endif /* HAVE_FONTCONFIG */

open_font:
	switch (spec.type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_FREETYPE:
		if ((font = (AG_Font *)AG_FontFtNew(name, nameBase, &spec,
		    fontPath, flags)) == NULL) {
			goto fail;
		}
		break;
#endif /* HAVE_FREETYPE */
	case AG_FONT_BITMAP:
		if ((font = (AG_Font *)AG_FontBfNew(name, nameBase, &spec,
		    fontPath, flags)) == NULL) {
			goto fail;
		}
		break;
	case AG_FONT_DUMMY:
		if ((font = AG_ObjectNew(NULL, name,
		    (AG_ObjectClass *)&agFontClass)) == NULL) {
			goto fail;
		}
		Strlcpy(font->name, nameBase, sizeof(font->name));
		memcpy(&font->spec, &spec, sizeof(AG_FontSpec));
		font->flags = flags;
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
	AG_MutexUnlock(&agTextLock);
	return (NULL);
}

/*
 * Decrement the reference count on a font. If it reaches 0, release the font.
 * The font must not be the default font (agDefaultFont).
 * agTextLock must be locked.
 */
void
AG_UnusedFont(AG_Font *font)
{
	AG_OBJECT_ISA(font, "AG_Font:*");

	if (font->nRefs > 0)
		font->nRefs--;

	if (font->nRefs == 0) {
#ifdef DEBUG_FONTS
		Debug(font, "Finalizing font\n");
#endif
		AGFONT_OPS(font)->close(font);
		AG_ObjectDestroy(font);
	}

}

static void
Init(void *_Nonnull obj)
{
	AG_Font *font = obj;

	font->name[0] = '\0';

	memset(&font->spec, 0, sizeof(AG_FontSpec) + /* spec */
	                       sizeof(Uint) +        /* flags */
	                       sizeof(int) +         /* height */
	                       sizeof(int) +         /* ascent */
	                       sizeof(int) +         /* descent */
	                       sizeof(int) +         /* lineskip */
	                       sizeof(Uint));        /* nRefs */
}

static int
Open(void *_Nonnull obj, const char *_Nonnull path)
{
/*	AG_Font *font = obj; */

	/*
	 * Load the font from path (or from memory if spec.type is
	 * AG_FONT_SOURCE_MEMORY).
	 */
	return (0);
}

static void
FlushCache(void *_Nonnull obj)
{
/*	AG_Font *font = obj; */

	/* Flush any internal cache by this font instance. */
}

static void
Close(void *_Nonnull obj)
{
/*	AG_Font *font = obj; */

	/* Flush any caches and finalize the font instance. */
}

static void *
GetGlyph(void *_Nonnull obj, AG_Char ch, Uint want)
{
/*	AG_Font *font = obj; */

	/*
	 * Return an AG_Glyph populated with the wanted information (want),
	 * which can be any combination of:
	 * 
	 * - AG_GLYPH_FT_METRICS (glyph metrics),
	 * - AG_GLYPH_FT_BITMAP (bitmap rendering) or
	 * - AG_GLYPH_FT_PIXMAP (pixmap rendering).
	 */
	AG_SetErrorS("Not implemented");
	return (NULL);
}

static void
GetGlyphMetrics(void *_Nonnull obj, AG_Glyph *G)
{
	/* Populate the advance field (for AG_TextRenderGlyph()). */
	G->advance = G->su->w;
}

static void
Render(const AG_Char *_Nonnull ucs, AG_Surface *_Nonnull S,
    const AG_TextMetrics *_Nonnull Tm, AG_Font *_Nonnull fontOrig,
    const AG_Color *_Nonnull cBgOrig, const AG_Color *_Nonnull cFgOrig)
{
	/*
	 * Render a string of native text (ucs) to surface S.
	 * Set Guide 0 of surface S to the typographical baseline.
	 *
	 * This routine must handle (or at the minimum skip over) any
	 * ANSI SGR sequences present in the text.
	 */
}

static void
Size(const AG_Font *_Nonnull font, const AG_Char *_Nonnull ucs,
    AG_TextMetrics *_Nonnull Tm, int extended)
{
	Tm->w = 0;
	Tm->h = 0;
	Tm->wLines = NULL;
	Tm->nLines = 0;
}

AG_FontClass agFontClass = {
	{
		"AG_Font",
		sizeof(AG_Font),
		{ 0, 0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL,		/* edit */
	},
	Open,
	FlushCache,
	Close,
	GetGlyph,
	GetGlyphMetrics,
	Render,
	Size
};

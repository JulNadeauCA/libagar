/*
 * Copyright (c) 2023 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Bitmap font (.agbf) module for the AG_Text(3) typography system.
 */

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/gui/text.h>
#include <agar/gui/font_bf.h>

/* #define DEBUG_FONTS */
/* #define DEBUG_FONTS_UNICODE */

/*
 * Create and load a bitmap font.
 * 
 * The name string may include weight/style suffices.
 * The nameBase string should be the bare font name (without any suffices).
 * Font names are case-insensitive.
 * Font names starting with `_' refer to embedded (built-in) fonts.
 * 
 */
AG_FontBf *
AG_FontBfNew(const char *name, const char *nameBase, const AG_FontSpec *spec,
    const char *path, Uint flags)
{
	AG_FontBf *fontBf;
	AG_Font *font;

	if ((fontBf = TryMalloc(sizeof(AG_FontBf))) == NULL) {
		return (NULL);
	}
	font = AGFONT(fontBf);
	AG_ObjectInit(fontBf, &agFontBfClass);
	AG_ObjectSetNameS(fontBf, name);

	Strlcpy(font->name, nameBase, sizeof(font->name));
	memcpy(&font->spec, spec, sizeof(AG_FontSpec));
	font->flags |= flags;
	
	if (AGFONT_OPS(fontBf)->open(fontBf, path) == -1) {
		AG_ObjectDestroy(fontBf);
		return (NULL);
	}
	return (fontBf);
}

static int
LoadImage(AG_FontBf *fontBf, const char *file)
{
	char path[AG_PATHNAME_MAX];

	if (AG_ConfigFind(AG_CONFIG_PATH_FONTS, file, path, sizeof(path)) != 0) {
		return (-1);
	}
	if ((fontBf->S = AG_SurfaceFromFile(path)) == NULL) {
		return (-1);
	}
#ifdef DEBUG_FONTS
	Debug(fontBf, "Loaded %dx%d image from %s\n",
	    fontBf->S->w, fontBf->S->h, file);
#endif
	return (0);
}

static void
MapGlyph(AG_FontBf *fontBf, int x1, int x2, int y, int yRef, Uint nGlyph)
{
	AG_GlyphBf *G = &fontBf->glyphs[nGlyph];
	const int height = fontBf->height;

	G->ch = fontBf->unicode[nGlyph];
	G->flags = AG_GLYPH_VALID;
	G->yOffset = y - yRef;
	G->rs.x = x1 + 1;
	G->rs.y = y + 1;
	G->rs.w = (x2 - x1) - 2;
	G->rs.h = height - 2;

#ifdef DEBUG_FONTS
	Debug(fontBf,
	    "New Glyph #%d: Ch=0x%lx ('%c') yOffs=%d (ref=%d) rs=[%d,%d %dx%d]\n",
	    nGlyph, (Ulong)G->ch, (char)G->ch,
	    G->yOffset, yRef,
	    G->rs.x, G->rs.y, G->rs.w, G->rs.h);
#endif
}

/* Scan the image for reference and glyph bounding boxes. */
static int
ScanImage(AG_FontBf *fontBf)
{
	AG_Surface *S = fontBf->S;
	Uint8 *p, *pScanBegin;
	Uint32 px, pxBB;
	Uint nGlyph=0, nRow=0;
	int x = 0, y, yRef, yBeg, yEnd;

	pxBB = AG_MapPixel32_RGBA8(&S->format, 0,0,0, 255);

	/* Find the first pixel of the reference bounding box of this row. */
	y = 0;
	p = S->pixels;
scan_row:
	for (; y < S->h; y++) {
		px = AG_SurfaceGet32_At(S,p);
		if (px == pxBB) {
			yBeg = y;
			break;
		}
		p += S->pitch;
	}
	if (y == S->h) {
		AG_SetErrorS("Bounding box not found");
		return (-1);
	}
	yRef = y;

	/* Measure the height of the bounding box. */
	p += S->pitch;
	y++;
	for (; y < S->h; y++) {
		px = AG_SurfaceGet32_At(S, p);

		if (px != pxBB) {
			yEnd = y;
			--y;
			p -= S->pitch;
			break;
		}
		p += S->pitch;
	}
	if (y == S->h)
		goto bad_bbox;

	/* Measure the width of the bounding box. */
	p += S->format.BytesPerPixel;
	for (x = 1; x < S->w; x++) {
		px = AG_SurfaceGet32_At(S, p);
		if (px != pxBB) {
			break;
		}
		p += S->format.BytesPerPixel;
	}
	if (x == S->w) {
		goto bad_bbox;
	}
	if (nRow == 0) {
		fontBf->height = (yEnd - yBeg);
		fontBf->wdRef = x;
	}

	/*
	 * Scan the glyphs in this row.
	 */

	/* Establish the beginnning of the vertical scanning area. */
	yBeg = yBeg - fontBf->height + 1;
	if (yBeg < 0) { yBeg = 0; }
	pScanBegin = S->pixels + (yBeg * S->pitch);

	/*
	 * Vertical downwards scan the established area left to right until
	 * a bounding box edge is found. Bounding box edges must be separated
	 * by at least 1px.
	 */
	for (x++; x < S->w - 1; x++) {
		for (y = yBeg, p = pScanBegin + (x * S->format.BytesPerPixel);
		     y < yEnd;
		     y++) {
			px = AG_SurfaceGet32_At(S, p);
			if (px == pxBB) {               /* BBox upper left? */
				break;
			}
			p += S->pitch;
		}
		if (y < yEnd) {                               /* BBox found */
			int xBox, diff;

			/* Measure the width of the BBox. */
			for (xBox = x+1, p += S->format.BytesPerPixel;
			     xBox < S->w;
			     xBox++) {
				px = AG_SurfaceGet32_At(S, p);
				if (px != pxBB) {
					break;
				}
				p += S->format.BytesPerPixel;
			}
			if (xBox == S->w)
				goto bad_bbox;

			/* Map the glyph at this location. */
			MapGlyph(fontBf, x, xBox, y, yRef, nGlyph);

			if (++nGlyph >= fontBf->nUnicode) {
				/* All unicode mappings are satisfied. */
				goto success;
			}
			diff = (xBox - x);
			x += diff;
			p += diff * S->format.BytesPerPixel;
		}
	}
	nRow++;
	if (y + fontBf->height < S->h) {              /* Next row possible? */
		y = (yEnd + (fontBf->height >> 1));
		p = S->pixels + (y * S->pitch);
		goto scan_row;                         /* Scan the next row */
	}
	if (fontBf->nGlyphs < fontBf->nUnicode) {
		AG_SetError("No unicode for glyph #%d", fontBf->nGlyphs+1);
		return (-1);
	}
success:
	fontBf->nGlyphs = fontBf->nUnicode;
#ifdef DEBUG_FONTS
	Debug(fontBf, "Bitmap Font OK (%d glyphs in %d rows, "
	              "height = %d, ref.wd = %d)\n",
	    fontBf->nGlyphs, nRow+1,
	    fontBf->height, fontBf->wdRef);
#endif
	return (0);
bad_bbox:
	AG_SetError("Unterminated bounding box at [%d,%d]", x, y);
	return (-1);
}

static int
Open(void *_Nonnull obj, const char *_Nonnull path)
{
	AG_FontBf *fontBf = obj;
	AG_Font *font = AGFONT(fontBf);
	const AG_FontSpec *spec = &font->spec;
	AG_Size size;
	char *buf, *sBuf, *line;
	int nLine=1, inUnicodeBlock=0, inMatchingSize=0, inMatchingFlags=0;
	int maxUnicode;

	if (spec->sourceType == AG_FONT_SOURCE_FILE) {
		AG_DataSource *ds;
#ifdef DEBUG_FONTS
		Debug(fontBf, "Open(%s)\n", path);
#endif
		if ((ds = AG_OpenFile(path, "rb")) == NULL) {
			return (-1);
		}
		if (AG_Seek(ds, 0, AG_SEEK_END) != 0) {
			AG_CloseFile(ds);
			return (-1);
		}
		size = (AG_Size)AG_Tell(ds);
		if (AG_Seek(ds, 0, AG_SEEK_SET) != 0) {
			AG_CloseFile(ds);
			return (-1);
		}
		if ((buf = TryMalloc(size+1)) == NULL) {
			AG_CloseFile(ds);
			return (-1);
		}
		if (AG_Read(ds, buf, size) == -1) {
			free(buf);
			AG_CloseFile(ds);
			return (-1);
		}
		AG_CloseFile(ds);
	} else if (spec->sourceType == AG_FONT_SOURCE_MEMORY) {
		size = spec->source.mem.size;
		if ((buf = TryMalloc(size+1)) == NULL) {
			return (-1);
		}
		memcpy(buf, spec->source.mem.data, size);
	} else {
		AG_SetErrorS("Bad font source");
		return (-1);
	}
	buf[size] = '\0';

	sBuf = buf;
	while ((line = Strsep(&sBuf, "\n\r")) != NULL) {
		if (line[0] == '\0' ||
		    (line[0] == '/' && line[1] == '*') || 
		    (line[0] == ' ' && line[1] == '*') || 
		    (line[0] == ' ' && line[1] == '*' && line[2] == '/')) {
			goto next_line;
		}
		if (inUnicodeBlock) {
			char *lineBuf, *tok, *sToks;
			/*
			 * Scope: Unicode mappings block.
			 */
			if (line[0] == '.') {
				inUnicodeBlock = 0;
				goto next_line;
			}
			lineBuf = Strdup(line);
			sToks = lineBuf;
			while ((tok = Strsep(&sToks, " ")) != NULL) {
				if (tok[0] == '\0') {
					continue;
				}
				if (fontBf->nUnicode+1 > maxUnicode) {
					maxUnicode <<= 1;
					fontBf->unicode = TryRealloc(fontBf->unicode,
					    (maxUnicode*sizeof(AG_Char)));
					if (fontBf->unicode == NULL) {
						free(lineBuf);
						goto fail;
					}
				}
				fontBf->unicode[fontBf->nUnicode++] =
				    (AG_Char)strtoul(tok,NULL,16);
#ifdef DEBUG_FONTS_UNICODE
				Debug(fontBf, "Map #%d: '%s' -> 0x%x ('%c')\n",
				    fontBf->nUnicode-1,
				    tok, 
				    (Uint)fontBf->unicode[fontBf->nUnicode-1],
				    (char)fontBf->unicode[fontBf->nUnicode-1]);
#endif
			}
			free(lineBuf);
			goto next_line;
		}
		if (inMatchingSize && inMatchingFlags) {
			/*
			 * Scope: Matching font variant.
			 */
			if (strncmp(line,"file \"",6) == 0 && line[6] != '\0') {
				const char *file = &line[6];
				char *fileEnd;

				if ((fileEnd = strrchr(file, '"')) == NULL) {
					goto syntax_error;
				}
				*fileEnd = '\0';
				if (LoadImage(fontBf, file) != 0) {
					AG_Verbose(AGSI_ITALIC "%s" AGSI_RST ": "
        					   "line %d: File(%s): %s\n",
						   OBJECT(fontBf)->name, nLine,
						   file, AG_GetError());
					continue;
				}
			} else if (strncmp(line,"underline-position ",18) == 0 && line[18] != '\0') {
				fontBf->underlinePos = (int)strtoul(&line[18],NULL,10);
#ifdef DEBUG_FONTS
				Debug(fontBf, "Underline pos: %d\n", fontBf->underlinePos);
#endif
			} else if (strncmp(line,"underline-thickness ",19) == 0 && line[19] != '\0') {
				fontBf->underlineThick = (int)strtoul(&line[19],NULL,10);
#ifdef DEBUG_FONTS
				Debug(fontBf, "Underline thickness: %d\n", fontBf->underlineThick);
#endif
			} else if (strcmp(line,"unicode") == 0) {
				inUnicodeBlock = 1;
				maxUnicode = 32;
				fontBf->unicode = TryMalloc(maxUnicode *
				                            sizeof(AG_Char));
				if (fontBf->unicode == NULL)
					goto fail;
			}
		}

		/*
		 * Scope: Font global.
		 */
		if (strncmp(line,"name \"",6) == 0 && line[6] != '\0') {
			const char *name = &line[6];
			char *nameEnd;

			if ((nameEnd = strrchr(line, '"')) == NULL) {
				goto syntax_error;
			}
			*nameEnd = '\0';
			if ((fontBf->name = TryStrdup(name)) == NULL) {
				goto fail;
			}
#ifdef DEBUG_FONTS
			Debug(fontBf, "Name: `%s'\n", name);
#endif
		} else if (strncmp(line,"author",6) == 0 ||
		           strncmp(line,"license",7) == 0) {
			goto next_line;
		} else if (strncmp(line,"colorize ",9) == 0 && line[9] != '\0') {
			const char *colorize = &line[9];

			switch (colorize[0]) {
			case 'g':
				fontBf->colorize = AG_FONT_BF_COLORIZE_GRAYS;
				break;
			case 'a':
				fontBf->colorize = AG_FONT_BF_COLORIZE_ALL;
				break;
			default:
				fontBf->colorize = AG_FONT_BF_COLORIZE_NONE;
				break;
			}
#ifdef DEBUG_FONTS
			Debug(fontBf, "Colorization mode: %d\n", fontBf->colorize);
#endif
		} else if (strncmp(line,"size ",5) == 0 && line[5] != '\0') {
			char *ep;
			float sizeMin, sizeMax;

			sizeMin = (float)strtod(&line[5],&ep);
			if (ep == NULL || *ep != '-') {
				goto syntax_error;
			}
			sizeMax = (float)strtod(&ep[1],NULL);
			if (spec->size >= sizeMin &&
			    spec->size <= sizeMax) {
				inMatchingSize = 1;
			} else {
				inMatchingSize = 0;
			}
		} else if (strncmp(line,"flags 0x",8) == 0 && line[8] != '\0') {
			Uint fontFlags;

			fontFlags = (Uint)strtoul(&line[8],NULL,16);
			inMatchingFlags = (font->flags == fontFlags) ? 1 : 0;
#ifdef DEBUG_FONTS
			if (inMatchingFlags)
				Debug(fontBf, "Matching flags: 0x%x\n", font->flags);
#endif
		}
next_line:
		nLine++;
		continue;
syntax_error:
		AG_Verbose(AGSI_ITALIC "%s" AGSI_RST ": "
		           "line %d: Syntax error near: `%s'\n",
			   OBJECT(fontBf)->name, nLine, line);
	}
	if (fontBf->S == NULL) {
		AG_SetErrorS("No matching bitmap font size");
		goto fail;
	}

	/* Pre-allocate glyph array. ScanImage() will set nGlyphs on success. */
	fontBf->glyphs = TryMalloc(fontBf->nUnicode * sizeof(AG_GlyphBf));
	if (fontBf->glyphs == NULL) {
		goto fail;
	}
	if (ScanImage(fontBf) == -1) {
		goto fail;
	}
	font->height = fontBf->height;
	font->ascent = font->height;
	font->descent = 0;
	font->lineskip = font->height;		/* TODO */

	fontBf->flags |= AG_FONT_BF_VALID;
	free(buf);
	return (0);
fail:
	free(buf);
	return (-1);
}

static void
FlushCache(void *_Nonnull obj)
{
/* 	AG_FontBf *fontBf = obj; */

	/* No glyph cache to flush. */
}

static void
Close(void *_Nonnull obj)
{
	AG_FontBf *fontBf = obj;

	Free(fontBf->name);
	fontBf->name = NULL;
	Free(fontBf->unicode);
	fontBf->nUnicode = 0;

	fontBf->flags &= ~(AG_FONT_BF_VALID);
}

static void *
GetGlyph(void *_Nonnull obj, AG_Char ch, Uint want)
{
	AG_FontBf *fontBf = obj;
	Uint i;

	/* TODO hash table */

	for (i = 0; i < fontBf->nGlyphs; i++) {
		AG_GlyphBf *Gbf = &fontBf->glyphs[i];

		if (Gbf->ch == ch) {
#ifdef AG_DEBUG
			if ((Gbf->flags & AG_GLYPH_VALID) == 0)
				AG_FatalError("Invalid glyph");
#endif
			return (void *)(Gbf);
		}
	}
	return (NULL);
}

static void
GetGlyphMetrics(void *_Nonnull obj, AG_Glyph *G)
{
	AG_FontBf *fontBf = obj;
	AG_GlyphBf *Gbf;

	Gbf = GetGlyph(fontBf, G->ch, 0);
	if (Gbf != NULL) {
		G->advance = Gbf->rs.w + fontBf->advance;
	} else {
		G->advance = fontBf->wdRef;
	}
}

/*
 * Calculate the offset in pixels needed to align text based on the
 * current justification mode.
 */
static __inline__ int
JustifyOffset(const AG_TextState *_Nonnull ts, int w, int wLine)
{
	switch (ts->justify) {
	case AG_TEXT_LEFT:   return (0);
	case AG_TEXT_CENTER: return ((w >> 1) - (wLine >> 1));
	case AG_TEXT_RIGHT:  return (w - wLine);
	}
	return (0);
}

static void
RenderColorizeNone(const AG_Char *_Nonnull ucs, AG_Surface *_Nonnull S,
    const AG_TextMetrics *_Nonnull Tm, AG_Font *_Nonnull font,
    const AG_Color *_Nonnull cBgOrig, const AG_Color *_Nonnull cFgOrig)
{
	AG_FontBf *fontBf = AGFONTBF(font);
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	const int lineskip = font->lineskip;
	const AG_Char *c;
	AG_Rect rd;
	int line;

	AG_FillRect(S, NULL, cBgOrig);

	/* TODO colorize mode */
	(void)cFgOrig;

	rd.x = (Tm->nLines > 1) ? JustifyOffset(ts, Tm->w, Tm->wLines[0]) : 0;
	rd.y = 0;

	for (c=&ucs[0], line=0; *c != '\0'; c++) {
		AG_GlyphBf *Gbf;

		if (*c == '\n') {
			rd.y += lineskip;
			rd.x = JustifyOffset(ts, Tm->w, Tm->wLines[++line]);
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
		if ((Gbf = GetGlyph(fontBf, *c, 0)) == NULL) {
#ifdef DEBUG_FONTS
			Debug(fontBf, "Not found: `%c' (0x%x)\n", (char)*c, *c);
#endif
			rd.x += fontBf->wdRef;
			continue;
		}
		if (*c != ' ') {
			AG_SurfaceBlit(fontBf->S, &Gbf->rs, S,
			    rd.x,
			    rd.y + Gbf->yOffset);
		}
		rd.x += Gbf->rs.w + fontBf->advance;
	}
}

static void
RenderColorizeGrays(const AG_Char *_Nonnull ucs, AG_Surface *_Nonnull S,
    const AG_TextMetrics *_Nonnull Tm, AG_Font *_Nonnull font,
    const AG_Color *_Nonnull cBgOrig, const AG_Color *_Nonnull cFgOrig)
{
	/* TODO */
	RenderColorizeNone(ucs, S, Tm, font, cBgOrig, cFgOrig);
}

static void
RenderColorizeAll(const AG_Char *_Nonnull ucs, AG_Surface *_Nonnull S,
    const AG_TextMetrics *_Nonnull Tm, AG_Font *_Nonnull font,
    const AG_Color *_Nonnull cBgOrig, const AG_Color *_Nonnull cFgOrig)
{
	/* TODO */
	RenderColorizeNone(ucs, S, Tm, font, cBgOrig, cFgOrig);
}

static void
Render(const AG_Char *_Nonnull ucs, AG_Surface *_Nonnull S,
    const AG_TextMetrics *_Nonnull Tm, AG_Font *_Nonnull font,
    const AG_Color *_Nonnull cBgOrig, const AG_Color *_Nonnull cFgOrig)
{
	AG_FontBf *fontBf = AGFONTBF(font);
	static void (*pfRender[])(const AG_Char *, AG_Surface *,
	    const AG_TextMetrics *, AG_Font *, const AG_Color *,
	    const AG_Color *) = {
		RenderColorizeNone,
		RenderColorizeGrays,
		RenderColorizeAll
	};
#ifdef AG_DEBUG
	if (fontBf->colorize >= AG_FONT_BF_COLORIZE_LAST)
		AG_FatalError("Bad colorize mode");
#endif
	pfRender[fontBf->colorize](ucs, S, Tm, font, cBgOrig, cFgOrig);
}

static void
Size(const AG_Font *_Nonnull font, const AG_Char *_Nonnull ucs,
    AG_TextMetrics *_Nonnull Tm, int extended)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_FontBf *fontBf = AGFONTBF(font);
	AG_GlyphBf *Gbf;
	const AG_Char *c;
	const int tabWd = ts->tabWd;
	const int lineskip = font->lineskip;
	int wLine=0;

	for (c = &ucs[0]; *c != '\0'; c++) {
		if (*c == '\n') {
			if (extended) {
				Tm->wLines = Realloc(Tm->wLines,
				    (Tm->nLines+2)*sizeof(Uint));
				Tm->wLines[Tm->nLines++] = wLine;
				wLine = 0;
			}
			Tm->h += lineskip;
			continue;
		}
		if (*c == '\t') {
			wLine += tabWd;
			Tm->w += tabWd;
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

		if ((Gbf = GetGlyph(fontBf, *c, 0)) != NULL) {
			const int advance = Gbf->rs.w + fontBf->advance;

			wLine += advance;
			Tm->w += advance;
			Tm->h = MAX(Tm->h, Gbf->rs.h);
		} else {
			wLine += fontBf->wdRef;
			Tm->w += fontBf->wdRef;
			Tm->h = MAX(Tm->h, fontBf->height);
		}
	}
	if (*c != '\n' && extended) {
		if (Tm->nLines > 0) {
			Tm->wLines = Realloc(Tm->wLines,
			    (Tm->nLines + 2) * sizeof(Uint));
			Tm->wLines[Tm->nLines] = wLine;
		}
		Tm->nLines++;
	}
}

static void
Init(void *_Nonnull obj)
{
	AG_FontBf *fontBf = obj;

	memset(&fontBf->flags, 0,
	    sizeof(Uint) +                           /* flags */
	    sizeof(enum ag_font_bf_colorize_mode) +  /* colorize */
	    sizeof(char *) +                         /* name */
	    sizeof(int) +                            /* underlinePos */
	    sizeof(int) +                            /* underlineThick */
	    sizeof(AG_Char *) +                      /* unicode */
	    sizeof(Uint) +                           /* nUnicode */
	    sizeof(Uint) +                           /* nGlyphs */
	    sizeof(AG_GlyphBf *) +                   /* glyphs */
	    sizeof(AG_Surface *) +                   /* S */
	    sizeof(int) +                            /* height */
	    sizeof(int) +                            /* wdRef */
	    sizeof(AG_Rect *) +                      /* rects */
	    sizeof(Uint));                           /* nRects */

	fontBf->advance = 1;
}

AG_FontClass agFontBfClass = {
	{
		"Agar(Font:FontBf)",
		sizeof(AG_FontBf),
		{ 0, 0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Open,
	FlushCache,
	Close,
	GetGlyph,
	GetGlyphMetrics,
	Render,
	Size
};

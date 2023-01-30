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

#define DEBUG_FONTS

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
	Debug(fontBf, "Loaded %dx%d image from %s\n",
	    fontBf->S->w, fontBf->S->h, file);

	return (0);
}

static int
Open(void *_Nonnull obj, const char *_Nonnull path)
{
	AG_FontBf *fontBf = obj;
	AG_Font *font = AGFONT(fontBf);
	const AG_FontSpec *spec = &font->spec;
	AG_Size size;
	char *buf, *sBuf, *line;
	AG_Surface *S;
	Uint8 *p;
	Uint32 pxBlack;
	int nLine=1, inUnicodeBlock=0, inMatchingSize=0, maxUnicode;
	int x,y, yBox;

	if (spec->sourceType == AG_FONT_SOURCE_FILE) {
		AG_DataSource *ds;

		Debug(fontBf, "Open(%s)\n", path);
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
//				Debug(fontBf, "tok '%s' -> 0x%x ('%c')\n",
//				    tok, 
//				    (Uint)fontBf->unicode[fontBf->nUnicode-1],
//				    (char)fontBf->unicode[fontBf->nUnicode-1]);
			}
			free(lineBuf);
			goto next_line;
		}
		if (inMatchingSize) {
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
			} else if (strncmp(line,"flags 0x",8) == 0 && line[8] != '\0') {
				font->flags = (Uint)strtoul(&line[8],NULL,16);
				Debug(fontBf, "Font flags: 0x%x\n", font->flags);
			} else if (strncmp(line,"underline-position ",18) == 0 && line[18] != '\0') {
				fontBf->underlinePos = (int)strtoul(&line[18],NULL,10);
				Debug(fontBf, "Underline pos: %d\n", fontBf->underlinePos);
			} else if (strncmp(line,"underline-thickness ",19) == 0 && line[19] != '\0') {
				fontBf->underlineThick = (int)strtoul(&line[19],NULL,10);
				Debug(fontBf, "Underline thickness: %d\n", fontBf->underlineThick);
			} else if (strcmp(line,"unicode") == 0) {
				inUnicodeBlock = 1;
				maxUnicode = 32;
				fontBf->unicode = TryMalloc(maxUnicode *
				                            sizeof(AG_Char));
				if (fontBf->unicode == NULL)
					goto fail;
			}
		}
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
			Debug(fontBf, "Name: `%s'\n", name);
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
		}
next_line:
		nLine++;
		continue;
syntax_error:
		AG_Verbose(AGSI_ITALIC "%s" AGSI_RST ": "
		           "line %d: Syntax error near: `%s'\n",
			   OBJECT(fontBf)->name, nLine, line);
	}
	if ((S = fontBf->S) == NULL) {
		AG_SetErrorS("No matching bitmap font size");
		goto fail;
	}
	p = S->pixels;
	(void)x;
//	pxBlack = AG_MapPixel32_RGBA8(&S->format, 0,0,0, 255);
#if AG_BYTEORDER == AG_BIG_ENDIAN
	pxBlack = 0x000000ff;
#else
	pxBlack = 0xff000000;
#endif
	for (y = 0;
	     y < S->h;
	     y++, p += S->pitch) {
		Uint32 px;
		Uint8 r,g,b,a;

		if ((px = AG_SurfaceGet32_At(S,p)) != pxBlack) {
			Debug(fontBf, "px=0x%x c\n", px);
			continue;
		}
		AG_GetColor32_RGBA8(px, &S->format, &r,&g,&b,&a);
		Debug(fontBf, "px %d,%d,%d,%d\n", r,g,b,a);
		Debug(fontBf, "px=0x%x b (blk=0x%x)\n", px, pxBlack);
		Debug(fontBf, "Amask=0x%08lx\n", S->format.Amask);
		Debug(fontBf, "Ashift=%d\n", S->format.Ashift);
		Debug(fontBf, "Aloss=%d\n", S->format.Aloss);
		break;
	}
	if (y == S->h) {
		AG_SetErrorS("Bounding box not found");
		goto fail;
	}
	Debug(fontBf, "Bounding box at %d (pxBlack=%x)\n", y, pxBlack);
	p += S->pitch;
	y++;
	for (yBox = y; yBox < S->h; yBox++) {
		Uint32 pxBox = AG_SurfaceGet32_At(S, p);

		Debug(fontBf, "pxBox (%d) = 0x%x\n", yBox, pxBox);
		if (pxBox != pxBlack) {
			Debug(fontBf, "box break = %d\n", yBox);
			break;
		}
		p += S->pitch;
	}

#if 0
	font->height = fontBf->glyphs[0]->h;
	font->ascent = font->height;
	font->descent = 0;
	font->lineskip = font->height;
#endif
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
//	return (void *)GetGlyphSurface((const AG_FontBf *)obj, ch);
	return (NULL);
}

static void
GetGlyphMetrics(void *_Nonnull obj, AG_Glyph *G)
{
//	G->advance = G->su->w;
	G->advance = 10;
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
Render(const AG_Char *_Nonnull ucs, AG_Surface *_Nonnull S,
    const AG_TextMetrics *_Nonnull Tm, AG_Font *_Nonnull font,
    const AG_Color *_Nonnull cBgOrig, const AG_Color *_Nonnull cFgOrig)
{
//	AG_FontBf *fontBf = AGFONTBF(font);
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	const int lineskip = font->lineskip;
//	AG_Surface *Sglyph;
	const AG_Char *c;
	AG_Rect rd;
	int line;

	AG_FillRect(S, NULL, cBgOrig);

	/* TODO colorize mode */
	(void)cFgOrig;

	rd.x = (Tm->nLines > 1) ? JustifyOffset(ts, Tm->w, Tm->wLines[0]) : 0;
	rd.y = 0;

	for (c=&ucs[0], line=0; *c != '\0'; c++) {
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
#if 0
		Sglyph = GetGlyphSurface(fontBf, *c);
		if (*c != ' ') {
			Uint8 *pSrc = Sglyph->pixels;
			int x,y;

			for (y = 0; y < Sglyph->h; y++) {
				Uint8 *pDst = S->pixels +
				    (rd.y + y)*S->pitch +
				     rd.x*S->format.BytesPerPixel;

				for (x = 0; x < Sglyph->w; x++) {
					const AG_Pixel pxGlyph =
					    AG_SurfaceGet32_At(Sglyph,pSrc);
					AG_Color c;

					AG_GetColor(&c, pxGlyph,
					    &Sglyph->format);
	
					if (c.a != 0) {
						Debug(NULL, "GetColor(%d,%d,%d,%d)\n",
						    c.r, c.g, c.b, c.a);
						AG_SurfacePut_At(S, pDst,
						    AG_MapPixel(&S->format,
						        cFgOrig));
					}

					pSrc += Sglyph->format.BytesPerPixel;
					pDst += S->format.BytesPerPixel;
				}
				pSrc += Sglyph->padding;
				pDst += S->padding;
			}
		}
		rd.x += Sglyph->w;
#endif
	}
}

static void
Size(const AG_Font *_Nonnull font, const AG_Char *_Nonnull ucs,
    AG_TextMetrics *_Nonnull Tm, int extended)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
//	AG_FontBf *fontBf = AGFONTBF(font);
	const AG_Char *c;
//	AG_Surface *Sglyph;
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
#if 0
		Sglyph = GetGlyphSurface(fontBf, *c);
		wLine += Sglyph->w;
		Tm->w += Sglyph->w;
		Tm->h = MAX(Tm->h, Sglyph->h);
#endif
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
	    sizeof(enum ag_font_bmp_colorize_mode) + /* colorize */
	    sizeof(char *) +                         /* name */
	    sizeof(int) +                            /* underlinePos */
	    sizeof(int) +                            /* underlineThick */
	    sizeof(AG_Char *) +                      /* unicode */
	    sizeof(Uint) +                           /* nUnicode */
	    sizeof(Uint) +                           /* nGlyphs */
	    sizeof(AG_GlyphBf *) +                   /* glyphs */
	    sizeof(AG_Surface *));                   /* S */
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

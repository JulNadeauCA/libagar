/*	$Csoft: uniconv.c,v 1.4 2004/03/30 16:32:51 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>

#ifdef DEBUG

#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/combo.h>
#include <engine/widget/tlist.h>

#include <engine/unicode/unicode.h>

#include "monitor.h"

static Uint32 unitext[2] = { '\0', '\0' };
static char utf8text[256] = "";
static char bytetext[256] = "";

static const struct unicode_range {
	Uint32 start;
	char *name;
} unicode_ranges[] = {
	{ 0x0000, "Basic Latin" },
	{ 0x0080, "C1 Controls and Latin-1 Supplement" },
	{ 0x0100, "Latin Extended-A" },
	{ 0x0180, "Latin Extended-B" },
	{ 0x0250, "IPA Extensions" },
	{ 0x02B0, "Spacing Modifier Letters" },
	{ 0x0300, "Combining Diacritical Marks" },
	{ 0x0370, "Greek and Coptic" },
	{ 0x0400, "Cyrillic" },
	{ 0x0500, "Cyrillic Supplement" },
	{ 0x0530, "Armenian" },
	{ 0x0590, "Hebrew" },
	{ 0x0600, "Arabic" },
	{ 0x0700, "Syriac" },
	{ 0x0750, "" },
	{ 0x0780, "Thaana" },
	{ 0x07c0, "" },
	{ 0x0900, "Devanagari" },
	{ 0x0980, "Bengali/Assamese" },
	{ 0x0A00, "Gurmukhi" },
	{ 0x0A80, "Gujarati" },
	{ 0x0B00, "Oriya" },
	{ 0x0B80, "Tamil" },
	{ 0x0C00, "Telugu" },
	{ 0x0C80, "Kannada" },
	{ 0x0D00, "Malayalam" },
	{ 0x0D80, "Sinhala" },
	{ 0x0E00, "Thai" },
	{ 0x0E80, "Lao" },
	{ 0x0F00, "Tibetan" },
	{ 0x1000, "Myanmar" },
	{ 0x10A0, "Georgian" },
	{ 0x1100, "Hangul Jamo" },
	{ 0x1200, "Ethiopic" },
	{ 0x1380, "" },
	{ 0x13A0, "Cherokee" },
	{ 0x1400, "Unified Canadian Aboriginal Syllabics" },
	{ 0x1680, "Ogham" },
	{ 0x16A0, "Runic" },
	{ 0x1700, "Tagalog" },
	{ 0x1720, "Hanunoo" },
	{ 0x1740, "Buhid" },
	{ 0x1760, "Tagbanwa" },
	{ 0x1780, "Khmer" },
	{ 0x1800, "Mongolian" },
	{ 0x18b0, "" },
	{ 0x1900, "Limbu" },
	{ 0x1950, "Tai Le" },
	{ 0x1980, "" },
	{ 0x19E0, "Khmer Symbols" },
	{ 0x1A00, "" },
	{ 0x1D00, "Phonetic Extensions" },
	{ 0x1D80, "" },
	{ 0x1E00, "Latin Extended Additional" },
	{ 0x1F00, "Greek Extended" },
	{ 0x2000, "General Punctuation" },
	{ 0x2070, "Superscripts and Subscripts" },
	{ 0x20A0, "Currency Symbols" },
	{ 0x20D0, "Combining Diacritical Marks for Symbols" },
	{ 0x2100, "Letterlike Symbols" },
	{ 0x2150, "Number Forms" },
	{ 0x2190, "Arrows" },
	{ 0x2200, "Mathematical Operators" },
	{ 0x2300, "Miscellaneous Technical" },
	{ 0x2400, "Control Pictures" },
	{ 0x2440, "Optical Character Recognition" },
	{ 0x2460, "Enclosed Alphanumerics" },
	{ 0x2500, "Box Drawing" },
	{ 0x2580, "Block Elements" },
	{ 0x25A0, "Geometric Shapes" },
	{ 0x2600, "Miscalleneous Symbols" },
	{ 0x2700, "Dingbats" },
	{ 0x27C0, "Miscellaneous Mathematical Symbols-A" },
	{ 0x27F0, "Supplemental Arrows-A" },
	{ 0x2800, "Braille Patterns" },
	{ 0x2900, "Supplemental Arrows-B" },
	{ 0x2980, "Miscellaneous Mathematical Symbols-B" },
	{ 0x2A00, "Supplemental Mathematical Operators" },
	{ 0x2B00, "Miscellaneous Symbols and Arrows" },
	{ 0x2C00, "" },
	{ 0x2E80, "CJK Radicals Supplement" },
	{ 0x2F00, "Kangxi Radicals" },
	{ 0x2FE0, "" },
	{ 0x2FF0, "Ideographic Description Characters" },
	{ 0x3000, "CJK Symbols and Punctuation" },
	{ 0x3040, "Hiragana" },
	{ 0x30A0, "Katakana" },
	{ 0x3100, "Bopomofo" },
	{ 0x3130, "Hangul Compatibility Jamo" },
	{ 0x3190, "Kanbun (Kunten)" },
	{ 0x31A0, "Bopomofo Extended" },
	{ 0x31C0, "" },
	{ 0x31F0, "Katakana Phonetic Extensions" },
	{ 0x3200, "Enclosed CKJ Letters and Months" },
	{ 0x3300, "CJK Compatibility" },
	{ 0x3400, "CJK Unified Ideographs Extension A" },
	{ 0x4DC0, "Yijing Hexagram Symbols" },
	{ 0x4E00, "CJK Unified Ideographs" },
	{ 0x9FB0, "" },
	{ 0xA000, "Yi Syllables" },
	{ 0xA490, "Yi Radicals" },
	{ 0xA4D0, "" },
	{ 0xAC00, "Hangul Syllables" },
	{ 0xD7B0, "" },
	{ 0xD800, "High Surrogate Area" },
	{ 0xDC00, "Low Surrogate Area" },
	{ 0xE000, "Private Use Area" },
	{ 0xF900, "CJK Compatibility Ideographs" },
	{ 0xFB00, "Alphabetic Presentation Forms" },
	{ 0xFB50, "Arabic Presentation Forms-A" },
	{ 0xFE00, "Variation Selectors" },
	{ 0xFE10, "" },
	{ 0xFE20, "Combining Half Marks" },
	{ 0xFE30, "CJK Compatibility Forms" },
	{ 0xFE50, "Small Form Variants" },
	{ 0xFE70, "Arabic Presentation Forms-B" },
	{ 0xFF00, "Halfwidth and Fullwidth Forms" },
	{ 0xFFF0, "Specials" },
};
const int nunicode_ranges = sizeof(unicode_ranges) / sizeof(unicode_ranges[0]);


static void
select_character(int argc, union evarg *argv)
{
	int i;
	char *c;

	unicode_export(UNICODE_TO_UTF8, utf8text, unitext, sizeof(unitext));

	bytetext[0] = '\0';
	for (c = &utf8text[0]; *c != '\0'; c++) {
		char s[4];

		snprintf(s, sizeof(s), " %x", (unsigned char)*c);
		strlcat(bytetext, s, sizeof(bytetext));
	}
}

static void
select_range(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tlist_item *it = argv[2].p;
	struct unicode_range *range = it->p1;
	const struct unicode_range *next_range = NULL;
	Uint32 i, end;

	for (i = 0; i < nunicode_ranges; i++) {
		if ((&unicode_ranges[i] == range) &&
		    (i+1 < nunicode_ranges)) {
			next_range = &unicode_ranges[i+1];
			break;
		}
	}
	end = (next_range != NULL) ? next_range->start-1 : 0xffff;

	tlist_clear_items(tl);
	for (i = range->start; i < next_range->start-1; i++) {
		char text[TLIST_LABEL_MAX];
		char *c;

		if (i == 10)
			continue;

		unitext[0] = i;
		unicode_export(UNICODE_TO_UTF8, utf8text, unitext,
		    sizeof(unitext));

		bytetext[0] = '\0';
		for (c = &utf8text[0]; *c != '\0'; c++) {
			char s[4];

			snprintf(s, sizeof(s), " %x", (unsigned char)*c);
			strlcat(bytetext, s, sizeof(bytetext));
		}
		snprintf(text, sizeof(text), "%s (%s )", utf8text, bytetext);
		tlist_insert_item(tl, NULL, text, NULL);
	}
}

struct window *
uniconv_window(void)
{
	struct window *win;
	struct spinbutton *sbu;
	struct combo *com;
	struct tlist *tl;
	int i;

	if ((win = window_new("uniconv")) == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Unicode Conversion"));
	window_set_closure(win, WINDOW_DETACH);
	
	label_new(win, LABEL_POLLED, "%s (%s )", &utf8text, &bytetext);

	sbu = spinbutton_new(win, "Unicode: ");
	spinbutton_set_min(sbu, 0);
	spinbutton_set_max(sbu, 65537);
	widget_bind(sbu, "value", WIDGET_UINT32, &unitext[0]);
	event_new(sbu, "spinbutton-changed", select_character, NULL);
	
	com = combo_new(win, 0, _("Range: "));
	for (i = 0; i < nunicode_ranges; i++) {
		tlist_insert_item(com->list, NULL, unicode_ranges[i].name,
		    &unicode_ranges[i]);
	}
	tl = tlist_new(win, 0);
	event_new(com, "combo-selected", select_range, "%p", tl);
	return (win);
}

#endif	/* DEBUG */

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
 * Adjustments to the scaling and baselines of fonts relative to the
 * default font (Algue). Also indicate when given styles of a font are also
 * considered the Regular or Normal style of that font family.
 */
const AG_FontAdjustment agFontAdjustments[] = {
/*                                   Pts From:   0.0 10.4 14.0 21.0 23.8 35.0 */
/*                                         To:  10.4 14.0 21.0 23.8 35.0 +inf */
	{ "charter.otf",                  1.1f, { -3,  -2,  -2,  -3,  -3,  -6 }, 0,0 },
	{ "courier",                      1.0f, {  0,   0,  +1,  +1,  +2,  +3 }, 0,0 },
	{ "courier 10 pitch",             1.0f, {  0,   0,  +1,  +1,  +2,  +3 }, 0,0 },
	{ "dejavu sans",                  1.0f, { -4,  -4,  -6,  -7,  -9, -16 }, 0,0 },
	{ "dejavu sans mono",             1.0f, { -2,  -2,  -2,   0,   0,  -1 }, 0,0 },
	{ "dejavu serif",                 1.0f, { -2,  -3,  -3,  -4,  -5,  -7 }, 0,0 },
	{ "droid arabic kufi",            1.0f, {  0,   0,   0,  +1,  +2,  +3 }, 0,0 },
	{ "droid arabic naskh",           1.0f, {  0,   0,   0,  +1,  +3, +10 }, 0,0 },
	{ "droid sans",                   1.0f, {  0,  -1,  -1,  -1,  -2,  -4 }, 0,0 },
	{ "droid sans arabic",            1.0f, {  0,   0,   0,   0,  +1,  +2 }, 0,0 },
	{ "droid sans armenian",          1.0f, {  0,   0,   0,   0,  +1,  +3 }, 0,0 },
	{ "droid sans japanese",          1.0f, {  0,   0,  +1,  +1,  +4,  +9 }, 0,0 },
	{ "droid sans hebrew",            1.0f, {  0,   0,   0,  -1,  -2,  -4 }, 0,0 },
	{ "droid sans mono",              1.0f, {  0,   0,  -1,  -2,  -4,  -5 }, 0,0 },
	{ "gentium basic",                1.0f, { -2,  -3,  -3,  -4,  -5,  -7 }, 0,0 },
	{ "gentium book basic",           1.0f, { -2,  -3,  -3,  -4,  -5,  -7 }, 0,0 },
	{ "goha-tibeb zemen",             1.0f, { -3,  -3,  -5,  -7,  -8, -13 }, 0,0 },
/*                                  Pts From:   0.0 10.4 14.0 21.0 23.8 35.0 */
/*                                        To:  10.4 14.0 21.0 23.8 35.0 +inf */
	{ "league-gothic-"
	  "condensed-italic.otf",         1.1f, { -1,  -1,   0,  -1,  -2,  -4 }, AG_FONT_BOLD,0 },
	{ "league-gothic-condensed.otf",  1.1f, { -1,  -1,   0,  -1,  -2,  -4 }, AG_FONT_BOLD,0 },
	{ "league-gothic-italic.otf",     1.1f, { -1,  -1,   0,  -1,  -2,  -4 }, AG_FONT_BOLD,0 },
	{ "league-gothic.otf",            1.1f, { -1,  -1,   0,  -1,  -2,  -4 }, AG_FONT_BOLD,0 },
	{ "league-spartan.otf",           1.1f, { -3,  -3,  -4,  -5,  -7, -10 }, 0,0 },
	{ "league-spartan-black.otf",     1.1f, { -3,  -3,  -4,  -5,  -7, -10 }, 0,0 },
	{ "league-spartan-bold.otf",      1.1f, { -3,  -3,  -4,  -5,  -7, -10 }, 0,0 },
	{ "league-spartan-extrabold.otf", 1.1f, { -3,  -3,  -4,  -5,  -7, -10 }, 0,0 },
	{ "league-spartan-extralight.otf",1.1f, { -3,  -3,  -4,  -5,  -7, -10 }, 0,0 },
	{ "league-spartan-light.otf",     1.1f, { -3,  -3,  -4,  -5,  -7, -10 }, 0,0 },
	{ "league-spartan-semibold.otf",  1.1f, { -3,  -3,  -4,  -5,  -7, -10 }, 0,0 },
	{ "fraktur.ttf",                  1.1f, { +1,  +1,  +1,  +1,  +1,  +1 }, AG_FONT_BOLD,0 },
	{ "noto sans cjk sc",             1.0f, { -9, -11, -15, -17, -21, -42 }, 0,0 },
	{ "noto serif sc",                1.0f, { -6,  -8, -12, -14, -18, -35 }, 0,0 },
	{ "noto serif cjk sc",            1.0f, {-10, -12, -16, -18, -23, -42 }, 0,0 },
	{ "noto sans sc",                 1.0f, { -6,  -8, -13, -15, -18, -38 }, 0,0 },
	{ "noto sans symbols",            1.0f, {  0,   0,   0,   0,  -1,  -2 }, 0,0 },
	{ "noto sans mono cjk sc",        1.0f, { -6,  -9, -11, -14, -20, -38 }, 0,0 },
	{ "noto mono",                    1.0f, { -1,  -1,  -1,  -1,  -2,  -4 }, 0,0 },
	{ "serto jerusalem",              1.0f, {  0,   0,   0,   0,  +2,  +4 }, 0,0 },
	{ "serto jerusalem outline",      1.0f, {  0,   0,   0,   0,  +2,  +4 }, 0,0 },
/*                                   Pts From:   0.0 10.4 14.0 21.0 23.8 35.0 */
/*                                         To:  10.4 14.0 21.0 23.8 35.0 +inf */
	{ "serto kharput",                1.0f, {  0,   0,   0,   0,  +2,  +4 }, 0,0 },
	{ "serto malankara",              1.0f, {  0,   0,   0,   0,  +2,  +4 }, 0,0 },
	{ "serto mardin",                 1.0f, {  0,   0,   0,   0,  +2,  +4 }, 0,0 },
	{ "serto urhoy",                  1.0f, {  0,   0,   0,   0,  +2,  +4 }, 0,0 },
	{ "unialgue.ttf",                 1.0f, { -6,  -7,  -7, -10, -12, -15 }, 0,0 },
	{ "utopia",                       1.0f, {  0,   0,   0,   0,  +2,  +4 }, 0,0 },
	{ NULL,                           0.0f, {  0,   0,   0,   0,   0,   0 }, 0,0 }
};

/*
 * Aliases for font names. This table exists for performance reasons.
 * It prevents unnecessary loading of the built-in (algue) from its file copy.
 * We also map core font names to their respective files so the loader will
 * not have to cycle through (and test) every possible font file extension.
 */
const AG_FontAlias agFontAliases[] = {
	{ "algue",                          "_agFontAlgue" },
	{ "algue.ttf",                      "_agFontAlgue" },
	{ "algue-bold",                     "_agFontAlgue_Bold" },
	{ "algue-bold.ttf",                 "_agFontAlgue_Bold" },
	{ "algue-italic",                   "_agFontAlgue_Italic" },
	{ "algue-italic.ttf",               "_agFontAlgue_Italic" },
	{ "algue-bold-italic",              "_agFontAlgue_BoldItalic" },
	{ "algue-bold-italic.ttf",          "_agFontAlgue_BoldItalic" },
	{ "unialgue",                       "unialgue.ttf" },
	{ "agar-minimal",                   "agar-minimal.agbf" },
	{ "agar-ideograms",                 "agar-ideograms.agbf" },
	{ "monoalgue",                      "monoalgue.ttf" },
	{ "monoalgue-bold",                 "monoalgue-bold.ttf" },
	{ "monoalgue-italic",               "monoalgue-italic.ttf" },
	{ "monoalgue-bold-italic",          "monoalgue-bold-italic.ttf" },
	{ "charter",                        "charter.otf" },
	{ "charter-bold",                   "charter-bold.otf" },
	{ "charter-italic",                 "charter-bold-italic.otf" },
	{ "league-spartan",                 "league-spartan.otf" },
	{ "league-gothic",                  "league-gothic.otf" },
	{ "league-gothic-italic",           "league-gothic-italic.otf" },
	{ "league-gothic-condensed",        "league-gothic-condensed.otf" },
	{ "league-gothic-condensed-italic", "league-gothic-condensed-italic.otf" },
	{ "fraktur",                        "fraktur.ttf" },
	{ NULL,                             NULL }
};

/* Map fontconfig FC_STYLE names to AG_Font flags. */
const AG_FontStyleName agFontStyleNames[] = {
	/* Width variants */
	{ "UltraCondensed", AG_FONT_ULTRACONDENSED },
	{ "Condensed",      AG_FONT_CONDENSED },
	{ "SemiCondensed",  AG_FONT_SEMICONDENSED },
	{ "SemiExpanded",   AG_FONT_SEMIEXPANDED },
	{ "Expanded",       AG_FONT_EXPANDED },
	{ "UltraExpanded",  AG_FONT_ULTRAEXPANDED },
	/* Weights */
	{ "Thin",           AG_FONT_THIN },
	{ "ExtraLight",     AG_FONT_EXTRALIGHT },
	{ "Light",          AG_FONT_LIGHT },
	{ "Regular",        0 },
	{ "SemiBold",       AG_FONT_SEMIBOLD },
	{ "Bold",           AG_FONT_BOLD },
	{ "ExtraBold",      AG_FONT_EXTRABOLD },
	{ "Black",          AG_FONT_BLACK },
	/* Styles */
	{ "Oblique",        AG_FONT_OBLIQUE },
	{ "Italic",         AG_FONT_ITALIC },
	{ NULL,             0 }
};

/* Sort keys for styles, weights and width variants combinations. */
const AG_FontStyleSort agFontStyleSort[] = {
	{ AG_FONT_THIN,                                                   0 },
	{ AG_FONT_EXTRALIGHT,                                             1 },
	{ AG_FONT_LIGHT,                                                  2 },
	{ 0,                                                              3 },
	{ AG_FONT_SEMIBOLD,                                               4 },
	{ AG_FONT_BOLD,                                                   5 },
	{ AG_FONT_EXTRABOLD,                                              6 },
	{ AG_FONT_BLACK,                                                  7 },
	{ AG_FONT_OBLIQUE,                                                8 },

	{ AG_FONT_THIN       | AG_FONT_OBLIQUE,                           9 },
	{ AG_FONT_EXTRALIGHT | AG_FONT_OBLIQUE,                          10 },
	{ AG_FONT_LIGHT      | AG_FONT_OBLIQUE,                          11 },
	{ AG_FONT_SEMIBOLD   | AG_FONT_OBLIQUE,                          12 },
	{ AG_FONT_BOLD       | AG_FONT_OBLIQUE,                          13 },
	{ AG_FONT_EXTRABOLD  | AG_FONT_OBLIQUE,                          14 },
	{ AG_FONT_BLACK      | AG_FONT_OBLIQUE,                          15 },
	{ AG_FONT_ITALIC,                                                16 },
	{ AG_FONT_THIN       | AG_FONT_ITALIC,                           17 },
	{ AG_FONT_EXTRALIGHT | AG_FONT_ITALIC,                           18 },
	{ AG_FONT_LIGHT      | AG_FONT_ITALIC,                           19 },
	{ AG_FONT_SEMIBOLD   | AG_FONT_ITALIC,                           20 },
	{ AG_FONT_BOLD       | AG_FONT_ITALIC,                           21 },
	{ AG_FONT_EXTRABOLD  | AG_FONT_ITALIC,                           22 },
	{ AG_FONT_BLACK      | AG_FONT_ITALIC,                           23 },

	{ AG_FONT_ULTRACONDENSED,                                        32 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_OBLIQUE,                      33 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_ITALIC,                       34 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_THIN,                         35 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_THIN | AG_FONT_OBLIQUE,       36 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_THIN | AG_FONT_ITALIC,        37 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_EXTRALIGHT,                   38 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_EXTRALIGHT | AG_FONT_OBLIQUE, 39 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_EXTRALIGHT | AG_FONT_ITALIC,  40 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_LIGHT,                        41 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_LIGHT | AG_FONT_OBLIQUE,      42 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_LIGHT | AG_FONT_ITALIC,       43 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_SEMIBOLD,                     44 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_SEMIBOLD | AG_FONT_OBLIQUE,   45 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_SEMIBOLD | AG_FONT_ITALIC,    46 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_BOLD,                         47 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_BOLD | AG_FONT_OBLIQUE,       48 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_BOLD | AG_FONT_ITALIC,        49 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_EXTRABOLD,                    50 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_EXTRABOLD | AG_FONT_OBLIQUE,  51 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_EXTRABOLD | AG_FONT_ITALIC,   52 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_BLACK,                        53 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_BLACK | AG_FONT_OBLIQUE,      54 },
	{ AG_FONT_ULTRACONDENSED | AG_FONT_BLACK | AG_FONT_ITALIC,       55 },

	{ AG_FONT_CONDENSED,                                             56 },
	{ AG_FONT_CONDENSED | AG_FONT_OBLIQUE,                           57 },
	{ AG_FONT_CONDENSED | AG_FONT_ITALIC,                            58 },
	{ AG_FONT_CONDENSED | AG_FONT_THIN,                              59 },
	{ AG_FONT_CONDENSED | AG_FONT_THIN | AG_FONT_OBLIQUE,            60 },
	{ AG_FONT_CONDENSED | AG_FONT_THIN | AG_FONT_ITALIC,             61 },
	{ AG_FONT_CONDENSED | AG_FONT_EXTRALIGHT,                        62 },
	{ AG_FONT_CONDENSED | AG_FONT_EXTRALIGHT | AG_FONT_OBLIQUE,      63 },
	{ AG_FONT_CONDENSED | AG_FONT_EXTRALIGHT | AG_FONT_ITALIC,       64 },
	{ AG_FONT_CONDENSED | AG_FONT_LIGHT,                             65 },
	{ AG_FONT_CONDENSED | AG_FONT_LIGHT | AG_FONT_OBLIQUE,           66 },
	{ AG_FONT_CONDENSED | AG_FONT_LIGHT | AG_FONT_ITALIC,            67 },
	{ AG_FONT_CONDENSED | AG_FONT_SEMIBOLD,                          68 },
	{ AG_FONT_CONDENSED | AG_FONT_SEMIBOLD | AG_FONT_OBLIQUE,        69 },
	{ AG_FONT_CONDENSED | AG_FONT_SEMIBOLD | AG_FONT_ITALIC,         70 },
	{ AG_FONT_CONDENSED | AG_FONT_BOLD,                              71 },
	{ AG_FONT_CONDENSED | AG_FONT_BOLD | AG_FONT_OBLIQUE,            72 },
	{ AG_FONT_CONDENSED | AG_FONT_BOLD | AG_FONT_ITALIC,             73 },
	{ AG_FONT_CONDENSED | AG_FONT_EXTRABOLD,                         74 },
	{ AG_FONT_CONDENSED | AG_FONT_EXTRABOLD | AG_FONT_OBLIQUE,       75 },
	{ AG_FONT_CONDENSED | AG_FONT_EXTRABOLD | AG_FONT_ITALIC,        76 },
	{ AG_FONT_CONDENSED | AG_FONT_BLACK,                             77 },
	{ AG_FONT_CONDENSED | AG_FONT_BLACK | AG_FONT_OBLIQUE,           78 },
	{ AG_FONT_CONDENSED | AG_FONT_BLACK | AG_FONT_ITALIC,            79 },

	{ AG_FONT_SEMICONDENSED,                                         80 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_OBLIQUE,                       81 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_ITALIC,                        82 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_THIN,                          83 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_THIN | AG_FONT_OBLIQUE,        84 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_THIN | AG_FONT_ITALIC,         85 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_EXTRALIGHT,                    86 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_EXTRALIGHT | AG_FONT_OBLIQUE,  87 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_EXTRALIGHT | AG_FONT_ITALIC,   88 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_LIGHT,                         89 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_LIGHT | AG_FONT_OBLIQUE,       90 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_LIGHT | AG_FONT_ITALIC,        91 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_SEMIBOLD,                      92 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_SEMIBOLD | AG_FONT_OBLIQUE,    93 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_SEMIBOLD | AG_FONT_ITALIC,     94 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_BOLD,                          95 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_BOLD | AG_FONT_OBLIQUE,        96 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_BOLD | AG_FONT_ITALIC,         97 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_EXTRABOLD,                     98 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_EXTRABOLD | AG_FONT_OBLIQUE,   99 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_EXTRABOLD | AG_FONT_ITALIC,   100 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_BLACK,                        101 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_BLACK | AG_FONT_OBLIQUE,      102 },
	{ AG_FONT_SEMICONDENSED | AG_FONT_BLACK | AG_FONT_ITALIC,       103 },

	{ AG_FONT_SEMIEXPANDED,                                         104 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_OBLIQUE,                       105 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_ITALIC,                        106 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_THIN,                          107 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_THIN | AG_FONT_OBLIQUE,        108 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_THIN | AG_FONT_ITALIC,         109 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_EXTRALIGHT,                    110 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_EXTRALIGHT | AG_FONT_OBLIQUE,  111 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_EXTRALIGHT | AG_FONT_ITALIC,   112 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_LIGHT,                         113 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_LIGHT | AG_FONT_OBLIQUE,       114 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_LIGHT | AG_FONT_ITALIC,        115 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_SEMIBOLD,                      116 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_SEMIBOLD | AG_FONT_OBLIQUE,    117 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_SEMIBOLD | AG_FONT_ITALIC,     118 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_BOLD,                          119 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_BOLD | AG_FONT_OBLIQUE,        120 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_BOLD | AG_FONT_ITALIC,         121 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_EXTRABOLD,                     122 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_EXTRABOLD | AG_FONT_OBLIQUE,   123 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_EXTRABOLD | AG_FONT_ITALIC,    124 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_BLACK,                         125 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_BLACK | AG_FONT_OBLIQUE,       126 },
	{ AG_FONT_SEMIEXPANDED | AG_FONT_BLACK | AG_FONT_ITALIC,        127 },

	{ AG_FONT_EXPANDED,                                             128 },
	{ AG_FONT_EXPANDED | AG_FONT_OBLIQUE,                           129 },
	{ AG_FONT_EXPANDED | AG_FONT_ITALIC,                            130 },
	{ AG_FONT_EXPANDED | AG_FONT_THIN,                              131 },
	{ AG_FONT_EXPANDED | AG_FONT_THIN | AG_FONT_OBLIQUE,            132 },
	{ AG_FONT_EXPANDED | AG_FONT_THIN | AG_FONT_ITALIC,             133 },
	{ AG_FONT_EXPANDED | AG_FONT_EXTRALIGHT,                        134 },
	{ AG_FONT_EXPANDED | AG_FONT_EXTRALIGHT | AG_FONT_OBLIQUE,      135 },
	{ AG_FONT_EXPANDED | AG_FONT_EXTRALIGHT | AG_FONT_ITALIC,       136 },
	{ AG_FONT_EXPANDED | AG_FONT_LIGHT,                             137 },
	{ AG_FONT_EXPANDED | AG_FONT_LIGHT | AG_FONT_OBLIQUE,           138 },
	{ AG_FONT_EXPANDED | AG_FONT_LIGHT | AG_FONT_ITALIC,            139 },
	{ AG_FONT_EXPANDED | AG_FONT_SEMIBOLD,                          140 },
	{ AG_FONT_EXPANDED | AG_FONT_SEMIBOLD | AG_FONT_OBLIQUE,        141 },
	{ AG_FONT_EXPANDED | AG_FONT_SEMIBOLD | AG_FONT_ITALIC,         142 },
	{ AG_FONT_EXPANDED | AG_FONT_BOLD,                              143 },
	{ AG_FONT_EXPANDED | AG_FONT_BOLD | AG_FONT_OBLIQUE,            144 },
	{ AG_FONT_EXPANDED | AG_FONT_BOLD | AG_FONT_ITALIC,             145 },
	{ AG_FONT_EXPANDED | AG_FONT_EXTRABOLD,                         146 },
	{ AG_FONT_EXPANDED | AG_FONT_EXTRABOLD | AG_FONT_OBLIQUE,       147 },
	{ AG_FONT_EXPANDED | AG_FONT_EXTRABOLD | AG_FONT_ITALIC,        148 },
	{ AG_FONT_EXPANDED | AG_FONT_BLACK,                             149 },
	{ AG_FONT_EXPANDED | AG_FONT_BLACK | AG_FONT_OBLIQUE,           150 },
	{ AG_FONT_EXPANDED | AG_FONT_BLACK | AG_FONT_ITALIC,            151 },

	{ AG_FONT_ULTRAEXPANDED,                                        152 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_OBLIQUE,                      153 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_ITALIC,                       154 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_THIN,                         155 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_THIN | AG_FONT_OBLIQUE,       156 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_THIN | AG_FONT_ITALIC,        157 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_EXTRALIGHT,                   158 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_EXTRALIGHT | AG_FONT_OBLIQUE, 159 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_EXTRALIGHT | AG_FONT_ITALIC,  160 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_LIGHT,                        161 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_LIGHT | AG_FONT_OBLIQUE,      162 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_LIGHT | AG_FONT_ITALIC,       163 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_SEMIBOLD,                     164 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_SEMIBOLD | AG_FONT_OBLIQUE,   165 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_SEMIBOLD | AG_FONT_ITALIC,    166 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_BOLD,                         167 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_BOLD | AG_FONT_OBLIQUE,       168 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_BOLD | AG_FONT_ITALIC,        169 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_EXTRABOLD,                    170 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_EXTRABOLD | AG_FONT_OBLIQUE,  171 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_EXTRABOLD | AG_FONT_ITALIC,   172 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_BLACK,                        173 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_BLACK | AG_FONT_OBLIQUE,      174 },
	{ AG_FONT_ULTRAEXPANDED | AG_FONT_BLACK | AG_FONT_ITALIC,       175 },
	{ 0,                                                            -1 }
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
		    (fa->regFlags & (weight)))                                 \
			break;                                                 \
	}                                                                      \
	if (fa->face == NULL) {                                                \
		if (flags & AG_FONT_ITALIC) {                                  \
			Strlcat(path, suffix "-italic", sizeof(path));         \
		} else if (flags & AG_FONT_OBLIQUE) {                          \
			Strlcat(path, suffix "-oblique", sizeof(path));        \
		} else {                                                       \
			Strlcat(path, suffix, sizeof(path));                   \
		}                                                              \
	} else {                                                               \
		if (flags & AG_FONT_ITALIC) {                                  \
			Strlcat(path, "-italic", sizeof(path));                \
		} else if (flags & AG_FONT_OBLIQUE) {                          \
			Strlcat(path, "-oblique", sizeof(path));               \
		}                                                              \
	}

/*
 * Load the given font (or return a pointer to an existing one), from
 * a specified font face, size (in points), and option flags.
 *
 * If face is NULL or fontSize is 0.0, use the defaults from AG_Config(3).
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
	AG_Font *font;
	const AG_FontAdjustment *fa;
	const AG_FontAlias *fontAlias;
	int isInFontPath, foundByFontconfig = 0;

	if (face == NULL) {                                  /* Use default */
		Strlcpy(name, agConfig->fontFace, sizeof(name));
	} else {
		for (fontAlias = &agFontAliases[0];
		     fontAlias->from != NULL;
		     fontAlias++) {
			if (Strcasecmp(fontAlias->from, face) == 0)
				face = fontAlias->to;
		}
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

	if (fontSize < AG_FONT_PTS_EPSILON) {               /* Default size */
		fontSize = agConfig->fontSize;
	}
	for (fa = &agFontAdjustments[0]; fa->face != NULL; fa++) {
		/* TODO cache lowercase name so we can case-sensitive compare */
		if (Strcasecmp(name, fa->face) == 0) {
			fontSize *= fa->size_factor; /* Scaling correction */
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
		
		if (pExt && Strcasecmp(pExt, ".agbf") == 0) {
			/*
			 * Agar bitmap fonts are always specified by filename
			 * to prevent confusion with other types of fonts.
			 */
			spec.type = AG_FONT_BITMAP;
		} else {
			spec.type = AG_FONT_FREETYPE;
		}
		isInFontPath = 1;
	} else {                      /* Search "name.ttf", "name.otf", etc */
		char path[AG_FILENAME_MAX];
		const char **ffe;
		const AG_FontAdjustment *fa;
	
		spec.type = AG_FONT_FREETYPE;

		for (ffe = &agFontFileExts[0]; *ffe != NULL; ffe++) {
			Strlcpy(path, name, sizeof(path));

			/* Width variant suffix */
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

			/* Weight suffix */
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

			/* Style suffix */
			if (flags & AG_FONT_ITALIC) {
				Strlcat(path, "-italic", sizeof(path));
			} else if (flags & AG_FONT_OBLIQUE) {
				Strlcat(path, "-oblique", sizeof(path));
			}

			/* File extension */
			Strlcat(path, *ffe, sizeof(path));

			if (AG_ConfigFind(AG_CONFIG_PATH_FONTS, path,
			    fontPath, sizeof(fontPath)) == 0) {
				isInFontPath = 1;
				break;
			}
		}
	}

#ifdef HAVE_FONTCONFIG
	/*
	 * Fontconfig query.
	 */
	if (agFontconfigInited && !isInFontPath) {
		FcPattern *pattern, *fpat;
		FcResult fres = FcResultMatch;
		FcChar8 *filename;
		FcMatrix *mat;
		char *s;
		AG_Size len;
/*		double sizeDbl; */

		len = strlen(nameBase)+64;
		s = Malloc(len);

		if ((fontSize - floorf(fontSize)) > 0.0) {
			Snprintf(s,len, "%s-%.2f", nameBase, fontSize);
		} else {
			Snprintf(s,len, "%s-%.0f", nameBase, fontSize);
		}

		if ((flags & AG_FONT_WEIGHTS) || (flags & AG_FONT_STYLES) ||
		    (flags & AG_FONT_WD_VARIANTS)) {
			Strlcat(s, ":style=", len);

			if (flags == AG_FONT_CONDENSED) {
				Strlcat(s, "Condensed", len);
			} else if (flags == AG_FONT_SEMICONDENSED) {
				Strlcat(s, "SemiCondensed", len);
			} else if (flags == AG_FONT_ULTRACONDENSED) {
				Strlcat(s, "UltraCondensed", len);
			} else if (flags == AG_FONT_SEMIEXPANDED) {
				Strlcat(s, "SemiExpanded", len);
			} else if (flags == AG_FONT_EXPANDED) {
				Strlcat(s, "Expanded", len);
			} else if (flags == AG_FONT_ULTRAEXPANDED) {
				Strlcat(s, "UltraExpanded", len);
			} else {
				if (flags & AG_FONT_CONDENSED) {
					Strlcat(s, "Condensed ", len);
				} else if (flags & AG_FONT_SEMICONDENSED) {
					Strlcat(s, "SemiCondensed ", len);
				} else if (flags & AG_FONT_ULTRACONDENSED) {
					Strlcat(s, "UltraCondensed ", len);
				} else if (flags & AG_FONT_SEMIEXPANDED) {
					Strlcat(s, "SemiExpanded ", len);
				} else if (flags & AG_FONT_EXPANDED) {
					Strlcat(s, "Expanded ", len);
				} else if (flags & AG_FONT_ULTRAEXPANDED) {
					Strlcat(s, "UltraExpanded ", len);
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
				} else if (flags & AG_FONT_THIN) {
					if (flags & AG_FONT_ITALIC) {
						Strlcat(s, "Thin Italic,"
						           "Thin Oblique", len);
					} else if (flags & AG_FONT_OBLIQUE) {
						Strlcat(s, "Thin Oblique,"
						           "Thin Italic", len);
					} else {
						Strlcat(s, "Thin", len);
					}
				} else if (flags & AG_FONT_EXTRALIGHT) {
					if (flags & AG_FONT_ITALIC) {
						Strlcat(s, "ExtraLight Italic,"
						           "ExtraLight Oblique", len);
					} else if (flags & AG_FONT_OBLIQUE) {
						Strlcat(s, "ExtraLight Oblique,"
						           "ExtraLight Italic", len);
					} else {
						Strlcat(s, "ExtraLight", len);
					}
				} else if (flags & AG_FONT_LIGHT) {
					if (flags & AG_FONT_ITALIC) {
						Strlcat(s, "Light Italic,"
						           "Light Oblique", len);
					} else if (flags & AG_FONT_OBLIQUE) {
						Strlcat(s, "Light Oblique,"
						           "Light Italic", len);
					} else {
						Strlcat(s, "Light", len);
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
				} else if (flags & AG_FONT_EXTRABOLD) {
					if (flags & AG_FONT_ITALIC) {
						Strlcat(s, "ExtraBold Italic,"
						           "ExtraBold Oblique", len);
					} else if (flags & AG_FONT_OBLIQUE) {
						Strlcat(s, "ExtraBold Oblique,"
						           "ExtraBold Italic", len);
					} else {
						Strlcat(s, "ExtraBold", len);
					}
				} else if (flags & AG_FONT_BLACK) {
					if (flags & AG_FONT_ITALIC) {
						Strlcat(s, "Black Italic,"
						           "Black Oblique", len);
					} else if (flags & AG_FONT_OBLIQUE) {
						Strlcat(s, "Black Oblique,"
						           "Black Italic", len);
					} else {
						Strlcat(s, "Black", len);
					}
				} else if (flags & AG_FONT_ITALIC) {
					Strlcat(s, "Italic,Oblique", len);
				} else if (flags & AG_FONT_OBLIQUE) {
					Strlcat(s, "Oblique,Italic", len);
				}
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
		foundByFontconfig = 1;
		FcPatternDestroy(fpat);
		FcPatternDestroy(pattern);
	}
#else /* !HAVE_FONTCONFIG */
	(void)isInFontPath;
#endif /* HAVE_FONTCONFIG */

open_font:
	switch (spec.type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_FREETYPE:
		font = (AG_Font *)AG_FontFtNew(name, nameBase, &spec,
		                               fontPath, flags);
		if (font == NULL) {
			goto fail;
		}
		break;
#endif /* HAVE_FREETYPE */
	case AG_FONT_BITMAP:
		font = (AG_Font *)AG_FontBfNew(name, nameBase, &spec,
		                               fontPath, flags);
		if (font == NULL) {
			goto fail;
		}
		break;
	case AG_FONT_DUMMY:
		font = AG_ObjectNew(NULL, name, AGCLASS(&agFontClass));
		if (font == NULL) {
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

	if (foundByFontconfig) { font->stateFlags |= AG_FONT_FONTCONFIGED; }

	TAILQ_INSERT_HEAD(&agFontCache, font, fonts);
out:
#ifdef AG_DEBUG
	font->tAccess = AG_GetTicks();
#endif
	AG_MutexUnlock(&agTextLock);
	return (font);
fail:
	AG_MutexUnlock(&agTextLock);
	return (NULL);
}

/*
 * Update the familyStyles[] array (and nFamilyStyles) according to the set
 * of styles, weights and width variants available under the font's family.
 *
 * Return 0 on success or -1 if the information could not be queried (or
 */
int
AG_FontGetFamilyStyles(AG_Font *font)
{
#if defined(HAVE_FONTCONFIG)
	FcObjectSet *os;
	FcFontSet *fset;
	FcPattern *pat;
	int i, nFamilyStylesMax = 4;

	if (!agFontconfigInited || !(font->stateFlags & AG_FONT_FONTCONFIGED)) {
		AG_SetErrorS("No style information");
		return (-1);
	}

	/*
	 * This font was discovered via fontconfig. Query fontconfig
	 * for the available styles for this font family.
	 */
	pat = FcPatternCreate();
	os = FcObjectSetBuild(FC_FAMILY, FC_STYLE, (char *)0);
	fset = FcFontList(NULL, pat, os);
	if (fset == NULL) {
		FcObjectSetDestroy(os);
		FcPatternDestroy(pat);
		AG_SetErrorS("Fontconfig query failed");
		return (-1);
	}

	font->familyStyles = TryMalloc(nFamilyStylesMax * sizeof(Uint));
	font->nFamilyStyles = 0;

	for (i = 0; i < fset->nfont; i++) {
		char styleBuf[64];
		FcPattern *fcfont = fset->fonts[i];
		FcChar8 *pFam, *pStyle;
		char *tok, *pStyleBuf;
		Uint *familyStyles;

		if (FcPatternGetString(fcfont, FC_FAMILY, 0, &pFam) != FcResultMatch)
			continue;
		if (FcPatternGetString(fcfont, FC_STYLE, 0, &pStyle) != FcResultMatch)
			continue;

		if (Strcasecmp((char *)pFam, font->name) != 0)
			continue;

		if (font->nFamilyStyles+1 > nFamilyStylesMax) {
			Uint *flagsNew;

			nFamilyStylesMax += 4;
			flagsNew = TryRealloc(font->familyStyles,
			    nFamilyStylesMax * sizeof(Uint));
			if (flagsNew == NULL) {
				return (-1);
			}
			font->familyStyles = flagsNew;
		}
		familyStyles = &font->familyStyles[font->nFamilyStyles++];
		*familyStyles = 0;

		Strlcpy(styleBuf, (char *)pStyle, sizeof(styleBuf));
		pStyleBuf = styleBuf;
		while ((tok = Strsep(&pStyleBuf, " ,")) != NULL) {
			const AG_FontStyleName *fsn;

			if (tok[0] == '\0') {
				continue;
			}
			for (fsn = &agFontStyleNames[0]; fsn->name != NULL; fsn++) {
				 if (strcmp(fsn->name, tok) == 0)
					 break;
			}
			if (fsn->name == NULL) {
			/*	Debug(font, "Unknown flag `%s'\n", tok); */
				continue;
			}
			*familyStyles |= fsn->flag;
		}
	}

	FcFontSetDestroy(fset);
	FcObjectSetDestroy(os);
	FcPatternDestroy(pat);

	font->stateFlags |= AG_FONT_FAMILY_FLAGS;
	return (0);
#else
	AG_SetErrorS("No fontconfig support");
	return (-1);
#endif /* HAVE_FONTCONFIG */
}

/*
 * Write a string representation of the given AG_Font style flags to a
 * fixed-size buffer buf. The representation should be compatible with
 * Fontconfig FC_STYLE.
 *
 * Return the number of bytes that would have been written were bufSize
 * unlimited.
 */
AG_Size
AG_FontGetStyleName(char *buf, AG_Size bufSize, Uint flags)
{
	const AG_FontStyleName *fsn;
	AG_Size rv = 0;

	if (bufSize < 1) {
		return (0);
	}
	buf[0] = '\0';

	for (fsn = &agFontStyleNames[0]; fsn->name != NULL; fsn++) {
		if ((flags & fsn->flag) == 0) {
			continue;
		}
		if (buf[0] != '\0') {
			rv += Strlcat(buf, " ", bufSize);
		}
		rv += Strlcat(buf, fsn->name, bufSize);
	}
	if (buf[0] == '\0') {
		rv += Strlcpy(buf, "Regular", sizeof(buf));
	}
	return (rv);
}

/*
 * Return the AG_Font style flags corresponding to the given string
 * representation of a font style. The representation is case-insensitive
 * and should be compatible with Fontconfig FC_STYLE.
 *
 * Return 0 if the string could not be parsed.
 */
Uint
AG_FontGetStyleByName(const char *style)
{
	const AG_FontStyleName *fsn;

	for (fsn = &agFontStyleNames[0]; fsn->name != NULL; fsn++) {
		if (Strcasecmp(fsn->name, style) == 0)
			return (fsn->flag);
	}
	return (0);
}

static void
Init(void *_Nonnull obj)
{
	AG_Font *font = obj;

	font->name[0] = '\0';

	memset(&font->spec, 0, sizeof(AG_FontSpec) + /* spec */
	                       sizeof(Uint) +        /* flags */
	                       sizeof(Uint *) +      /* familyStyles */
	                       sizeof(Uint) +        /* nFamilyStyles */
	                       sizeof(Uint) +        /* stateFlags */
	                       sizeof(int) +         /* height */
	                       sizeof(int) +         /* ascent */
	                       sizeof(int) +         /* descent */
	                       sizeof(int) +         /* lineskip */
	                       sizeof(int) +         /* underlinePos */
	                       sizeof(int) +         /* underlineThk */
	                       sizeof(Uint));        /* tAccess */
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
	 * 
	 * Set Guide 0 of surface S to (a pixel approximation of) the
	 * typographical baseline.
	 *
	 * This routine must handle (or at the minimum skip over) any
	 * ANSI SGR sequences present in the text.
	 *
	 * Where SGR sequences require switching to an alternate font, this
	 * routine must handle the case where the alternate font uses a
	 * different font engine, and perform the necessary integration
	 * (including aligning the text to a common baseline -- normally
	 * the lowest baseline of all the fonts appearing on a given line).
	 */
}

static void
Size(const AG_Font *_Nonnull font, const AG_Char *_Nonnull ucs,
    AG_TextMetrics *_Nonnull Tm, int extended)
{
	/*
	 * Calculate the minimum size in pixels needed to render a string
	 * of text ucs using the given font.
	 *
	 * Return the Text Metrics into Tm. This should at minimum include the
	 * width x height in pixels.
	 *
	 * If the extended flag is 1, we must return the line count into nLines
	 * and initialize the wLines[] array according to the width in pixels
	 * of each individual line of rendered text.
	 */
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

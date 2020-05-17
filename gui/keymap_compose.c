/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Input composition. This is useful for those platforms lacking proper
 * support for international keyboard input, or in special cases where
 * Unicode keyboard translation is disabled for performance reasons.
 */

#include <agar/config/ag_unicode.h>
#ifdef AG_UNICODE

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/editable.h>
#include <agar/gui/keymap.h>

int
AG_KeyInputCompose(AG_Editable *ed, AG_Char key, AG_Char *ins)
{
	int i;

	if (ed->compose != 0) {
		for (i = 0; i < agCompositionMapSize; i++) {
			if (agCompositionMap[i].comp == ed->compose &&
			    agCompositionMap[i].key == key)
				break;
		}
		if (i < agCompositionMapSize) {		/* Insert composition */
			ins[0] = agCompositionMap[i].res;
			ed->compose = 0;
			return (1);
		} else {				/* Insert as-is */
			ed->compose = 0;
			ins[0] = ed->compose;
			ins[1] = key;
			return (2);
		}
	} else {
		for (i = 0; i < agCompositionMapSize; i++) {
			if (agCompositionMap[i].comp == key)
				break;
		}
		if (i < agCompositionMapSize) {		/* Wait until next */
			ed->compose = key;
			return (0);
		} else {
			ins[0] = key;
			return (1);
		}
	}
	return (0);
}

const struct ag_key_composition agCompositionMap[] = {
	{ 0x0060, 0x0020, 0x0060 },  /* GRAVE ACCENT */
	{ 0x0060, 0x0061, 0x00e0 },  /* LATIN SMALL LETTER A */
	{ 0x0060, 0x0041, 0x00c0 },  /* LATIN CAPITAL LETTER A */
	{ 0x0060, 0x0065, 0x00e8 },  /* LATIN SMALL LETTER E */
	{ 0x0060, 0x0045, 0x00c8 },  /* LATIN CAPITAL LETTER E */
	{ 0x0060, 0x0069, 0x00ec },  /* LATIN SMALL LETTER I */
	{ 0x0060, 0x0049, 0x00cc },  /* LATIN CAPITAL LETTER I */
	{ 0x0060, 0x006f, 0x00f2 },  /* LATIN SMALL LETTER O */
	{ 0x0060, 0x004f, 0x00d2 },  /* LATIN CAPITAL LETTER O */
	{ 0x0060, 0x0075, 0x00f9 },  /* LATIN SMALL LETTER U */
	{ 0x0060, 0x0055, 0x00d9 },  /* LATIN CAPITAL LETTER U */
	
	{ 0x00b4, 0x0020, 0x0060 },  /* ACUTE ACCENT */
	{ 0x00b4, 0x0065, 0x00e9 },  /* LATIN SMALL LETTER E */
	{ 0x00b4, 0x0045, 0x00c9 },  /* LATIN CAPITAL LETTER E */
	
	{ 0x02db, 0x0020, 0x02db },  /* OGONEK */
	{ 0x02db, 0x0061, 0x0105 },  /* LATIN SMALL LETTER C */
	{ 0x02db, 0x0041, 0x0104 },  /* LATIN CAPITAL LETTER C */
	{ 0x02db, 0x0075, 0x0173 },  /* LATIN SMALL LETTER U */
	{ 0x02db, 0x0055, 0x0172 },  /* LATIN CAPITAL LETTER U */

	{ 0x00b8, 0x0020, 0x00b8 },  /* CEDILLA */
	{ 0x00b8, 0x0063, 0x00e7 },  /* LATIN SMALL LETTER C */
	{ 0x00b8, 0x0043, 0x00c7 },  /* LATIN CAPITAL LETTER C */
	{ 0x00b8, 0x0067, 0x0123 },  /* LATIN SMALL LETTER G */
	{ 0x00b8, 0x0047, 0x0122 },  /* LATIN CAPITAL LETTER G */
	{ 0x00b8, 0x006e, 0x0146 },  /* LATIN SMALL LETTER N */
	{ 0x00b8, 0x004e, 0x0145 },  /* LATIN CAPITAL LETTER N */
	{ 0x00b8, 0x006b, 0x0137 },  /* LATIN SMALL LETTER K */
	{ 0x00b8, 0x004b, 0x0136 },  /* LATIN CAPITAL LETTER K */
	{ 0x00b8, 0x0072, 0x0157 },  /* LATIN SMALL LETTER R */
	{ 0x00b8, 0x0052, 0x0156 },  /* LATIN CAPITAL LETTER R */
	{ 0x00b8, 0x0074, 0x0163 },  /* LATIN SMALL LETTER T */
	{ 0x00b8, 0x0054, 0x0162 },  /* LATIN CAPITAL LETTER T */
	{ 0x00b8, 0x0073, 0x015f },  /* LATIN SMALL LETTER S */
	{ 0x00b8, 0x0053, 0x015e },  /* LATIN CAPITAL LETTER S */
	
	{ 0x00a8, 0x0020, 0x00a8 },  /* DIAERESIS */
	{ 0x00a8, 0x0061, 0x00e4 },  /* LATIN SMALL LETTER A */
	{ 0x00a8, 0x0041, 0x00c4 },  /* LATIN CAPITAL LETTER A */
	{ 0x00a8, 0x0065, 0x00eb },  /* LATIN SMALL LETTER E */
	{ 0x00a8, 0x0045, 0x00cb },  /* LATIN CAPITAL LETTER E */
	{ 0x00a8, 0x0069, 0x00ef },  /* LATIN SMALL LETTER I */
	{ 0x00a8, 0x0049, 0x00cf },  /* LATIN CAPITAL LETTER I */
	{ 0x00a8, 0x006f, 0x00f6 },  /* LATIN SMALL LETTER O */
	{ 0x00a8, 0x004f, 0x00d6 },  /* LATIN CAPITAL LETTER O */
	{ 0x00a8, 0x0079, 0x00ff },  /* LATIN SMALL LETTER Y */
	{ 0x00a8, 0x0059, 0x0178 },  /* LATIN CAPITAL LETTER Y */
	{ 0x00a8, 0x0075, 0x00fc },  /* LATIN SMALL LETTER U */
	{ 0x00a8, 0x0055, 0x00dc },  /* LATIN CAPITAL LETTER U */
	
	{ 0x005e, 0x0020, 0x005e },  /* CIRCUMFLEX ACCENT */
	{ 0x005e, 0x0061, 0x00e2 },  /* LATIN SMALL LETTER A */
	{ 0x005e, 0x0041, 0x00c2 },  /* LATIN CAPITAL LETTER A */
	{ 0x005e, 0x0063, 0x0109 },  /* LATIN SMALL LETTER C */
	{ 0x005e, 0x0043, 0x0108 },  /* LATIN CAPITAL LETTER C */
	{ 0x005e, 0x0065, 0x00ea },  /* LATIN SMALL LETTER E */
	{ 0x005e, 0x0045, 0x00ca },  /* LATIN CAPITAL LETTER E */
	{ 0x005e, 0x0067, 0x011d },  /* LATIN SMALL LETTER G */
	{ 0x005e, 0x0047, 0x011c },  /* LATIN CAPITAL LETTER G */
	{ 0x005e, 0x0069, 0x00ee },  /* LATIN SMALL LETTER I */
	{ 0x005e, 0x0049, 0x00ce },  /* LATIN CAPITAL LETTER I */
	{ 0x005e, 0x006f, 0x00f4 },  /* LATIN SMALL LETTER O */
	{ 0x005e, 0x004f, 0x00d4 },  /* LATIN CAPITAL LETTER O */
	{ 0x005e, 0x0073, 0x015d },  /* LATIN SMALL LETTER S */
	{ 0x005e, 0x0053, 0x015c },  /* LATIN CAPITAL LETTER S */
	{ 0x005e, 0x0079, 0x0177 },  /* LATIN SMALL LETTER Y */
	{ 0x005e, 0x0059, 0x0176 },  /* LATIN CAPITAL LETTER Y */
	{ 0x005e, 0x0075, 0x00fb },  /* LATIN SMALL LETTER U */
	{ 0x005e, 0x0055, 0x00db },  /* LATIN CAPITAL LETTER U */
	{ 0x005e, 0x0077, 0x0175 },  /* LATIN SMALL LETTER W */
	{ 0x005e, 0x0057, 0x0174 },  /* LATIN CAPITAL LETTER W */
};
const int agCompositionMapSize = sizeof(agCompositionMap) /
                                 sizeof(agCompositionMap[0]);

#endif /* AG_UNICODE */

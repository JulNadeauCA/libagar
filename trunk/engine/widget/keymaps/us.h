/*	$Csoft: us.h,v 1.5 2003/01/04 13:57:00 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
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

const struct keycode keycodes[] = {
	/* Non-alphanumeric characters */
	{ "i-exclaim",	SDLK_1,		KMOD_SHIFT,	insert_ascii,	"!" },
	{ "i-amper",	SDLK_2,		KMOD_SHIFT,	insert_ascii,	"@" },
	{ "i-hash",	SDLK_3,		KMOD_SHIFT,	insert_ascii,	"#" },
	{ "i-dollar",	SDLK_4,		KMOD_SHIFT,	insert_ascii,	"$" },
	{ "i-percent",	SDLK_5,		KMOD_SHIFT,	insert_ascii,	"%" },
	{ "i-caret",	SDLK_6,		KMOD_SHIFT,	insert_ascii,	"^" },
	{ "i-amper",	SDLK_7,		KMOD_SHIFT,	insert_ascii,	"&" },
	{ "i-asterisk",	SDLK_8,		KMOD_SHIFT,	insert_ascii,	"*" },
	{ "i-lparent",	SDLK_9,		KMOD_SHIFT,	insert_ascii,	"(" },
	{ "i-rparen",	SDLK_0,		KMOD_SHIFT,	insert_ascii,	")" },
	{ "i-tilde",	SDLK_BACKQUOTE,	KMOD_SHIFT,	insert_ascii,	"~" },
	{ "i-bckquote",	SDLK_BACKQUOTE,	0,		insert_ascii,	"`" },
	{ "i-uscore",	SDLK_MINUS,	KMOD_SHIFT,	insert_ascii,	"_" },
	{ "i-minus",	SDLK_MINUS,	0,		insert_ascii,	"-" },
	{ "i-plus",	SDLK_EQUALS,	KMOD_SHIFT,	insert_ascii,	"+" },
	{ "i-equal",	SDLK_EQUALS,	0,		insert_ascii,	"=" },
	{ "i-lbraces",	SDLK_LEFTBRACKET, KMOD_SHIFT,	insert_ascii,	"{" },
	{ "i-lbrack",	SDLK_LEFTBRACKET, 0,		insert_ascii,	"[" },
	{ "i-rbraces",	SDLK_RIGHTBRACKET, KMOD_SHIFT,	insert_ascii,	"}" },
	{ "i-rbrack",	SDLK_RIGHTBRACKET, 0,		insert_ascii,	"]" },
	{ "i-pipe",	SDLK_BACKSLASH,	KMOD_SHIFT,	insert_ascii,	"|" },
	{ "i-bslash",	SDLK_BACKSLASH,	0,		insert_ascii,	"\\" },
	{ "i-colon",	SDLK_SEMICOLON,	KMOD_SHIFT,	insert_ascii,	":" },
	{ "i-scolon",	SDLK_SEMICOLON,	0,		insert_ascii,	";" },
	{ "i-dquote",	SDLK_QUOTE,	KMOD_SHIFT,	insert_ascii,	"\"" },
	{ "i-squote",	SDLK_QUOTE,	0,		insert_ascii,	"'" },
	{ "i-question",	SDLK_SLASH,	KMOD_SHIFT,	insert_ascii,	"?" },
	{ "i-slash",	SDLK_SLASH,	0,		insert_ascii,	"/" },
	{ "i-less",	SDLK_COMMA,	KMOD_SHIFT,	insert_ascii,	"<" },
	{ "i-comma",	SDLK_COMMA,	0,		insert_ascii,	"," },
	{ "i-greater",	SDLK_PERIOD,	KMOD_SHIFT,	insert_ascii,	">" },
	{ "i-period",	SDLK_PERIOD,	0,		insert_ascii,	"." },
	{ "i-space",	SDLK_SPACE,	0,		insert_ascii,	" " },
	
	/* Control characters */
	{ "c-bspace",	SDLK_BACKSPACE, 0,		key_bspace,	NULL },
	{ "c-delete",	SDLK_DELETE,	0,		key_delete,	NULL },
	{ "c-home",	SDLK_HOME,	0,		key_home,	NULL },
	{ "c-end",	SDLK_END,	0,		key_end,	NULL },
	{ "c-home",	SDLK_a,		KMOD_CTRL,	key_home,	NULL },
	{ "c-end",	SDLK_e,		KMOD_CTRL,	key_end,	NULL },
	{ "c-kill",	SDLK_k,		KMOD_CTRL,	key_kill,	NULL },
	{ "c-left",	SDLK_LEFT,	0,		key_left,	NULL },
	{ "c-right",	SDLK_RIGHT,	0,		key_right,	NULL },

	/* Alphabetic characters */
	{ "a",		SDLK_a,		0,		insert_alpha,	"a" },
	{ "b",		SDLK_b,		0,		insert_alpha,	"b" },
	{ "c",		SDLK_c,		0,		insert_alpha,	"c" },
	{ "d",		SDLK_d,		0,		insert_alpha,	"d" },
	{ "e",		SDLK_e,		0,		insert_alpha,	"e" },
	{ "f",		SDLK_f,		0,		insert_alpha,	"f" },
	{ "g",		SDLK_g,		0,		insert_alpha,	"g" },
	{ "h",		SDLK_h,		0,		insert_alpha,	"h" },
	{ "i",		SDLK_i,		0,		insert_alpha,	"i" },
	{ "j",		SDLK_j,		0,		insert_alpha,	"j" },
	{ "k",		SDLK_k,		0,		insert_alpha,	"k" },
	{ "l",		SDLK_l,		0,		insert_alpha,	"l" },
	{ "m",		SDLK_m,		0,		insert_alpha,	"m" },
	{ "n",		SDLK_n,		0,		insert_alpha,	"n" },
	{ "o",		SDLK_o,		0,		insert_alpha,	"o" },
	{ "p",		SDLK_p,		0,		insert_alpha,	"p" },
	{ "q",		SDLK_q,		0,		insert_alpha,	"q" },
	{ "r",		SDLK_r,		0,		insert_alpha,	"r" },
	{ "s",		SDLK_s,		0,		insert_alpha,	"s" },
	{ "t",		SDLK_t,		0,		insert_alpha,	"t" },
	{ "u",		SDLK_u,		0,		insert_alpha,	"u" },
	{ "v",		SDLK_v,		0,		insert_alpha,	"v" },
	{ "w",		SDLK_w,		0,		insert_alpha,	"w" },
	{ "x",		SDLK_x,		0,		insert_alpha,	"x" },
	{ "y",		SDLK_y,		0,		insert_alpha,	"y" },
	{ "z",		SDLK_z,		0,		insert_alpha,	"z" },

	/* Digits */
	{ "one",	SDLK_1,		0,		insert_ascii,	"1" },
	{ "two",	SDLK_2,		0,		insert_ascii,	"2" },
	{ "three",	SDLK_3,		0,		insert_ascii,	"3" },
	{ "four",	SDLK_4,		0,		insert_ascii,	"4" },
	{ "five",	SDLK_5,		0,		insert_ascii,	"5" },
	{ "six",	SDLK_6,		0,		insert_ascii,	"6" },
	{ "seven",	SDLK_7,		0,		insert_ascii,	"7" },
	{ "eight",	SDLK_8,		0,		insert_ascii,	"8" },
	{ "nine",	SDLK_9,		0,		insert_ascii,	"9" },
	{ "zero",	SDLK_0,		0,		insert_ascii,	"0" },

	{ NULL,		SDLK_LAST,	0,		NULL, NULL }
};

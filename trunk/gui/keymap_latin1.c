/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>

#include "widget.h"
#include "editable.h"
#include "keymap.h"
#include "unicode.h"

/*
 * Traditional ISO-8859-1 keyboard input mapping to Unicode.
 */
const struct ag_key_mapping agKeymapLATIN1[] = {
	{ SDLK_q,		KMOD_ALT|KMOD_SHIFT,	0x00d1	},
	{ SDLK_w,		KMOD_ALT|KMOD_SHIFT,	0x00d7	},
	{ SDLK_e,		KMOD_ALT|KMOD_SHIFT,	0x00c5	},
	{ SDLK_r,		KMOD_ALT|KMOD_SHIFT,	0x00d2	},
	{ SDLK_t,		KMOD_ALT|KMOD_SHIFT,	0x00d4	},
	{ SDLK_y,		KMOD_ALT|KMOD_SHIFT,	0x00d9	},
	{ SDLK_u,		KMOD_ALT|KMOD_SHIFT,	0x00d5	},
	{ SDLK_i,		KMOD_ALT|KMOD_SHIFT,	0x00c9	},
	{ SDLK_o,		KMOD_ALT|KMOD_SHIFT,	0x00cf	},
	{ SDLK_p,		KMOD_ALT|KMOD_SHIFT,	0x00d0	},
	{ SDLK_LEFTBRACKET,	KMOD_ALT|KMOD_SHIFT,	0x00fb	},
	{ SDLK_RIGHTBRACKET,	KMOD_ALT|KMOD_SHIFT,	0x00fd	},
	{ SDLK_a,		KMOD_ALT|KMOD_SHIFT,	0x00c1	},
	{ SDLK_s,		KMOD_ALT|KMOD_SHIFT,	0x00d3	},
	{ SDLK_d,		KMOD_ALT|KMOD_SHIFT,	0x00c4	},
	{ SDLK_f,		KMOD_ALT|KMOD_SHIFT,	0x00c6	},
	{ SDLK_g,		KMOD_ALT|KMOD_SHIFT,	0x00c7	},
	{ SDLK_h,		KMOD_ALT|KMOD_SHIFT,	0x00c8	},
	{ SDLK_j,		KMOD_ALT|KMOD_SHIFT,	0x00ca	},
	{ SDLK_k,		KMOD_ALT|KMOD_SHIFT,	0x00cb	},
	{ SDLK_l,		KMOD_ALT|KMOD_SHIFT,	0x00cc	},
	{ SDLK_SEMICOLON,	KMOD_ALT|KMOD_SHIFT,	0x00ba	},
	{ SDLK_QUOTE,		KMOD_ALT|KMOD_SHIFT,	0x00a2	},
	{ SDLK_BACKSLASH,	KMOD_ALT|KMOD_SHIFT,	0x00fc	},
	{ SDLK_z,		KMOD_ALT|KMOD_SHIFT,	0x00da	},
	{ SDLK_x,		KMOD_ALT|KMOD_SHIFT,	0x00d8	},
	{ SDLK_c,		KMOD_ALT|KMOD_SHIFT,	0x00c3	},
	{ SDLK_v,		KMOD_ALT|KMOD_SHIFT,	0x00d6	},
	{ SDLK_b,		KMOD_ALT|KMOD_SHIFT,	0x00c2	},
	{ SDLK_n,		KMOD_ALT|KMOD_SHIFT,	0x00ce	},
	{ SDLK_m,		KMOD_ALT|KMOD_SHIFT,	0x00cd	},
	{ SDLK_COMMA,		KMOD_ALT|KMOD_SHIFT,	0x00bc	},
	{ SDLK_PERIOD,		KMOD_ALT|KMOD_SHIFT,	0x00be	},
	{ SDLK_SLASH,		KMOD_ALT|KMOD_SHIFT,	0x00bf	},
	{ SDLK_BACKQUOTE,	KMOD_ALT|KMOD_SHIFT,	0x00fe	},
	{ SDLK_1,		KMOD_ALT|KMOD_SHIFT,	0x00a1	},
	{ SDLK_2,		KMOD_ALT|KMOD_SHIFT,	0x00c0	},
	{ SDLK_3,		KMOD_ALT|KMOD_SHIFT,	0x00a3	},
	{ SDLK_4,		KMOD_ALT|KMOD_SHIFT,	0x00a4	},
	{ SDLK_5,		KMOD_ALT|KMOD_SHIFT,	0x00a5	},
	{ SDLK_6,		KMOD_ALT|KMOD_SHIFT,	0x00de	},
	{ SDLK_7,		KMOD_ALT|KMOD_SHIFT,	0x00a6	},
	{ SDLK_8,		KMOD_ALT|KMOD_SHIFT,	0x00aa	},
	{ SDLK_9,		KMOD_ALT|KMOD_SHIFT,	0x00a8	},
	{ SDLK_0,		KMOD_ALT|KMOD_SHIFT,	0x00a9	},
	{ SDLK_MINUS,		KMOD_ALT|KMOD_SHIFT,	0x00df	},
	{ SDLK_EQUALS,		KMOD_ALT|KMOD_SHIFT,	0x00ab	},

	{ SDLK_q,		KMOD_ALT,		0x00f1	},
	{ SDLK_w,		KMOD_ALT,		0x00f7	},
	{ SDLK_e,		KMOD_ALT,		0x00e5	},
	{ SDLK_r,		KMOD_ALT,		0x00f2	},
	{ SDLK_t,		KMOD_ALT,		0x00f4	},
	{ SDLK_y,		KMOD_ALT,		0x00f9	},
	{ SDLK_u,		KMOD_ALT,		0x00f5	},
	{ SDLK_i,		KMOD_ALT,		0x00e9	},
	{ SDLK_o,		KMOD_ALT,		0x00ef	},
	{ SDLK_p,		KMOD_ALT,		0x00f0	},
	{ SDLK_LEFTBRACKET,	KMOD_ALT,		0x00db	},
	{ SDLK_RIGHTBRACKET,	KMOD_ALT,		0x00fd	},
	{ SDLK_a,		KMOD_ALT,		0x00e1	},
	{ SDLK_s,		KMOD_ALT,		0x00f3	},
	{ SDLK_d,		KMOD_ALT,		0x00e4	},
	{ SDLK_f,		KMOD_ALT,		0x00e6	},
	{ SDLK_g,		KMOD_ALT,		0x00e7	},
	{ SDLK_h,		KMOD_ALT,		0x00e8	},
	{ SDLK_j,		KMOD_ALT,		0x00ea	},
	{ SDLK_k,		KMOD_ALT,		0x00eb	},
	{ SDLK_l,		KMOD_ALT,		0x00ec	},
	{ SDLK_SEMICOLON,	KMOD_ALT,		0x00bb	},
	{ SDLK_QUOTE,		KMOD_ALT,		0x00a7	},
	{ SDLK_BACKSLASH,	KMOD_ALT,		0x00dc	},
	{ SDLK_z,		KMOD_ALT,		0x00fa	},
	{ SDLK_x,		KMOD_ALT,		0x00f8	},
	{ SDLK_c,		KMOD_ALT,		0x00e3	},
	{ SDLK_v,		KMOD_ALT,		0x00f6	},
	{ SDLK_b,		KMOD_ALT,		0x00e2	},
	{ SDLK_n,		KMOD_ALT,		0x00ee	},
	{ SDLK_m,		KMOD_ALT,		0x00ed	},
	{ SDLK_COMMA,		KMOD_ALT,		0x00ac	},
	{ SDLK_PERIOD,		KMOD_ALT,		0x00ae	},
	{ SDLK_SLASH,		KMOD_ALT,		0x00af	},
	{ SDLK_BACKQUOTE,	KMOD_ALT,		0x00e0	},
	{ SDLK_1,		KMOD_ALT,		0x00b1	},
	{ SDLK_2,		KMOD_ALT,		0x00b2	},
	{ SDLK_3,		KMOD_ALT,		0x00b3	},
	{ SDLK_4,		KMOD_ALT,		0x00b4	},
	{ SDLK_5,		KMOD_ALT,		0x00b5	},
	{ SDLK_6,		KMOD_ALT,		0x00b6	},
	{ SDLK_7,		KMOD_ALT,		0x00b7	},
	{ SDLK_8,		KMOD_ALT,		0x00b8	},
	{ SDLK_9,		KMOD_ALT,		0x00b9	},
	{ SDLK_0,		KMOD_ALT,		0x00b0	},
	{ SDLK_MINUS,		KMOD_ALT,		0x00bd	},
	{ SDLK_EQUALS,		KMOD_ALT,		0x00ad	},
	
	{ SDLK_LAST,		0,			0x0	},
};

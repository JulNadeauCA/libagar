/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Standard color palette for GUI elements.
 */

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/gui/gui.h>
#include <agar/gui/colors.h>
#include <agar/gui/drv.h>
#include <agar/gui/stylesheet.h>
#include <agar/gui/gui_math.h>

#include <ctype.h>

/*
 * Color name keywords used by the style engine.
 */
AG_ColorName agColorNames[] = {
#if AG_MODEL == AG_LARGE
	/*
	 * Base (48-bit)
	 */
	{ "red",     { 0xffff, 0x0000, 0x0000, 0xffff } },
	{ "green",   { 0x0000, 0x8080, 0x0000, 0xffff } },
	{ "blue",    { 0x0000, 0x0000, 0xffff, 0xffff } },
	{ "yellow",  { 0xffff, 0xffff, 0x0000, 0xffff } },
	{ "cyan",    { 0x0000, 0xffff, 0xffff, 0xffff } },
	{ "magenta", { 0xffff, 0x0000, 0xffff, 0xffff } },
	{ "black",   { 0x0000, 0x0000, 0x0000, 0xffff } },
	{ "grey",    { 0x8080, 0x8080, 0x8080, 0xffff } },
	{ "gray",    { 0x8080, 0x8080, 0x8080, 0xffff } },
	{ "white",   { 0xffff, 0xffff, 0xffff, 0xffff } },
	/*
	 * CSS (48-bit)
	 */
	{ "aliceblue",            { 0xf0f0, 0xf8f8, 0xffff, 0xffff } },
	{ "antiquewhite",         { 0xfafa, 0xebeb, 0xd7d7, 0xffff } },
	{ "aqua",                 { 0x0000, 0xffff, 0xffff, 0xffff } },
	{ "aquamarine",           { 0x7f7f, 0xffff, 0xd4d4, 0xffff } },
	{ "azure",                { 0xf0f0, 0xffff, 0xffff, 0xffff } },
	{ "beige",                { 0xf5f5, 0xf5f5, 0xdcdc, 0xffff } },
	{ "bisque",               { 0xffff, 0xe4e4, 0xc4c4, 0xffff } },
	{ "blanchedalmond",       { 0xffff, 0xebeb, 0xcdcd, 0xffff } },
	{ "blueviolet",           { 0x8a8a, 0x2b2b, 0xe2e2, 0xffff } },
	{ "brown",                { 0xa5a5, 0x2a2a, 0x2a2a, 0xffff } },
	{ "burlywood",            { 0xdede, 0xb8b8, 0x8787, 0xffff } },
	{ "cadetblue",            { 0x5f5f, 0x9e9e, 0xa0a0, 0xffff } },
	{ "chartreuse",           { 0x7f7f, 0xffff, 0x0000, 0xffff } },
	{ "chocolate",            { 0xd2d2, 0x6969, 0x1e1e, 0xffff } },
	{ "coral",                { 0xffff, 0x7f7f, 0x5050, 0xffff } },
	{ "cornflowerblue",       { 0x6464, 0x9595, 0xeded, 0xffff } },
	{ "cornsilk",             { 0xffff, 0xf8f8, 0xdcdc, 0xffff } },
	{ "crimson",              { 0xdcdc, 0x1414, 0x3c3c, 0xffff } },
	{ "darkblue",             { 0x0000, 0x0000, 0x8b8b, 0xffff } },
	{ "darkcyan",             { 0x0000, 0x8b8b, 0x8b8b, 0xffff } },
	{ "darkgoldenrod",        { 0xb8b8, 0x8686, 0x0b0b, 0xffff } },
	{ "darkgray",             { 0xa9a9, 0xa9a9, 0xa9a9, 0xffff } },
	{ "darkgreen",            { 0x0000, 0x6464, 0x0000, 0xffff } },
	{ "darkgrey",             { 0xa9a9, 0xa9a9, 0xa9a9, 0xffff } },
	{ "darkkhaki",            { 0xbdbd, 0xb7b7, 0x6b6b, 0xffff } },
	{ "darkmagenta",          { 0x8b8b, 0x0000, 0x8b8b, 0xffff } },
	{ "darkolivegreen",       { 0x5555, 0x6b6b, 0x2f2f, 0xffff } },
	{ "darkorange",           { 0xffff, 0x8c8c, 0x0000, 0xffff } },
	{ "darkorchid",           { 0x9999, 0x3232, 0xcccc, 0xffff } },
	{ "darkred",              { 0x8b8b, 0x0000, 0x0000, 0xffff } },
	{ "darksalmon",           { 0xe9e9, 0x9696, 0x7a7a, 0xffff } },
	{ "darkseagreen",         { 0x8f8f, 0xbcbc, 0x8f8f, 0xffff } },
	{ "darkslateblue",        { 0x4848, 0x3d3d, 0x8b8b, 0xffff } },
	{ "darkslategray",        { 0x2f2f, 0x4f4f, 0x4f4f, 0xffff } },
	{ "darkslategrey",        { 0x2f2f, 0x4f4f, 0x4f4f, 0xffff } },
	{ "darkturquoise",        { 0x0000, 0xcece, 0xd1d1, 0xffff } },
	{ "darkviolet",           { 0x9494, 0x0000, 0xd3d3, 0xffff } },
	{ "deeppink",             { 0xffff, 0x1414, 0x9393, 0xffff } },
	{ "deepskyblue",          { 0x0000, 0xbfbf, 0xffff, 0xffff } },
	{ "dimgray",              { 0x6969, 0x6969, 0x6969, 0xffff } },
	{ "dimgrey",              { 0x6969, 0x6969, 0x6969, 0xffff } },
	{ "dodgerblue",           { 0x1e1e, 0x9090, 0xffff, 0xffff } },
	{ "firebrick",            { 0xb2b2, 0x2222, 0x2222, 0xffff } },
	{ "floralwhite",          { 0xffff, 0xfafa, 0xf0f0, 0xffff } },
	{ "forestgreen",          { 0x2222, 0x8b8b, 0x2222, 0xffff } },
	{ "fuchsia",              { 0xffff, 0x0000, 0xffff, 0xffff } },
	{ "gainsboro",            { 0xdcdc, 0xdcdc, 0xdcdc, 0xffff } },
	{ "ghostwhite",           { 0xf8f8, 0xf8f8, 0xffff, 0xffff } },
	{ "gold",                 { 0xffff, 0xd7d7, 0x0000, 0xffff } },
	{ "goldenrod",            { 0xdada, 0xa5a5, 0x2020, 0xffff } },
	{ "greenyellow",          { 0xadad, 0xffff, 0x2f2f, 0xffff } },
	{ "honeydew",             { 0xf0f0, 0xffff, 0xf0f0, 0xffff } },
	{ "hotpink",              { 0xffff, 0x6969, 0xb4b4, 0xffff } },
	{ "indianred",            { 0xcdcd, 0x5c5c, 0x5c5c, 0xffff } },
	{ "indigo",               { 0x4b4b, 0x0000, 0x8282, 0xffff } },
	{ "ivory",                { 0xffff, 0xffff, 0xf0f0, 0xffff } },
	{ "khaki",                { 0xf0f0, 0xe6e6, 0x8c8c, 0xffff } },
	{ "lavender",             { 0xe6e6, 0xe6e6, 0xfafa, 0xffff } },
	{ "lavenderblush",        { 0xffff, 0xf0f0, 0xf5f5, 0xffff } },
	{ "lawngreen",            { 0x7c7c, 0xfcfc, 0x0000, 0xffff } },
	{ "lemonchiffon",         { 0xffff, 0xfafa, 0xcdcd, 0xffff } },
	{ "lightblue",            { 0xadad, 0xd8d8, 0xe6e6, 0xffff } },
	{ "lightcoral",           { 0xf0f0, 0x8080, 0x8080, 0xffff } },
	{ "lightcyan",            { 0xe0e0, 0xffff, 0xffff, 0xffff } },
	{ "lightgoldenrodyellow", { 0xfafa, 0xfafa, 0xd2d2, 0xffff } },
	{ "lightgray",            { 0xd3d3, 0xd3d3, 0xd3d3, 0xffff } },
	{ "lightgreen",           { 0x9090, 0xeeee, 0x9090, 0xffff } },
	{ "lightgrey",            { 0xd3d3, 0xd3d3, 0xd3d3, 0xffff } },
	{ "lightpink",            { 0xffff, 0xb6b6, 0xc1c1, 0xffff } },
	{ "lightsalmon",          { 0xffff, 0xa0a0, 0x7a7a, 0xffff } },
	{ "lightseagreen",        { 0x2020, 0xb2b2, 0xaaaa, 0xffff } },
	{ "lightskyblue",         { 0x8787, 0xcece, 0xfafa, 0xffff } },
	{ "lightslategray",       { 0x7777, 0x8888, 0x9999, 0xffff } },
	{ "lightslategrey",       { 0x7777, 0x8888, 0x9999, 0xffff } },
	{ "lightsteelblue",       { 0xb0b0, 0xc4c4, 0xdede, 0xffff } },
	{ "lightyellow",          { 0xffff, 0xffff, 0xe0e0, 0xffff } },
	{ "lime",                 { 0x0000, 0xffff, 0x0000, 0xffff } },
	{ "limegreen",            { 0x3232, 0xcdcd, 0x3232, 0xffff } },
	{ "linen",                { 0xfafa, 0xf0f0, 0xe6e6, 0xffff } },
	{ "maroon",               { 0x8080, 0x0000, 0x0000, 0xffff } },
	{ "mediumaquamarine",     { 0x6666, 0xcdcd, 0xaaaa, 0xffff } },
	{ "mediumblue",           { 0x0000, 0x0000, 0xcdcd, 0xffff } },
	{ "mediumorchid",         { 0xbaba, 0x5555, 0xd3d3, 0xffff } },
	{ "mediumpurple",         { 0x9393, 0x7070, 0xdbdb, 0xffff } },
	{ "mediumseagreen",       { 0x3c3c, 0xb3b3, 0x7171, 0xffff } },
	{ "mediumslateblue",      { 0x7b7b, 0x6868, 0xeeee, 0xffff } },
	{ "mediumspringgreen",    { 0x0000, 0xfafa, 0x9a9a, 0xffff } },
	{ "mediumturquoise",      { 0x4848, 0xd1d1, 0xcccc, 0xffff } },
	{ "mediumvioletred",      { 0xc7c7, 0x1515, 0x8585, 0xffff } },
	{ "midnightblue",         { 0x1919, 0x1919, 0x7070, 0xffff } },
	{ "mintcream",            { 0xf5f5, 0xffff, 0xfafa, 0xffff } },
	{ "mistyrose",            { 0xffff, 0xe4e4, 0xe1e1, 0xffff } },
	{ "moccasin",             { 0xffff, 0xe4e4, 0xb5b5, 0xffff } },
	{ "navajowhite",          { 0xffff, 0xdede, 0xadad, 0xffff } },
	{ "navy",                 { 0x0000, 0x0000, 0x8080, 0xffff } },
	{ "oldlace",              { 0xfdfd, 0xf5f5, 0xe6e6, 0xffff } },
	{ "olive",                { 0x8080, 0x8080, 0x0000, 0xffff } },
	{ "olivedrab",            { 0x6b6b, 0x8e8e, 0x2323, 0xffff } },
	{ "orange",               { 0xffff, 0xa5a5, 0x0000, 0xffff } },
	{ "orangered",            { 0xffff, 0x4545, 0x0000, 0xffff } },
	{ "orchid",               { 0xdada, 0x7070, 0xd6d6, 0xffff } },
	{ "palegoldenrod",        { 0xeeee, 0xe8e8, 0xaaaa, 0xffff } },
	{ "palegreen",            { 0x9898, 0xfbfb, 0x9898, 0xffff } },
	{ "paleturquoise",        { 0xafaf, 0xeeee, 0xeeee, 0xffff } },
	{ "palevioletred",        { 0xdbdb, 0x7070, 0x9393, 0xffff } },
	{ "papayawhip",           { 0xffff, 0xefef, 0xd5d5, 0xffff } },
	{ "peachpuff",            { 0xffff, 0xdada, 0xb9b9, 0xffff } },
	{ "peru",                 { 0xcdcd, 0x8585, 0x3f3f, 0xffff } },
	{ "pink",                 { 0xffff, 0xc0c0, 0xcbcb, 0xffff } },
	{ "plum",                 { 0xdddd, 0xa0a0, 0xdddd, 0xffff } },
	{ "powderblue",           { 0xb0b0, 0xe0e0, 0xe6e6, 0xffff } },
	{ "purple",               { 0x8080, 0x0000, 0x8080, 0xffff } },
	{ "rosybrown",            { 0xbcbc, 0x8f8f, 0x8f8f, 0xffff } },
	{ "royalblue",            { 0x4141, 0x6969, 0xe1e1, 0xffff } },
	{ "saddlebrown",          { 0x8b8b, 0x4545, 0x1313, 0xffff } },
	{ "salmon",               { 0xfafa, 0x8080, 0x7272, 0xffff } },
	{ "sandybrown",           { 0xf4f4, 0xa4a4, 0x6060, 0xffff } },
	{ "seagreen",             { 0x2e2e, 0x8b8b, 0x5757, 0xffff } },
	{ "seashell",             { 0xffff, 0xf5f5, 0xeeee, 0xffff } },
	{ "sienna",               { 0xa0a0, 0x5252, 0x2d2d, 0xffff } },
	{ "silver",               { 0xc0c0, 0xc0c0, 0xc0c0, 0xffff } },
	{ "skyblue",              { 0x8787, 0xcece, 0xebeb, 0xffff } },
	{ "slateblue",            { 0x6a6a, 0x5a5a, 0xcdcd, 0xffff } },
	{ "slategray",            { 0x7070, 0x8080, 0x9090, 0xffff } },
	{ "slategrey",            { 0x7070, 0x8080, 0x9090, 0xffff } },
	{ "snow",                 { 0xffff, 0xfafa, 0xfafa, 0xffff } },
	{ "springgreen",          { 0x0000, 0xffff, 0x7f7f, 0xffff } },
	{ "steelblue",            { 0x4646, 0x8282, 0xb4b4, 0xffff } },
	{ "tan",                  { 0xd2d2, 0xb4b4, 0x8c8c, 0xffff } },
	{ "teal",                 { 0x0000, 0x8080, 0x8080, 0xffff } },
	{ "thistle",              { 0xd8d8, 0xbfbf, 0xd8d8, 0xffff } },
	{ "tomato",               { 0xffff, 0x6363, 0x4747, 0xffff } },
	{ "turquoise",            { 0x4040, 0xe0e0, 0xd0d0, 0xffff } },
	{ "violet",               { 0xeeee, 0x8282, 0xeeee, 0xffff } },
	{ "wheat",                { 0xf5f5, 0xdede, 0xb3b3, 0xffff } },
	{ "whitesmoke",           { 0xf5f5, 0xf5f5, 0xf5f5, 0xffff } },
	{ "yellowgreen",          { 0x9a9a, 0xcdcd, 0x3232, 0xffff } },
#else /* !AG_LARGE */
	/*
	 * Base (24-bit)
	 */
	{ "red",     { 255, 0,   0,   255 } },
	{ "green",   { 0,   128, 0,   255 } },
	{ "blue",    { 0,   0,   255, 255 } },
	{ "yellow",  { 255, 255, 0,   255 } },
	{ "cyan",    { 0,   255, 255, 255 } },
	{ "magenta", { 255, 0,   255, 255 } },
	{ "black",   { 0,   0,   0,   255 } },
	{ "grey",    { 128, 128, 128, 255 } },
	{ "gray",    { 128, 128, 128, 255 } },
	{ "white",   { 255, 255, 255, 255 } },
	/*
	 * CSS (24-bit)
	 */
	{ "aliceblue",            { 240, 248, 255, 255 } },
	{ "antiquewhite",         { 250, 235, 215, 255 } },
	{ "aqua",                 {   0, 255, 255, 255 } },
	{ "aquamarine",           { 127, 255, 212, 255 } },
	{ "azure",                { 240, 255, 255, 255 } },
	{ "beige",                { 245, 245, 220, 255 } },
	{ "bisque",               { 255, 228, 196, 255 } },
	{ "blanchedalmond",       { 255, 235, 205, 255 } },
	{ "blueviolet",           { 138,  43, 226, 255 } },
	{ "brown",                { 165,  42,  42, 255 } },
	{ "burlywood",            { 222, 184, 135, 255 } },
	{ "cadetblue",            {  95, 158, 160, 255 } },
	{ "chartreuse",           { 127, 255,   0, 255 } },
	{ "chocolate",            { 210, 105,  30, 255 } },
	{ "coral",                { 255, 127,  80, 255 } },
	{ "cornflowerblue",       { 100, 149, 237, 255 } },
	{ "cornsilk",             { 255, 248, 220, 255 } },
	{ "crimson",              { 220,  20,  60, 255 } },
	{ "darkblue",             { 0,     0, 139, 255 } },
	{ "darkcyan",             { 0,   139, 139, 255 } },
	{ "darkgoldenrod",        { 184, 134,  11, 255 } },
	{ "darkgray",             { 169, 169, 169, 255 } },
	{ "darkgreen",            { 0,   100,   0, 255 } },
	{ "darkgrey",             { 169, 169, 169, 255 } },
	{ "darkkhaki",            { 189, 183, 107, 255 } },
	{ "darkmagenta",          { 139,   0, 139, 255 } },
	{ "darkolivegreen",       { 85,  107,  47, 255 } },
	{ "darkorange",           { 255, 140,   0, 255 } },
	{ "darkorchid",           { 153,  50, 204, 255 } },
	{ "darkred",              { 139,   0,   0, 255 } },
	{ "darksalmon",           { 233, 150, 122, 255 } },
	{ "darkseagreen",         { 143, 188, 143, 255 } },
	{ "darkslateblue",        { 72,   61, 139, 255 } },
	{ "darkslategray",        { 47,   79,  79, 255 } },
	{ "darkslategrey",        { 47,   79,  79, 255 } },
	{ "darkturquoise",        { 0,   206, 209, 255 } },
	{ "darkviolet",           { 148,   0, 211, 255 } },
	{ "deeppink",             { 255,  20, 147, 255 } },
	{ "deepskyblue",          { 0,   191, 255, 255 } },
	{ "dimgray",              { 105, 105, 105, 255 } },
	{ "dimgrey",              { 105, 105, 105, 255 } },
	{ "dodgerblue",           { 30,  144, 255, 255 } },
	{ "firebrick",            { 178,  34,  34, 255 } },
	{ "floralwhite",          { 255, 250, 240, 255 } },
	{ "forestgreen",          { 34,  139,  34, 255 } },
	{ "fuchsia",              { 255,   0, 255, 255 } },
	{ "gainsboro",            { 220, 220, 220, 255 } },
	{ "ghostwhite",           { 248, 248, 255, 255 } },
	{ "gold",                 { 255, 215,   0, 255 } },
	{ "goldenrod",            { 218, 165,  32, 255 } },
	{ "greenyellow",          { 173, 255,  47, 255 } },
	{ "honeydew",             { 240, 255, 240, 255 } },
	{ "hotpink",              { 255, 105, 180, 255 } },
	{ "indianred",            { 205,  92,  92, 255 } },
	{ "indigo",               { 75,    0, 130, 255 } },
	{ "ivory",                { 255, 255, 240, 255 } },
	{ "khaki",                { 240, 230, 140, 255 } },
	{ "lavender",             { 230, 230, 250, 255 } },
	{ "lavenderblush",        { 255, 240, 245, 255 } },
	{ "lawngreen",            { 124, 252,   0, 255 } },
	{ "lemonchiffon",         { 255, 250, 205, 255 } },
	{ "lightblue",            { 173, 216, 230, 255 } },
	{ "lightcoral",           { 240, 128, 128, 255 } },
	{ "lightcyan",            { 224, 255, 255, 255 } },
	{ "lightgoldenrodyellow", { 250, 250, 210, 255 } },
	{ "lightgray",            { 211, 211, 211, 255 } },
	{ "lightgreen",           { 144, 238, 144, 255 } },
	{ "lightgrey",            { 211, 211, 211, 255 } },
	{ "lightpink",            { 255, 182, 193, 255 } },
	{ "lightsalmon",          { 255, 160, 122, 255 } },
	{ "lightseagreen",        {  32, 178, 170, 255 } },
	{ "lightskyblue",         { 135, 206, 250, 255 } },
	{ "lightslategray",       { 119, 136, 153, 255 } },
	{ "lightslategrey",       { 119, 136, 153, 255 } },
	{ "lightsteelblue",       { 176, 196, 222, 255 } },
	{ "lightyellow",          { 255, 255, 224, 255 } },
	{ "lime",                 { 0,   255,   0, 255 } },
	{ "limegreen",            { 50,  205,  50, 255 } },
	{ "linen",                { 250, 240, 230, 255 } },
	{ "maroon",               { 128,   0,   0, 255 } },
	{ "mediumaquamarine",     { 102, 205, 170, 255 } },
	{ "mediumblue",           { 0,     0, 205, 255 } },
	{ "mediumorchid",         { 186,  85, 211, 255 } },
	{ "mediumpurple",         { 147, 112, 219, 255 } },
	{ "mediumseagreen",       { 60,  179, 113, 255 } },
	{ "mediumslateblue",      { 123, 104, 238, 255 } },
	{ "mediumspringgreen",    { 0,   250, 154, 255 } },
	{ "mediumturquoise",      { 72,  209, 204, 255 } },
	{ "mediumvioletred",      { 199,  21, 133, 255 } },
	{ "midnightblue",         { 25,   25, 112, 255 } },
	{ "mintcream",            { 245, 255, 250, 255 } },
	{ "mistyrose",            { 255, 228, 225, 255 } },
	{ "moccasin",             { 255, 228, 181, 255 } },
	{ "navajowhite",          { 255, 222, 173, 255 } },
	{ "navy",                 {   0,   0, 128, 255 } },
	{ "oldlace",              { 253, 245, 230, 255 } },
	{ "olive",                { 128, 128,   0, 255 } },
	{ "olivedrab",            { 107, 142,  35, 255 } },
	{ "orange",               { 255, 165,   0, 255 } },
	{ "orangered",            { 255, 69,    0, 255 } },
	{ "orchid",               { 218, 112, 214, 255 } },
	{ "palegoldenrod",        { 238, 232, 170, 255 } },
	{ "palegreen",            { 152, 251, 152, 255 } },
	{ "paleturquoise",        { 175, 238, 238, 255 } },
	{ "palevioletred",        { 219, 112, 147, 255 } },
	{ "papayawhip",           { 255, 239, 213, 255 } },
	{ "peachpuff",            { 255, 218, 185, 255 } },
	{ "peru",                 { 205, 133,  63, 255 } },
	{ "pink",                 { 255, 192, 203, 255 } },
	{ "plum",                 { 221, 160, 221, 255 } },
	{ "powderblue",           { 176, 224, 230, 255 } },
	{ "purple",               { 128,   0, 128, 255 } },
	{ "rosybrown",            { 188, 143, 143, 255 } },
	{ "royalblue",            {  65, 105, 225, 255 } },
	{ "saddlebrown",          { 139,  69,  19, 255 } },
	{ "salmon",               { 250, 128, 114, 255 } },
	{ "sandybrown",           { 244, 164,  96, 255 } },
	{ "seagreen",             {  46, 139,  87, 255 } },
	{ "seashell",             { 255, 245, 238, 255 } },
	{ "sienna",               { 160,  82,  45, 255 } },
	{ "silver",               { 192, 192, 192, 255 } },
	{ "skyblue",              { 135, 206, 235, 255 } },
	{ "slateblue",            { 106,  90, 205, 255 } },
	{ "slategray",            { 112, 128, 144, 255 } },
	{ "slategrey",            { 112, 128, 144, 255 } },
	{ "snow",                 { 255, 250, 250, 255 } },
	{ "springgreen",          { 0,   255, 127, 255 } },
	{ "steelblue",            { 70,  130, 180, 255 } },
	{ "tan",                  { 210, 180, 140, 255 } },
	{ "teal",                 { 0,   128, 128, 255 } },
	{ "thistle",              { 216, 191, 216, 255 } },
	{ "tomato",               { 255,  99,  71, 255 } },
	{ "turquoise",            { 64,  224, 208, 255 } },
	{ "violet",               { 238, 130, 238, 255 } },
	{ "wheat",                { 245, 222, 179, 255 } },
	{ "whitesmoke",           { 245, 245, 245, 255 } },
	{ "yellowgreen",          { 154, 205,  50, 255 } },
#endif /* !AG_LARGE */
	{ NULL,                   { 0,0,0,0 } }
};

/* Import inlinables */
#undef AG_INLINE_HEADER
#include "inline_colors.h"

static __inline__ AG_Component
PctRGB(AG_Component c, double v)
{
	double x = ((double)c)*v/100.0;

	if (x <= 0.0)            { return (0); }
	if (x >= AG_COLOR_LASTD) { return (AG_COLOR_LAST); }
	return (AG_Component)(x);
}
static __inline__ float
PctHSV(float c, double v)
{
	float x = c*((float)v)/100.0f;

	if (x < 0.0f) { return (0.0f); }
	if (x > 1.0f) { return (1.0f); }
	return (x);
}

/*
 * Parse an Agar color specification string. Acceptable forms include:
 *
 * 	"r,g,b[,a]"            # 8-bit/component RGB(A)
 * 	"rgb(r,g,b[,a])"       # The "rgb()" may be omitted
 * 	"hsv(h,s,v[,a])"       # Float (Hue,Saturation,Value) + 8-bit Alpha
 *      "#rgb[a]"              # Hex 4-bit RGB(A)
 *      "#rrggbb[aa]"          # Hex 8-bit RGB(A)
 *      "#rrrrggggbbbb[aaaa]"  # Hex 16-bit RGB(A) (needs AG_LARGE)
 *      "AliceBlue"            # Color name keyword (case-insensitive)
 * 
 * Components may be separated by `/', ':' or ',' characters.
 *
 * A component terminating with a `%' character is interpreted as a ratio in %
 * of the corresponding component of pColor (or White if pColor = NULL).
 */
void
AG_ColorFromString(AG_Color *cOut, const char *s, const AG_Color *pColor)
{
	char buf[AG_STYLE_VALUE_MAX];
	AG_Color cIn;
 	char *c, *pc;
	double v[4];
	int isPct[4], i, argc;
	enum color_format {
		FORMAT_NAME,		/* "aliceblue" */
		FORMAT_RGB,		/* "rgb(r,g,b,a)" or "r,g,b,a" */
		FORMAT_HSV		/* "hsv(h,s,v,a)" */
	} format = FORMAT_NAME;

	if (pColor != NULL) {
		cIn = *pColor;
	} else {
		AG_ColorWhite(&cIn);
	}

	Strlcpy(buf, s, sizeof(buf));
	
	for (c = &buf[0]; *c != '\0' && isspace((int)*c); c++) {
		;;
	}
	if (Strncasecmp(c,"rgb(",4) == 0) {
		format = FORMAT_RGB;
	} else if (Strncasecmp(c,"hsv(",4) == 0) {
		format = FORMAT_HSV;
	} else if (*c == '#') {
		const char *ch;

		for (ch = &c[1]; *ch != '\0'; ch++) {
			if (!isdigit(*ch) &&
			    (*ch < 'a' || *ch > 'f') &&
			    (*ch < 'A' || *ch > 'F'))
				break;
		}
		switch (ch - &c[1]) {
		case 3:					/* #rgb */
			{
				Uint16 h;

				memmove(&c[2], &c[1], strlen(&c[1])+1);
				c[0] = '0';
				c[1] = 'x';
				h = (Uint16)strtoul(c, NULL, 16);
				cOut->r = AG_4toH((h >> 8) & 0xf);
				cOut->g = AG_4toH((h >> 4) & 0xf);
				cOut->b = AG_4toH((h)      & 0xf);
				cOut->a = AG_OPAQUE;
			}
			return;
		case 4:					/* #rgba */
			{
				Uint16 h;

				memmove(&c[2], &c[1], strlen(&c[1])+1);
				c[0] = '0';
				c[1] = 'x';
				h = (Uint16)strtoul(c, NULL, 16);
				cOut->r = AG_4toH((h >> 12) & 0xf);
				cOut->g = AG_4toH((h >> 8)  & 0xf);
				cOut->b = AG_4toH((h >> 4)  & 0xf);
				cOut->a = AG_4toH((h)       & 0xf);
			}
			return;
		case 6: 				/* #rrggbb */
			{
				Uint32 h;

				memmove(&c[2], &c[1], strlen(&c[1])+1);
				c[0] = '0';
				c[1] = 'x';
				h = (Uint32)strtoul(c, NULL, 16);
				cOut->r = AG_8toH((h >> 16) & 0xff);
				cOut->g = AG_8toH((h >> 8)  & 0xff);
				cOut->b = AG_8toH((h)       & 0xff);
				cOut->a = AG_OPAQUE;
			}
			return;
		case 8: 				/* #rrggbbaa */
			{
				Uint32 h;

				memmove(&c[2], &c[1], strlen(&c[1])+1);
				c[0] = '0';
				c[1] = 'x';
				h = (Uint32)strtoul(c, NULL, 16);
				cOut->r = AG_8toH((h >> 24) & 0xff);
				cOut->g = AG_8toH((h >> 16) & 0xff);
				cOut->b = AG_8toH((h >> 8)  & 0xff);
				cOut->a = AG_8toH((h)       & 0xff);
			}
			return;
#if (AG_MODEL == AG_LARGE) && !defined(_WIN32)
		case 12: 			/* #rrrrggggbbbb */
			{
				Uint64 h;

				memmove(&c[2], &c[1], strlen(&c[1])+1);
				c[0] = '0';
				c[1] = 'x';
				h = (Uint64)strtoull(c, NULL, 16);
				cOut->r = AG_8toH((h >> 32)  & 0xffff);
				cOut->g = AG_8toH((h >> 16)  & 0xffff);
				cOut->b = AG_8toH((h)        & 0xffff);
				cOut->a = AG_OPAQUE;
			}
			return;
		case 16: 			/* #rrrrggggbbbbaaaa */
			{
				Uint64 h;

				memmove(&c[2], &c[1], strlen(&c[1])+1);
				c[0] = '0';
				c[1] = 'x';
				h = (Uint64)strtoull(c, NULL, 16);
				cOut->r = AG_8toH((h >> 48) & 0xffff);
				cOut->g = AG_8toH((h >> 32) & 0xffff);
				cOut->b = AG_8toH((h >> 16) & 0xffff);
				cOut->a = AG_8toH((h)       & 0xffff);
			}
			return;
#endif /* AG_LARGE */

		default:
			goto fail_parse;
		}
	}

	if (format != FORMAT_NAME) {
		for (; *c != '\0' && *c != '('; c++)    /* Strip "rgb(" part */
			;;
		if (*c == '\0' || c[1] == '\0') {
			goto fail_parse;
		}
		pc = &c[1];
	} else {
		if (strpbrk(c, ",:/") != NULL) {      /* Cannot be a keyword */
			format = FORMAT_RGB;
		}
		pc = &c[0];
	}

	if (format == FORMAT_NAME) {                 /* Lookup color keyword */
		const AG_ColorName *cn;

		for (cn = &agColorNames[0]; cn->name != NULL; cn++) {
			if (Strcasecmp(cn->name, pc) == 0)
				break;
		}
		if (cn->name == NULL) {
			goto fail_parse;
		}
		memcpy(cOut, &cn->c, sizeof(AG_Color));
		return;
	}

	for (i=0, argc=0; i<4; i++) {
		char *tok, *ep;

		if ((tok = AG_Strsep(&pc, ",:/")) == NULL) {
			break;
		}
		v[i] = strtod(tok, &ep);
		isPct[i] = (*ep == '%');
		argc++;
	}
	if (argc < 3) {
		goto fail_parse;
	}
	switch (format) {
	case FORMAT_RGB:
		cOut->r = isPct[0] ? PctRGB(cIn.r,v[0]) : AG_8toH(v[0]);
		cOut->g = isPct[1] ? PctRGB(cIn.g,v[1]) : AG_8toH(v[1]);
		cOut->b = isPct[2] ? PctRGB(cIn.b,v[2]) : AG_8toH(v[2]);
		cOut->a = (argc >= 4) ?
		         (isPct[3] ? PctRGB(cIn.a,v[3]) : AG_8toH(v[3])) :
			 AG_OPAQUE;
		break;
	case FORMAT_HSV:
		{
			float hue, sat, val;

			AG_Color2HSV(&cIn, &hue, &sat, &val);
			hue = isPct[0] ? PctHSV(hue, v[0]) : v[0];
			sat = isPct[1] ? PctHSV(sat, v[1]) : v[1];
			val = isPct[2] ? PctHSV(val, v[2]) : v[2];
			AG_HSV2Color(hue, sat, val, cOut);
			cOut->a = (argc >= 4) ?
			          (isPct[3] ? PctHSV(cIn.a,v[3]) : AG_8toH(v[3])) :
			          AG_OPAQUE;
			break;
		}
	default:
		break;
	}
	return;
fail_parse:
	Debug(NULL, "AG_ColorFromString: Syntax error near \"%s\"\n", s);
}

/*
 * HSV (Hue/Saturation/Value) to RGB conversion.
 */
/*
 * Map 8-bit RGB components to single-precision Hue/Saturation/Value.
 * TODO make this a const function returning an AG_ColorHSV.
 */
void
AG_MapRGB8_HSVf(Uint8 r, Uint8 g, Uint8 b,
    float *_Nonnull h, float *_Nonnull s, float *_Nonnull v)
{
	float vR, vG, vB;
	float vMin, vMax, deltaMax;
	float deltaR, deltaG, deltaB;

	vR = (float)r/255.0f;
	vG = (float)g/255.0f;
	vB = (float)b/255.0f;

	vMin = MIN3(vR, vG, vB);
	vMax = MAX3(vR, vG, vB);
	deltaMax = vMax - vMin;
	*v = vMax;
	
	if (deltaMax == 0.0) {					/* Gray */
		*h = 0.0;
		*s = 0.0;
	} else {
		*s = deltaMax / vMax;
		deltaR = ((vMax - vR)/6.0f + deltaMax/2.0f) / deltaMax;
		deltaG = ((vMax - vG)/6.0f + deltaMax/2.0f) / deltaMax;
		deltaB = ((vMax - vB)/6.0f + deltaMax/2.0f) / deltaMax;

		if (vR == vMax) {
			*h = (deltaB - deltaG)*360.0f;
		} else if (vG == vMax) {
			*h = 120.0f + (deltaR - deltaB)*360.0f;	/* 1/3 */
		} else if (vB == vMax) {
			*h = 240.0f + (deltaG - deltaR)*360.0f;	/* 2/3 */
		}

		if (*h < 0.0f)   (*h)++;
		if (*h > 360.0f) (*h)--;
	}
}

#if AG_MODEL == AG_LARGE
/*
 * Map 16-bit RGB components to single-precision Hue/Saturation/Value.
 * TODO make this a const function returning an AG_ColorHSV.
 */
void
AG_MapRGB16_HSVf(Uint16 r, Uint16 g, Uint16 b,
    float *_Nonnull h, float *_Nonnull s, float *_Nonnull v)
{
	float vR, vG, vB;
	float vMin, vMax, deltaMax;
	float deltaR, deltaG, deltaB;

	vR = (float)r/65535.0f;
	vG = (float)g/65535.0f;
	vB = (float)b/65535.0f;

	vMin = MIN3(vR, vG, vB);
	vMax = MAX3(vR, vG, vB);
	deltaMax = vMax - vMin;
	*v = vMax;
	
	if (deltaMax == 0.0) {					/* Gray */
		*h = 0.0;
		*s = 0.0;
	} else {
		*s = deltaMax / vMax;
		deltaR = ((vMax - vR)/6.0f + deltaMax/2.0f) / deltaMax;
		deltaG = ((vMax - vG)/6.0f + deltaMax/2.0f) / deltaMax;
		deltaB = ((vMax - vB)/6.0f + deltaMax/2.0f) / deltaMax;

		if (vR == vMax) {
			*h = (deltaB - deltaG)*360.0f;
		} else if (vG == vMax) {
			*h = 120.0f + (deltaR - deltaB)*360.0f;	/* 1/3 */
		} else if (vB == vMax) {
			*h = 240.0f + (deltaG - deltaR)*360.0f;	/* 2/3 */
		}

		if (*h < 0.0f)   (*h)++;
		if (*h > 360.0f) (*h)--;
	}
}
#endif /* AG_LARGE */

/* Map single-precision Hue/Saturation/Value to 8-bit RGB components. */
void
AG_MapHSVf_RGB8(float h, float s, float v,
    Uint8 *_Nonnull r, Uint8 *_Nonnull g, Uint8 *_Nonnull b)
{
	float vR, vG, vB, hv, var[3];
	int iv;

	if (s < AG_SATURATION_EPSILON) {        /* Short-circuit pure grays */
		*r = (Uint8)(v * 255.0f);
		*g = (Uint8)(v * 255.0f);
		*b = (Uint8)(v * 255.0f);
		return;
	}
	if (v < AG_VALUE_EPSILON) {             /* Short-circuit pure black */
		*r = 0;
		*g = 0;
		*b = 0;
		return;
	}
	
	hv = h/60.0f;
	iv = Floor(hv);
	var[0] = v * (1.0f - s);
	var[1] = v * (1.0f - s*(hv - iv));
	var[2] = v * (1.0f - s*(1.0f - (hv - iv)));

	switch (iv) {
	case 0:		vR = v;		vG = var[2];	vB = var[0];	break;
	case 1:		vR = var[1];	vG = v;		vB = var[0];	break;
	case 2:		vR = var[0];	vG = v;		vB = var[2];	break;
	case 3:		vR = var[0];	vG = var[1];	vB = v;		break;
	case 4:		vR = var[2];	vG = var[0];	vB = v;		break;
	default:	vR = v;		vG = var[0];	vB = var[1];	break;
	}
	
	*r = (Uint8)(vR * 255.0f);
	*g = (Uint8)(vG * 255.0f);
	*b = (Uint8)(vB * 255.0f);
}

#if AG_MODEL == AG_LARGE
/*
 * Map single-precision Hue/Saturation/Value to 16-bit RGB components.
 */
void
AG_MapHSVf_RGB16(float h, float s, float v,
    Uint16 *_Nonnull r, Uint16 *_Nonnull g, Uint16 *_Nonnull b)
{
	float vR, vG, vB, hv, var[3];
	int iv;

	if (s < AG_SATURATION_EPSILON) {        /* Short-circuit pure grays */
		*r = (Uint16)(v * 65535.0f);
		*g = (Uint16)(v * 65535.0f);
		*b = (Uint16)(v * 65535.0f);
		return;
	}
	if (v < AG_VALUE_EPSILON) {             /* Short-circuit pure black */
		*r = 0;
		*g = 0;
		*b = 0;
		return;
	}
	
	hv = h/60.0f;
	iv = Floor(hv);
	var[0] = v * (1.0f - s);
	var[1] = v * (1.0f - s*(hv - iv));
	var[2] = v * (1.0f - s*(1.0f - (hv - iv)));

	switch (iv) {
	case 0:		vR = v;		vG = var[2];	vB = var[0];	break;
	case 1:		vR = var[1];	vG = v;		vB = var[0];	break;
	case 2:		vR = var[0];	vG = v;		vB = var[2];	break;
	case 3:		vR = var[0];	vG = var[1];	vB = v;		break;
	case 4:		vR = var[2];	vG = var[0];	vB = v;		break;
	default:	vR = v;		vG = var[0];	vB = var[1];	break;
	}
	
	*r = (Uint16)(vR * 65535.0f);
	*g = (Uint16)(vG * 65535.0f);
	*b = (Uint16)(vB * 65535.0f);
}
#endif /* AG_LARGE */

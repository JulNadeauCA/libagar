/*	$Csoft: units.c,v 1.1 2003/11/15 02:03:33 vedge Exp $	*/

/*
 * Copyright (c) 2003 CubeSoft Communications, Inc.
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

#include "units.h"

/* Default unit */
const struct unit identity_unit = { "", "", 1 };

/* Units of length/distance */
const struct unit length_units[] = {
	{ "mic",	N_("Microns"),		0.000001 },
	{ "\xc2\xb5m",	N_("Micrometers"),	0.000001 },
	{ "mil",	N_("Mils"),		0.000025 },
	{ "mm",		N_("Millimeters"),	0.001 },
	{ "in",		N_("Inches"),		0.0254 },
	{ "cm",		N_("Centimeters"),	0.01 },
	{ "dm",		N_("Decimeters"),	0.1 },
	{ "lnk",	N_("Links"),		0.201168 },
	{ "span",	N_("Spans"),		0.2286 },
	{ "cbt",	N_("Cubits"),		0.4572 },
	{ "ft",		N_("Feet"),		0.3048 },
	{ "var",	N_("Varas"),		0.846668 },
	{ "yd",		N_("Yards"),		0.9144 },
	{ "m",		N_("Meters"),		1 },
	{ "fh",		N_("Fathoms"),		1.8288 },
	{ "rod",	N_("Rods"),		5.0292 },
	{ "cha",	N_("Chains"),		20.1168 },
	{ "fur",	N_("Furlongs"),		201.167981 },
	{ "cbl",	N_("Cable lengths"),	219.456 },
	{ "km",		N_("Kilometers"),	1000 },
	{ "mi",		N_("Miles"),		1609.344 },
	{ "nmi",	N_("Nautical miles"),	1852 },
	{ "lg",		N_("Leagues"),		4828.031551 },
	{ "nlg",	N_("Nautical leagues"),	5556 },
	{ "A.U.",	N_("Astronomical units"), 159598073000 },
	{ "L.Y.",	N_("Light years"),	946075309081900 },
	{ "P.S.",	N_("Parsecs"),		30856780000000000 },
	{ NULL,		NULL,			0 }
};

/* Units of area (SI derived) */
const struct unit area_units[] = {
	{ "\xc2\xb5m^2", N_("Square micrometers"),	0.000001 },
	{ "mm^2",	N_("Square millimeters"),	0.001 },
	{ "in^2",	N_("Square inches"),		0.0254 },
	{ "ft^2",	N_("Square feet"),		0.3048 },
	{ "yd^2",	N_("Square yards"),		0.9144 },
	{ "m^2",	N_("Square meters"),		1 },
	{ "km^2",	N_("Square kilometers"),	1000 },
	{ "mi^2",	N_("Square miles"),		1609.35 },
	{ NULL,		NULL,				0 }
};

/* Units of volume (SI derived) */
const struct unit volume_units[] = {
	{ "\xc2\xb5m^3", N_("Cubic micrometers"),	0.000001 },
	{ "mm^3",	N_("Cubic millimeters"),	0.001 },
	{ "in^3",	N_("Cubic inches"),		0.0254 },
	{ "ft^3",	N_("Cubic feet"),		0.3048 },
	{ "yd^3",	N_("Cubic yards"),		0.9144 },
	{ "m^3",	N_("Cubic meters"),		1 },
	{ "km^3",	N_("Cubic kilometers"),		1000 },
	{ "mi^3",	N_("Cubic miles"),		1609.35 },
	{ NULL,		NULL,				0 }
};

/* Units of speed/velocity (SI derived) */
const struct unit speed_units[] = {
	{ "\xc2\xb5m/s", N_("Micrometers per second"),	0.000001 },
	{ "mm/s",	N_("Millimeters per second"),	0.001 },
	{ "in/s",	N_("Inches per second"),	0.0254 },
	{ "ft/s",	N_("Feet per second"),		0.3048 },
	{ "yd/s",	N_("Yards per second"),		0.9144 },
	{ "m/s",	N_("Meters per second"),	1 },
	{ "km/s",	N_("Kilometers per second"),	1000 },
	{ "mi/s",	N_("Miles per second"),		1609.35 },
	{ NULL,		NULL,				0 }
};

/* Units of weight */
const struct unit mass_units[] = {
	{ "\xc2\xb5g",	N_("Micrograms"),	0.000001 },
	{ "mg",		N_("Milligrams"),	0.001 },
	{ "gr",		N_("Grains"),		0.0648 },
	{ "cg",		N_("Centigrams"),	0.01 },
	{ "dg",		N_("Decigrams"),	0.1 },
	{ "car",	N_("Carats"),		0.2 },		/* Troy */
	{ "g", 		N_("Grams"),		1 },
	{ "sc", 	N_("Scruples"),		1.296 },	/* Apot */
	{ "pw", 	N_("Pennyweight"),	1.5552 },
	{ "dr", 	N_("Drams"),		1.771875 },
	{ "dr(a)", 	N_("Drams [Apothecaries]"), 3.888 },
	{ "pda",	N_("Poundals"),		14.086956521739129 },
	{ "oz", 	N_("Ounces"),		28.349 },	/* Comm */
	{ "oz(t)", 	N_("Ounces [Troy]"),	31.104 },
	{ "lb",		N_("Pounds"),		453.59 },	/* Comm */
	{ "lb(t)",	N_("Pounds [Troy]"),	373.248 },
	{ "kg", 	N_("Kilograms"),	1000 },
	{ "sto",	N_("Stones"),		6530.40 },
	{ "qu",		N_("Quarters"),		11340 },
	{ "slu", 	N_("Slugs"),		14605.92 },
	{ "100wt", 	N_("100 weights"),	45360 },
	{ "t(s)",	N_("Tons [short]"),	907200 },
	{ "Mg", 	N_("Megagrams"),	1000000 },
	{ "t",		N_("Tons [metric]"),	1000000 },
	{ "t(l)",	N_("Tons [long]"),	1016064 },
	{ NULL,		NULL,			0 }
};

/* Units of time */
const struct unit time_units[] = {
	{ "\xc2\xb5s",	N_("Microseconds"),	0.000001 },
	{ "ms",		N_("Milliseconds"),	0.001 },
	{ "s", 		N_("Seconds"),		1 },
	{ "m",		N_("Minutes"),		60 },
	{ "h", 		N_("Hours"),		3600 },
	{ "d",		N_("Days"),		86400 },
	{ "Ed",		N_("Weeks [Erisian]"),	432000 },
	{ "w",		N_("Weeks"),		604800 },
	{ "Em",		N_("Months [Erisian]"),	6307200 },
	{ "y",		N_("Years"),		31104000 },
	{ NULL,		NULL,			0 }
};

/* Units of electrical current */
const struct unit current_units[] = {
	{ "pA",		N_("Picoamperes"),	0.000000000001 },
	{ "nA",		N_("Nanoamperes"),	0.000000001 },
	{ "\xc2\xb5\x41", N_("Microamperes"),	0.000001 },
	{ "mA",		N_("Milliamperes"),	0.001 },
	{ "A",		N_("Amperes"),		1 },
	{ "kA",		N_("Kiloamperes"),	1000 },
	{ "MA",		N_("Megaamperes"),	1000000 },
	{ NULL,		NULL,			0 }
};

/* Units of temperature */
const struct unit temperature_units[] = {
	{ "\xc2\xb5K",	N_("Microkelvins"),	0.000001 },
	{ "mK",		N_("Millikelvins"),	0.001 },
	{ "Ra",		N_("Degrees Rankine"),	0.5555556 },
	{ "K",		N_("Kelvins"),		1 },
	{ "F",		N_("Degrees Farenheit"), 255.9277778 },
	{ "C",		N_("Degrees Celsius"),	274.15 },
	{ "Re",		N_("Degrees Reaumur"),	274.4 },
	{ "kK",		N_("Kilokelvins"),	1000 },
	{ "MK",		N_("Megakelvins"),	1000000 },
	{ NULL,		NULL,			0 }
};

/* Units of substance amount */
const struct unit substance_amount_units[] = {
	{ "\xc2\xb5mol", N_("Micromoles"),	0.000001 },
	{ "mmol",	N_("Millimoles"),	0.001 },
	{ "mol",	N_("Moles"),		1 },
	{ "kmol",	N_("Kilomoles"),	1000 },
	{ "Mmol",	N_("Megamoles"),	1000000 },
	{ NULL,		NULL,			0 }
};

/* Units of light measurement */
const struct unit light_units[] = {
	{ "\xc2\xb5\x63\x64", N_("Microcandelas"), 0.000001 },
	{ "mcd",	N_("Millicandelas"),	0.001 },
	{ "cd",		N_("Candelas"),		1 },
	{ "kcd",	N_("Kilocandelas"),	1000 },
	{ "Mcd",	N_("Megacandelas"),	1000000 },
	{ NULL,		NULL,			0 }
};

/* Units of power */
const struct unit power_units[] = {
	{ "\xc2\xb5W",	N_("Microwatts"),	0.000001 },
	{ "mW",		N_("Milliwatts"),	0.001 },
	{ "BTU/h",	N_("BTU/hour"),		0.292875 },
	{ "f-lbs/s",	N_("Foot-lbs/sec"),	1.355818 },
	{ "W",		N_("Watts"),		1 },
	{ "kC/m",	N_("Kilocalories/min"),	69.733 },
	{ "HP",		N_("Horsepower"),	746 },
	{ "kW",		N_("Kilowatts"),	1000 },
	{ "kC/s",	N_("Kilocalories/sec"),	4183.98 },
	{ "MW",		N_("Megawatts"),	1000000 },
	{ "GW",		N_("Gigawatts"),	1000000000 },
	{ NULL,		NULL,			0 }
};

/* Units of electromotive force */
const struct unit emf_units[] = {
	{ "\xc2\xb5V",	N_("Microvolts"),	0.000001 },
	{ "mV",		N_("Millivolts"),	0.001 },
	{ "V",		N_("Volts"),		1 },
	{ "kV",		N_("Kilovolts"),	1000 },
	{ "MV",		N_("Megavolts"),	1000000 },
	{ NULL,		NULL,			0 }
};


/*	$Csoft: units.c,v 1.16 2004/05/06 06:23:38 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004 CubeSoft Communications, Inc.
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

/*
 * Return the unit in the given group with a matching abbreviation.
 * If abbr=NULL, return the base unit.
 */
const struct unit *
unit(const struct unit group[], const char *abbr)
{
	const struct unit *unit;

	for (unit = &group[0]; unit->abbr != NULL; unit++) {
		if (abbr == NULL) {
			/* Assume the first identity to be the base unit. */
			if (unit->divider == 1)
				return (unit);
		} else {
			if (strcmp(unit->abbr, abbr) == 0)
				return (unit);
		}
	}
	return (NULL);
}

/* Convert from n in given unit to base unit. */
double
unit2base(double n, const struct unit *unit)
{
	return (unit->func != NULL ? unit->func(n) : n*unit->divider);
}

/* Convert from n in base unit to given unit. */
double
base2unit(double n, const struct unit *unit)
{
	if (unit->func != NULL) {
		double rv;

		rv = unit->func(n);
		return (rv != 0 ? 1/rv : 0);
	} else {
		return (n/unit->divider);
	}
}

/* Convert n from one unit system to another. */
double
unit2unit(double n, const struct unit *ufrom, const struct unit *uto)
{
	return (base2unit(unit2base(n, ufrom), uto));
}

/* Default unit (identity) */
const struct unit identity_unit = { "", "", 1, NULL };

/* Units of length/distance */
const struct unit length_units[] = {
	{ "\xc3\x85",	N_("\xc3\x85ngstroms"),	0.0000000001,		NULL },
	{ "\xc2\xb5",	N_("Microns"),		0.000001,		NULL },
	{ "\xc2\xb5m",	N_("Micrometers"),	0.000001,		NULL },
	{ "mil",	N_("Mils"),		0.000025,		NULL },
	{ "mm",		N_("Millimeters"),	0.001,			NULL },
	{ "in",		N_("Inches"),		0.0254,			NULL },
	{ "cm",		N_("Centimeters"),	0.01,			NULL },
	{ "dm",		N_("Decimeters"),	0.1,			NULL },
	{ "lnk",	N_("Links"),		0.201168,		NULL },
	{ "span",	N_("Spans"),		0.2286,			NULL },
	{ "cbt",	N_("Cubits"),		0.4572,			NULL },
	{ "ft",		N_("Feet"),		0.3048,			NULL },
	{ "var",	N_("Varas"),		0.846668,		NULL },
	{ "yd",		N_("Yards"),		0.9144,			NULL },
	{ "m",		N_("Meters"),		1,			NULL },
	{ "fh",		N_("Fathoms"),		1.8288,			NULL },
	{ "rod",	N_("Rods"),		5.0292,			NULL },
	{ "cha",	N_("Chains"),		20.1168,		NULL },
	{ "fur",	N_("Furlongs"),		201.167981,		NULL },
	{ "cbl",	N_("Cable lengths"),	219.456,		NULL },
	{ "km",		N_("Kilometers"),	1000,			NULL },
	{ "mi",		N_("Miles"),		1609.344,		NULL },
	{ "nmi",	N_("Nautical miles"),	1852,			NULL },
	{ "lg",		N_("Leagues"),		4828.031551,		NULL },
	{ "nlg",	N_("Nautical leagues"),	5556,			NULL },
	{ "A.U.",	N_("Astronomical units"), 159598073000.0,	NULL },
	{ "L.Y.",	N_("Light years"),	946075309081900.0,	NULL },
	{ "P.S.",	N_("Parsecs"),		30856780000000000.0,	NULL },
	{ NULL,		NULL,			0,			NULL }
};

/* Units of area (SI derived) */
const struct unit area_units[] = {
	{ "\xc2\xb5\xc2\xb2", N_("Square micrometers"),	0.000001,	NULL },
	{ "mm\xc2\xb2",	N_("Square millimeters"),	0.001,		NULL },
	{ "cm\xc2\xb2",	N_("Square centimeters"),	0.01,		NULL },
	{ "in\xc2\xb2",	N_("Square inches"),		0.0254,		NULL },
	{ "ft\xc2\xb2",	N_("Square feet"),		0.3048,		NULL },
	{ "yd\xc2\xb2",	N_("Square yards"),		0.9144,		NULL },
	{ "m\xc2\xb2",	N_("Square meters"),		1,		NULL },
	{ "km\xc2\xb2",	N_("Square kilometers"),	1000,		NULL },
	{ "mi\xc2\xb2",	N_("Square miles"),		1609.35,	NULL },
	{ NULL,		NULL,				0,		NULL }
};

/* Units of volume (SI derived) */
const struct unit volume_units[] = {
	{ "\xc2\xb5m\xc2\xb3", N_("Cubic micrometers"),	0.000001,	NULL },
	{ "mm\xc2\xb3",	N_("Cubic millimeters"),	0.001,		NULL },
	{ "cm\xc2\xb3",	N_("Cubic centimeters"),	0.01,		NULL },
	{ "in\xc2\xb3",	N_("Cubic inches"),		0.0254,		NULL },
	{ "ft\xc2\xb3",	N_("Cubic feet"),		0.3048,		NULL },
	{ "yd\xc2\xb3",	N_("Cubic yards"),		0.9144,		NULL },
	{ "m\xc2\xb3",	N_("Cubic meters"),		1,		NULL },
	{ "km\xc2\xb3",	N_("Cubic kilometers"),		1000,		NULL },
	{ "mi\xc2\xb3",	N_("Cubic miles"),		1609.35,	NULL },
	{ NULL,		NULL,				0,		NULL }
};

/* Units of speed/velocity (SI derived) */
const struct unit speed_units[] = {
	{ "\xc2\xb5m/s", N_("Micrometers per second"),	0.000001,	NULL },
	{ "mm/s",	N_("Millimeters per second"),	0.001,		NULL },
	{ "cm/s",	N_("Centimeters per second"),	0.01,		NULL },
	{ "in/s",	N_("Inches per second"),	0.0254,		NULL },
	{ "ft/s",	N_("Feet per second"),		0.3048,		NULL },
	{ "yd/s",	N_("Yards per second"),		0.9144,		NULL },
	{ "m/s",	N_("Meters per second"),	1,		NULL },
	{ "km/s",	N_("Kilometers per second"),	1000,		NULL },
	{ "mi/s",	N_("Miles per second"),		1609.35,	NULL },
	{ NULL,		NULL,				0,		NULL }
};

/* Units of weight */
const struct unit mass_units[] = {
	{ "\xc2\xb5g",	N_("Micrograms"),	0.000001,		NULL },
	{ "mg",		N_("Milligrams"),	0.001,			NULL },
	{ "gr",		N_("Grains"),		0.0648,			NULL },
	{ "cg",		N_("Centigrams"),	0.01,			NULL },
	{ "dg",		N_("Decigrams"),	0.1,			NULL },
	{ "car",	N_("Carats [troy]"),	0.2,			NULL },
	{ "g", 		N_("Grams"),		1,			NULL },
	{ "sc", 	N_("Scruples [apot]"),	1.296,			NULL },
	{ "pw", 	N_("Pennyweight"),	1.5552,			NULL },
	{ "dr", 	N_("Drams"),		1.771875,		NULL },
	{ "dr(a)", 	N_("Drams [apot]"),	3.888,			NULL },
	{ "pda",	N_("Poundals"),		14.086956521739129,	NULL },
	{ "oz", 	N_("Ounces [comm]"),	28.349,			NULL },
	{ "oz(t)", 	N_("Ounces [troy]"),	31.104,			NULL },
	{ "lb",		N_("Pounds [comm]"),	453.59,			NULL },
	{ "lb(t)",	N_("Pounds [troy]"),	373.248,		NULL },
	{ "kg", 	N_("Kilograms"),	1000,			NULL },
	{ "sto",	N_("Stones"),		6530.40,		NULL },
	{ "qu",		N_("Quarters"),		11340,			NULL },
	{ "slu", 	N_("Slugs"),		14605.92,		NULL },
	{ "100wt", 	N_("100 weights"),	45360,			NULL },
	{ "t(s)",	N_("Tons [short]"),	907200,			NULL },
	{ "Mg", 	N_("Megagrams"),	1000000,		NULL },
	{ "t",		N_("Tons [metric]"),	1000000,		NULL },
	{ "t(l)",	N_("Tons [long]"),	1016064,		NULL },
	{ NULL,		NULL,			0,			NULL }
};

/* Units of time */
const struct unit time_units[] = {
	{ "ns",		N_("Nanoseconds"),	0.000000001,		NULL },
	{ "\xc2\xb5s",	N_("Microseconds"),	0.000001,		NULL },
	{ "ms",		N_("Milliseconds"),	0.001,			NULL },
	{ "s", 		N_("Seconds"),		1,			NULL },
	{ "m",		N_("Minutes"),		60,			NULL },
	{ "h", 		N_("Hours"),		3600,			NULL },
	{ "d",		N_("Days"),		86400,			NULL },
	{ "\xce\x9a\x64", N_("Weeks [Erisian]"), 432000,		NULL },
	{ "w",		N_("Weeks"),		604800,			NULL },
	{ "\xce\x9am",	N_("Months [Erisian]"),	6307200,		NULL },
	{ "y",		N_("Years"),		31104000,		NULL },
	{ NULL,		NULL,			0,			NULL }
};

/* Units of electrical current */
const struct unit current_units[] = {
	{ "pA",		N_("Picoamperes"),	0.000000000001,		NULL },
	{ "nA",		N_("Nanoamperes"),	0.000000001,		NULL },
	{ "\xc2\xb5\x41", N_("Microamperes"),	0.000001,		NULL },
	{ "mA",		N_("Milliamperes"),	0.001,			NULL },
	{ "A",		N_("Amperes"),		1,			NULL },
	{ "kA",		N_("Kiloamperes"),	1000,			NULL },
	{ "MA",		N_("Megaamperes"),	1000000,		NULL },
	{ NULL,		NULL,			0,			NULL }
};

/* Units of temperature */
const struct unit temperature_units[] = {
	{ "\xc2\xb5K",	N_("Microkelvins"),		0.000001,	NULL },
	{ "mK",		N_("Millikelvins"),		0.001,		NULL },
	{ "\xc2\xb0\x52", N_("Degrees Rankine"),	0.5555556,	NULL },
	{ "K",		N_("Kelvins"),			1,		NULL },
	{ "\xc2\xb0\x46", N_("Degrees Farenheit"),	255.9277778,	NULL },
	{ "\xc2\xb0\x43", N_("Degrees Celsius"),	274.15,		NULL },
	{ "\xc2\xb0\x65", N_("Degrees Reaumur"),	274.4,		NULL },
	{ "kK",		N_("Kilokelvins"),		1000,		NULL },
	{ "MK",		N_("Megakelvins"),		1000000,	NULL },
	{ NULL,		NULL,				0,		NULL }
};

/* Units of substance amount */
const struct unit substance_amount_units[] = {
	{ "pmol",	N_("Picomoles"),	0.000000000001,		NULL },
	{ "\xc2\xb5mol",N_("Micromoles"),	0.000001,		NULL },
	{ "mmol",	N_("Millimoles"),	0.001,			NULL },
	{ "mol",	N_("Moles"),		1,			NULL },
	{ "kmol",	N_("Kilomoles"),	1000,			NULL },
	{ "Mmol",	N_("Megamoles"),	1000000,		NULL },
	{ NULL,		NULL,			0,			NULL }
};

/* Units of light measurement */
const struct unit light_units[] = {
	{ "\xc2\xb5\x63\x64", N_("Microcandelas"),	0.000001,	NULL },
	{ "mcd",	N_("Millicandelas"),		0.001,		NULL },
	{ "cd",		N_("Candelas"),			1,		NULL },
	{ "kcd",	N_("Kilocandelas"),		1000,		NULL },
	{ "Mcd",	N_("Megacandelas"),		1000000,	NULL },
	{ NULL,		NULL,				0,		NULL }
};

/* Units of power */
const struct unit power_units[] = {
	{ "\xc2\xb5W",	N_("Microwatts"),	0.000001,		NULL },
	{ "mW",		N_("Milliwatts"),	0.001,			NULL },
	{ "BTU/h",	N_("BTU/hour"),		0.292875,		NULL },
	{ "f-lbs/s",	N_("Foot-lbs/sec"),	1.355818,		NULL },
	{ "W",		N_("Watts"),		1,			NULL },
	{ "kC/m",	N_("Kilocalories/min"),	69.733,			NULL },
	{ "HP",		N_("Horsepower"),	746,			NULL },
	{ "kW",		N_("Kilowatts"),	1000,			NULL },
	{ "kC/s",	N_("Kilocalories/sec"),	4183.98,		NULL },
	{ "MW",		N_("Megawatts"),	1000000,		NULL },
	{ "GW",		N_("Gigawatts"),	1000000000,		NULL },
	{ NULL,		NULL,			0,			NULL }
};

/* Units of electromotive force */
const struct unit emf_units[] = {
	{ "\xc2\xb5V",	N_("Microvolts"),	0.000001,		NULL },
	{ "mV",		N_("Millivolts"),	0.001,			NULL },
	{ "V",		N_("Volts"),		1,			NULL },
	{ "kV",		N_("Kilovolts"),	1000,			NULL },
	{ "MV",		N_("Megavolts"),	1000000,		NULL },
	{ NULL,		NULL,			0,			NULL }
};

/* Units of electrical resistance */
const struct unit resistance_units[] = {
	{ "\xc2\xb5\xce\xa9",	N_("Microohms"),	0.000001,	NULL },
	{ "m\xce\xa9",		N_("Milliohms"),	0.001,		NULL },
	{ "\xce\xa9",		N_("Ohms"),		1,		NULL },
	{ "k\xce\xa9",		N_("Kilohms"),		1000,		NULL },
	{ "M\xce\xa9",		N_("Megohms"),		1000000,	NULL },
	{ NULL,			NULL,			0,		NULL }
};

/* Units of electrical capacitance */
const struct unit capacitance_units[] = {
	{ "pF",			N_("Picofarads"),	0.000000000001,	NULL },
	{ "nF",			N_("Nanofarads"),	0.000000001,	NULL },
	{ "\xc2\xb5\x46",	N_("Microfarads"),	0.000001,	NULL },
	{ "mF"	,		N_("Millifarads"),	0.001,		NULL },
	{ "F",			N_("Farads"),		1,		NULL },
	{ "kF",			N_("Kilofarads"),	1000,		NULL },
	{ "MF",			N_("Megafarads"),	1000000,	NULL },
	{ NULL,			NULL,			0,		NULL }
};

/* Units of electrical inductance */
const struct unit inductance_units[] = {
	{ "\xc2\xb5\x48",	N_("Microhenries"),	0.000001,	NULL },
	{ "mH"	,		N_("Millihenries"),	0.001,		NULL },
	{ "H",			N_("Henries"),		1,		NULL },
	{ "kH",			N_("Kilohenries"),	1000,		NULL },
	{ NULL,			NULL,			0,		NULL }
};

/* Units of frequency */
const struct unit frequency_units[] = {
	{ "\xc2\xb5Hz",		N_("Microhertz"),	0.000001,	NULL },
	{ "mHz"	,		N_("Millihertz"),	0.001,		NULL },
	{ "Hz",			N_("Hertz"),		1,		NULL },
	{ "kHz",		N_("Kilohertz"),	1000,		NULL },
	{ "MHz",		N_("Megahertz"),	1000000,	NULL },
	{ "GHz",		N_("Gigahertz"),	1000000000,	NULL },
	{ NULL,			NULL,			0,		NULL }
};

/* Units of pressure and stress */
const struct unit pressure_units[] = {
	{ "Pa",			N_("Pascals"),		1,		NULL },
	{ "hPa",		N_("Hectopascals"),	100,		NULL },
	{ "Mba",		N_("Millibars"),	100,		NULL },
	{ "kPa",		N_("Kilopascals"),	1000,		NULL },
	{ "Bar",		N_("Bars"),		100000,		NULL },
	{ "Kg-f/m\xc2\xb2",	N_("Kg-force per square meters"), 9.80665,
	  NULL },
	{ "Cm H\xc2\xb2O",	N_("Centimeters of water"), 98.0665,	NULL },
	{ "In H\xc2\xb2O",	N_("Inches of water"),	249.08891,	NULL },
	{ "Cm Hg",		N_("Centimeters of mercury"), 1333.22,	NULL },
	{ "Ft H\xc2\xb2O",	N_("Feet of water"),	2989.06692,	NULL },
	{ "In Hg",		N_("Inches of mercury"), 3386.388,	NULL },
	{ "m H\xc2\xb2O",	N_("Meters of water"),	9806.65,	NULL },
	{ "Ki/In\xc2\xb2",	N_("Kips per square inch"), 6894760,	NULL },
	{ "P/A",		N_("Pluto Atmospheres"), 0.5,		NULL },
	{ "E/A",		N_("Earth Atmospheres"), 101325,	NULL },
	{ "M/A",		N_("Mars Atmospheres"), 1000,		NULL },
	{ NULL,			NULL,			0,		NULL }
};

/* Units of metabolic cost of physical activity */
const struct unit metabolic_expenditure_units[] = {
	{ "MET",	N_("Metabolic equivalent"),		1,	NULL },
	{ "Kcal/min",	N_("Kilokalories per minute"),		1,	NULL },
	{ "O\xc2/kg",	N_("Ml O\xc2/kg/minute"),		3.5,	NULL },
	{ "Mess",	N_("Attending church"),			2.5,	NULL },
	{ "Slo-mos",	N_("Very slow walking (<2.0mph)"),	2,	NULL },
	{ "Slomarches",	N_("Slow walking (2.0mph)"),		2.5,	NULL },
	{ "Minimarches", N_("Walking (2.5mph)"),		3,	NULL },
	{ "Marches",	N_("Moderate walking (3.0mph)"),	3.5,	NULL },
	{ "Supermarches", N_("Walking - brisk (3.5mph)"),	4,	NULL },
	{ "Hypermarches", N_("Walking - very brisk (4.5mph)"),	4.5,	NULL },
	{ "Treks",	N_("Walking (3-5mph, mountains)"), 	5,	NULL },
	{ "Uptreks",	N_("Uphill walking (3.5mph)"),		6,	NULL },
	{ "Minibikes",	N_("Light biking (10-11.9mph)"),	6,	NULL },
	{ "Bikes",	N_("Moderate biking (12-13.9mph)"),	8,	NULL },
	{ "Superbikes",	N_("Vigorous biking (14-15.9mph)"),	10,	NULL },
	{ "Megabikes",	N_("Race biking (16-19mph)"),		12,	NULL },
	{ "Gigabikes",	N_("Race biking (>20mph)"),		16,	NULL },
	{ "Buttes",	N_("Mountain and rock climbing"),	8,	NULL },
	{ "Yogas",	N_("Stretching, yoga"),			4,	NULL },
	{ "Jogs",	N_("Jogging"),				7,	NULL },
	{ "Skis",	N_("General/cross-country skiing"),	7,	NULL },
	{ "Superskis",	N_("Cross-country/moderate skiing"),	8,	NULL },
	{ "Gigaskis",	N_("Cross-country/vigorous skiing"),	14,	NULL },
	{ NULL,		NULL,					0,	NULL }
};


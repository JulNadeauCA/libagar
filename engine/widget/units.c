/*	$Csoft: units.c,v 1.21 2004/08/22 11:33:19 vedge Exp $	*/

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

#define ASTRONOMICAL_UNITS
/* #define HISTORICAL_UNITS */
/* #define MET_UNITS */

const struct unit *unit_groups[] = {
	identity_unit,
	length_units,
	area_units,
	volume_units,
	speed_units,
	mass_units,
	time_units,
	current_units,
	temperature_units,
	substance_amt_units,
	light_units,
	power_units,
	emf_units,
	resistance_units,
	capacitance_units,
	inductance_units,
	frequency_units,
	pressure_units,
#ifdef MET_UNITS
	met_units,
#endif
};
const int nunit_groups = sizeof(unit_groups) / sizeof(unit_groups[0]);

/*
 * Return the unit in the given group with a matching key.
 * If key=NULL, return the base unit.
 */
const struct unit *
unit_find(const char *key)
{
	int i;

	for (i = 0; i < nunit_groups; i++) {
		const struct unit *group = unit_groups[i];
		const struct unit *unit;

		for (unit = &group[0]; unit->key != NULL; unit++) {
			if (key == NULL) {
				if (unit->divider == 1)
					return (unit);
			} else {
				if (strcmp(unit->key, key) == 0)
					return (unit);
			}
		}
	}
	fatal("no such unit: `%s'", key);
	return (NULL);
}

/* Return the unit which yields the number the most close to 0. */
const struct unit *
unit_best(const struct unit ugroup[], double n)
{
	const struct unit *unit, *bestunit = NULL;
	double nearest = n;
	double quotient;

	if (n == 0) {
		goto defunit;
	}
	for (unit = &ugroup[0]; unit->key != NULL; unit++) {
		quotient = n/unit->divider;
		if (quotient >= 1.0 && quotient <= 1e3) {
			nearest = quotient;
			bestunit = unit;
		}
	}
	if (bestunit == NULL) {
		goto defunit;
	}
	return (bestunit);
defunit:
	for (unit = &ugroup[0]; unit->key != NULL; unit++) {
		if (unit->divider == 1.0)
			break;
	}
	return (unit);
}

/* Return the abbreviation associated with the given unit. */
const char *
unit_abbr(const struct unit *unit)
{
	return (unit->abbr[0] != '\0' ? unit->abbr : unit->key);
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
const struct unit identity_unit[] = {
	{ "identity", "", "",	1.0, NULL },
	{ NULL,	NULL, NULL,	0, NULL }
};

/* Units of length/distance */
const struct unit length_units[] = {
	{ "Ang", "\xc3\x85", N_("\xc3\x85ngstroms"), 1e-10, NULL },
	{ "um", "\xc2\xb5", N_("Microns"),	1e-6, NULL },
	{ "mil", "", N_("Mils"),		25e-6, NULL },
	{ "mm", "", N_("Millimeters"),		1e-3, NULL },
	{ "in", "", N_("Inches"),		0.0254, NULL },
	{ "cm",	"", N_("Centimeters"),		1e-2, NULL },
	{ "dm",	"", N_("Decimeters"),		0.1, NULL },
	{ "ft",	"", N_("Feet"),			0.3048, NULL },
	{ "yd", "", N_("Yards"),		0.9144,	NULL },
	{ "m", "", N_("Meters"),		1.0, NULL },
	{ "km", "", N_("Kilometers"),		1000, NULL },
	{ "mi", "", N_("Miles"),		1609.344, NULL },
	{ "mi(naut)", "", N_("Nautical miles"),	1852, NULL },
	{ "leagues", "", N_("Leagues"),		4828.031551, NULL },
	{ "leagues(naut)", "", N_("Nautical leagues"), 5556, NULL },
#ifdef HISTORICAL_UNITS
	{ "lnk", "", N_("Links"),		0.201168,	NULL },
	{ "span", "", N_("Spans"),		0.2286,		NULL },
	{ "cbt", "", N_("Cubits"),		0.4572,		NULL },
	{ "var", "", N_("Varas"),		0.846668,	NULL },
	{ "fh", "", N_("Fathoms"),		1.8288,		NULL },
	{ "rod", "", N_("Rods"),		5.0292,		NULL },
	{ "cha", "", N_("Chains"),		20.1168,	NULL },
	{ "fur", "", N_("Furlongs"),		201.167981,	NULL },
	{ "cbl", "", N_("Cable lengths"),	219.456,	NULL },
#endif
#ifdef ASTRONOMICAL_UNITS
	{ "A.U.", "", N_("Astronomical units"), 159598073000.0,	NULL },
	{ "L.Y.", "", N_("Light years"),	946075309081900.0, NULL },
	{ "P.S.", "", N_("Parsecs"),		3085678e10, NULL },
#endif
	{ NULL,	NULL, NULL,			0, NULL }
};

/* Units of area (SI derived) */
const struct unit area_units[] = {
	{ "u^2", "\xc2\xb5\xc2\xb2", N_("Square micrometers"),	1e-6, NULL },
	{ "mm^2", "mm\xc2\xb2",	N_("Square millimeters"),	1e-3, NULL },
	{ "cm^2", "cm\xc2\xb2",	N_("Square centimeters"),	1e-2, NULL },
	{ "in^2", "in\xc2\xb2",	N_("Square inches"),		0.0254,	NULL },
	{ "ft^2", "ft\xc2\xb2",	N_("Square feet"),		0.3048,	NULL },
	{ "yd^2", "yd\xc2\xb2",	N_("Square yards"),		0.9144,	NULL },
	{ "m^2", "m\xc2\xb2",	N_("Square meters"),		1.0, NULL },
	{ "km^2", "km\xc2\xb2",	N_("Square kilometers"),	1000, NULL },
	{ "mi^2", "mi\xc2\xb2",	N_("Square miles"),		1609.35, NULL },
	{ NULL, NULL, NULL,					0, NULL }
};

/* Units of volume (SI derived) */
const struct unit volume_units[] = {
	{ "u^3", "\xc2\xb5m\xc2\xb3", N_("Cubic micrometers"),	1e-6, NULL },
	{ "mm^3", "mm\xc2\xb3",	N_("Cubic millimeters"),	1e-3, NULL },
	{ "cm^3", "cm\xc2\xb3",	N_("Cubic centimeters"),	1e-2, NULL },
	{ "in^3", "in\xc2\xb3",	N_("Cubic inches"),		0.0254, NULL },
	{ "ft^3", "ft\xc2\xb3",	N_("Cubic feet"),		0.3048,	NULL },
	{ "yd^3", "yd\xc2\xb3",	N_("Cubic yards"),		0.9144, NULL },
	{ "m^3", "m\xc2\xb3", N_("Cubic meters"),		1.0, NULL },
	{ "km^3", "km\xc2\xb3",	N_("Cubic kilometers"),		1e3, NULL },
	{ "mi^3", "mi\xc2\xb3",	N_("Cubic miles"),		1609.35, NULL },
	{ NULL, NULL, NULL,					0, NULL }
};

/* Units of speed/velocity (SI derived) */
const struct unit speed_units[] = {
	{ "um/s", "\xc2\xb5m/s", N_("Micrometers per second"),	1e-6, NULL },
	{ "mm/s", "", N_("Millimeters per second"),		1e-3, NULL },
	{ "cm/s", "", N_("Centimeters per second"),		1e-2, NULL },
	{ "in/s", "", N_("Inches per second"),			0.0254, NULL },
	{ "ft/s", "", N_("Feet per second"),			0.3048,	NULL },
	{ "yd/s", "", N_("Yards per second"),			0.9144,	NULL },
	{ "m/s", "", N_("Meters per second"),			1.0, NULL },
	{ "km/s", "", N_("Kilometers per second"),		1e3, NULL },
	{ "mi/s", "", N_("Miles per second"),			1609.35, NULL },
	{ NULL, NULL, NULL,					0, NULL }
};

/* Units of weight */
const struct unit mass_units[] = {
	{ "ug", "\xc2\xb5g", N_("Micrograms"),	1e-6, NULL },
	{ "mg", "", N_("Milligrams"),		1e-3, NULL },
	{ "cg", "", N_("Centigrams"),		1e-2, NULL },
	{ "dg", "", N_("Decigrams"),		1e-1, NULL },
	{ "g", "", N_("Grams"),			1.0, NULL },
	{ "oz", "", N_("Ounces [comm]"),	28.349,	NULL },
	{ "lbs", "", N_("Pounds [comm]"),	453.59,	NULL },
	{ "kg", "", N_("Kilograms"),		1e3, NULL },
	{ "t(s)", "", N_("Tons [short]"),	907200, NULL },
	{ "t", "", N_("Tons [metric]"),		1e6, NULL },
	{ "t(l)", "", N_("Tons [long]"),	1016064, NULL },
#ifdef HISTORICAL_UNITS
	{ "grains", "", N_("Grains"),			0.0648,	NULL },
	{ "carats", "", N_("Carats [troy]"),		0.2, NULL },
	{ "oz(troy)", "", N_("Ounces [troy]"),		31.104,	NULL },
	{ "lb(troy)", "", N_("Pounds [troy]"),		373.248, NULL },
	{ "scruples", "", N_("Scruples [apot]"),	1.296, NULL },
	{ "pennywts", "", N_("Pennyweights"),		1.5552, NULL },
	{ "drams", "", N_("Drams"),			1.771875, NULL },
	{ "poundals", "", N_("Poundals"),		14.086956521739, NULL },
	{ "stones", "", N_("Stones"),			6530.40, NULL },
	{ "quarters", "", N_("Quarters"),		11340, NULL },
	{ "slugs", "", N_("Slugs"),			14605.92, NULL },
	{ "100wts", "", N_("100 weights"),		45360, NULL },
	{ "drams(apot)", "", N_("Drams [apot]"),	3.888, NULL },
	{ "batmans", "", _("Batmans"),			16e6, NULL },
#endif
	{ NULL, NULL, NULL,				0, NULL }
};

/* Units of time */
const struct unit time_units[] = {
	{ "ns", "", N_("Nanoseconds"),			1e-9, NULL },
	{ "us", "\xc2\xb5s", N_("Microseconds"),	1e-6, NULL },
	{ "ms", "", N_("Milliseconds"),			1e-3, NULL },
	{ "sec", "", N_("Seconds"),			1.0, NULL },
	{ "min", "", N_("Minutes"),			60, NULL },
	{ "hr", "", N_("Hours"),			3600, NULL },
	{ "day", "", N_("Days"),			86400, NULL },
	{ "w(poee)", "\xce\x9a\x64", N_("Weeks [POEE]"), 432000, NULL },
	{ "wks", "", N_("Weeks"),			604800,	NULL },
	{ "m(poee)", "\xce\x9am", N_("Months [POEE]"),	6307200, NULL },
	{ "yrs", "", N_("Years"),			31104000, NULL },
	{ NULL,	NULL, NULL,				0, NULL }
};

/* Units of electrical current */
const struct unit current_units[] = {
	{ "pA", "", N_("Picoamperes"),			1e-12, NULL },
	{ "nA", "", N_("Nanoamperes"),			1e-9, NULL },
	{ "uA", "\xc2\xb5\x41", N_("Microamperes"),	1e-6, NULL },
	{ "mA", "", N_("Milliamperes"),			1e-3, NULL },
	{ "A", "", N_("Amperes"),			1.0, NULL },
	{ "kA", "", N_("Kiloamperes"),			1e3, NULL },
	{ "MA", "", N_("Megaamperes"),			1e6, NULL },
	{ NULL, NULL, NULL,				0, NULL }
};

/* Units of temperature */
static double degF2K(double degF) { return ((degF-32)/1.8 + 273.15); }
static double degC2K(double degC) { return (degC+273.15); }
static double degRa2K(double degRa) { return ((degRa-32-459.67)/1.8 + 273.15); }
static double degRe2K(double degR) { return (degR*1.25 + 273.15); }
const struct unit temperature_units[] = {
	{ "degC", "\xc2\xb0\x43", N_("Degrees Celsius"),	0, degC2K },
	{ "degF", "\xc2\xb0\x46", N_("Degrees Farenheit"),	0, degF2K },
#ifdef HISTORICAL_UNITS
	{ "degRa", "\xc2\xb0\x52", N_("Degrees Rankine"),	0, degRa2K },
	{ "degRe", "\xc2\xb0\x65", N_("Degrees Reaumur"),	0, degRe2K },
#endif
	{ "uk", "\xc2\xb5k", "Microkelvins",			1e-6, NULL },
	{ "mk", "", "Millikelvins",				1e-3, NULL },
	{ "k", "", "Kelvins",					1.0, NULL },
	{ "kk",	"", "Kilokelvins",				1e3, NULL },
	{ "Mk",	"", N_("Megakelvins"),				1e6, NULL },
	{ NULL,	NULL, NULL,					0, NULL }
};

/* Units of substance amount */
const struct unit substance_amt_units[] = {
	{ "pmol", "", "Picomoles",			1e-12, NULL },
	{ "umol", "\xc2\xb5mol", "Micromoles",		1e-6, NULL },
	{ "mmol", "", "Millimoles",			1e-3, NULL },
	{ "mol", "", "Moles",				1.0, NULL },
	{ "kmol", "", "Kilomoles",			1e3, NULL },
	{ "Mmol", "", N_("Megamoles"),			1e6, NULL },
	{ NULL,	NULL, NULL,				0, NULL }
};

/* Units of light measurement */
const struct unit light_units[] = {
	{ "ucd", "\xc2\xb5\x63\x64", "Microcandelas",		1e-6, NULL },
	{ "mcd", "", "Millicandelas",				1e-3, NULL },
	{ "cd", "", "Candelas",					1.0, NULL },
	{ "kcd", "", "Kilocandelas",				1e3, NULL },
	{ "Mcd", "", N_("Megacandelas"),			1e6, NULL },
	{ NULL, NULL, NULL,					0, NULL }
};

/* Units of power */
const struct unit power_units[] = {
	{ "uW", "\xc2\xb5W", "Microwatts",	1e-6, NULL },
	{ "mW", "", "Milliwatts",		1e-3, NULL },
	{ "BTU/h", "", "BTU/hr",		0.292875, NULL },
	{ "f-lbs/s", "", N_("Foot-lbs/sec"),	1.355818, NULL },
	{ "W", "", "Watts",			1.0, NULL },
	{ "kC/m", "", "Kilocalories/min",	69.733, NULL },
	{ "HP", "", N_("Horsepower"),		746, NULL },
	{ "kW", "", "Kilowatts",		1e3, NULL },
	{ "kC/s", "", "Kilocalories/sec",	4183.98, NULL },
	{ "MW", "", N_("Megawatts"),		1e6, NULL },
	{ "GW", "", "Gigawatts",		1e9, NULL },
	{ NULL, NULL, NULL,			0, NULL }
};

/* Units of electromotive force */
const struct unit emf_units[] = {
	{ "uV", "\xc2\xb5V", "Microvolts",	1e-6, NULL },
	{ "mV", "", "Millivolts",		1e-3, NULL },
	{ "V", "", "Volts",			1.0, NULL },
	{ "kV", "", "Kilovolts",		1e3, NULL },
	{ "MV", "", N_("Megavolts"),		1e6, NULL },
	{ NULL, NULL, NULL,			0, NULL }
};

/* Units of electrical resistance */
const struct unit resistance_units[] = {
	{ "uohms", "\xc2\xb5\xce\xa9", "Microohms",	1e-6, NULL },
	{ "mohms", "m\xce\xa9", "Milliohms",		1e-3, NULL },
	{ "ohms", "\xce\xa9", "Ohms",			1.0, NULL },
	{ "kohms", "k\xce\xa9",	"Kilohms",		1e3, NULL },
	{ "megaohms", "M\xce\xa9", N_("Megaohms"),	1e6, NULL },
	{ NULL, NULL, NULL,				0, NULL }
};

/* Units of electrical capacitance */
const struct unit capacitance_units[] = {
	{ "pF", "", "Picofarads",			1e-12, NULL },
	{ "nF", "", "Nanofarads",			1e-9, NULL },
	{ "uF", "\xc2\xb5\x46",	"Microfarads",		1e-6, NULL },
	{ "mF", "", "Millifarads",			1e-3, NULL },
	{ "F", "", "Farads",				1.0, NULL },
	{ "kF", "", "Kilofarads",			1e3, NULL },
	{ NULL,	NULL, NULL,				0, NULL }
};

/* Units of electrical inductance */
const struct unit inductance_units[] = {
	{ "uH", "\xc2\xb5\x48",	"Microhenries",		1e-6, NULL },
	{ "mH", "", "Millihenries",			1e-3, NULL },
	{ "H", "", "Henries",				1.0, NULL },
	{ "kH", "", "Kilohenries",			1e3, NULL },
	{ NULL, NULL, NULL,				0, NULL }
};

/* Units of frequency */
const struct unit frequency_units[] = {
	{ "uHz", "\xc2\xb5Hz","Microhertz",	1e-6, NULL },
	{ "mHz", "", "Millihertz",		1e-3, NULL },
	{ "Hz", "", "Hertz",			1.0, NULL },
	{ "kHz", "", "Kilohertz",		1e3, NULL },
	{ "MHz", "", N_("Megahertz"),		1e6, NULL },
	{ "GHz", "", "Gigahertz",		1e9, NULL },
	{ NULL, NULL, NULL,			0, NULL }
};

/* Units of pressure and stress */
const struct unit pressure_units[] = {
	{ "Pa", "", "Pascals",					1.0, NULL },
	{ "Mba", "", "Millibars",				1e2, NULL },
	{ "kPa", "", "Kilopascals",				1e3, NULL },
	{ "Bar", "", "Bars",					1e5, NULL },
	{ "Kg-f/m^2", "Kg-f/m\xc2\xb2",	N_("Kg-force per m^2"), 9.80665, NULL },
	{ "Cm H2O", "Cm H\xc2\xb2O", N_("Centimeters of water"),98.0665, NULL },
	{ "In H2O", "In H\xc2\xb2O", N_("Inches of water"),  249.08891, NULL },
	{ "Cm Hg", "", N_("Centimeters of mercury"),		1333.22, NULL },
	{ "Ft H2O", "Ft H\xc2\xb2O", N_("Feet of water"),    2989.06692, NULL },
	{ "In Hg", "", N_("Inches of mercury"),		       3386.388, NULL },
	{ "m H2O", "m H\xc2\xb2O", N_("Meters of water"),	9806.65, NULL },
	{ "Kips/in^2", "Kips/In\xc2\xb2", N_("Kips per in^2"),	6894760, NULL },
#ifdef ASTRONOMICAL_UNITS
	{ "Atm(Pluto)", "", N_("Pluto Atmospheres"),		0.5, NULL },
	{ "Atm(Earth)", "", N_("Earth Atmospheres"),		101325, NULL },
	{ "Atm(Mars)", "", N_("Mars Atmospheres"),		1e3, NULL },
#endif
	{ NULL,	NULL, NULL,					0, NULL }
};

#ifdef MET_UNITS
/* Units of metabolic cost (ie. physical activity) */
const struct unit met_units[] = {
	{ "MET", "", N_("Metabolic equivalent"),		1.0, NULL },
	{ "MESS", "", N_("Attending church"),			2.5, NULL },
	{ "Kcal/min", "", N_("Kilokalories per minute"),	1.0, NULL },
	{ "O2/kg", "O\xc2/kg", "Millilitres O\xc2/kg/min",	3.5, NULL },
	{ "Slo-mos", "", N_("Slow walking (<=2.0mph)"),		2.0, NULL },
	{ "Marches", "", N_("Moderate walking (3.0mph)"),	3.5, NULL },
	{ "Supermarches", "", N_("Walking - brisk (3.5mph)"),	4.0, NULL },
	{ "Treks", "",	N_("Walking (3.5mph) + biking (10-11.9mph)"), 6, NULL},
	{ "Hypermarches", "", N_("Walking - very brisk (4.5mph)"), 4.5,	NULL },
	{ "Bikes", "", N_("Moderate biking (12-13.9mph)"),	8, NULL },
	{ "Superbikes", "", N_("Vigorous biking (14-15.9mph)"),	10, NULL },
	{ "Megabikes", "", N_("Race biking (16-19mph)"),	12, NULL },
	{ "Gigabikes", "", N_("Race biking (>20mph)"),		16, NULL },
	{ "Buttes", "", N_("Mountain and rock climbing"),	8, NULL },
	{ "Yogas", "", N_("Stretching, yoga"),			4, NULL },
	{ "Skis", "", N_("General/cross-country skiing"),	7, NULL },
	{ "Superskis", "", N_("Cross-country/moderate skiing"),	8, NULL },
	{ "Hyperskis", "", N_("Cross-country/vigorous skiing"),	14, NULL },
	{ NULL, NULL, NULL,					0, NULL }
};
#endif

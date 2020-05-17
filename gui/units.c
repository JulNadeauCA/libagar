/*	Public domain	*/

/*
 * Built-in unit conversion routines.
 */
/* TODO handle SI generically; registration interface */

#include <agar/core/core.h>
#include <agar/gui/units.h>
#include <agar/gui/gui_math.h>

#include <string.h>

/* #define ASTRONOMICAL_UNITS */
/* #define HISTORICAL_UNITS */

const char *agUnitGroupNames[] = {
	N_("Identity"),
	N_("Length"),
	N_("Angle"),
	N_("Video"),
	N_("Area"),
	N_("Volume"),
	N_("Velocity"),
	N_("Mass"),
	N_("Time"),
	N_("Electrical current"),
	N_("Temperature"),
	N_("Substance amount"),
	N_("Light"),
	N_("Power"),
	N_("Electromotive force"),
	N_("Electrical resistance"),
	N_("First-order resistance coefficient"),
	N_("Second-order resistance coefficient"),
	N_("Capacitance"),
	N_("Inductance"),
	N_("Frequency"),
	N_("Pressure"),
	N_("Vacuum"),
	N_("Percentage"),
	N_("K units (MOSFET)"),
	NULL
};
const AG_Unit *agUnitGroups[] = {
	agIdentityUnit,
	agLengthUnits,
	agAngleUnits,
	agVideoUnits,
	agAreaUnits,
	agVolumeUnits,
	agSpeedUnits,
	agMassUnits,
	agTimeUnits,
	agCurrentUnits,
	AG_UNIT(agTemperatureUnits),		/* Nonlinear */
	agSubstanceAmountUnits,
	agEnergyPerSubstanceAmountUnits,
	agLightUnits,
	agPowerUnits,
	agEMFUnits,
	agResistanceUnits,
	agResistanceTC1Units,
	agResistanceTC2Units,
	agCapacitanceUnits,
	agInductanceUnits,
	agFrequencyUnits,
	agPressureUnits,
	agVacuumUnits,
	agPercentageUnits,
	agResistivityUnits,
	agThermalConductivityUnits,
	agThermalExpansionUnits,
	agDensityUnits,
};
const int agnUnitGroups = sizeof(agUnitGroups) / sizeof(agUnitGroups[0]);

/* Test whether the given unit is a non-linear unit. */
int
AG_UnitIsNonlinear(const char *key)
{
	return (strcmp(key, "degC") == 0 ||
	        strcmp(key, "degF") == 0);
}

/* Convert n (in specified units) to its value in base units. */
double
AG_Unit2Base(double n, const AG_Unit *unit)
{
	if (AG_UnitIsNonlinear(unit->key)) {
		return AG_UNIT_NL(unit)->func(n, 1);
	}
	return (n * unit->divider);
}

/* Convert n (in base units) to specified units. */
double
AG_Base2Unit(double n, const AG_Unit *unit)
{
	if (AG_UnitIsNonlinear(unit->key)) {
		return AG_UNIT_NL(unit)->func(n, 0);
	}
	return (n / unit->divider);
}

/* Convert n (in fromUnit units) to its value in toUnit units. */
double
AG_Unit2Unit(double n, const AG_Unit *fromUnit, const AG_Unit *toUnit)
{
	return (AG_Base2Unit(AG_Unit2Base(n, fromUnit), toUnit));
}

/* Return the abbreviation string for a given unit. */
const char *
AG_UnitAbbr(const AG_Unit *unit)
{
	return (unit->abbr[0] != '\0' ? unit->abbr : unit->key);
}

/*
 * Return the unit in the given group with a matching key.
 * If key=NULL, return the base unit.
 */
const AG_Unit *
AG_FindUnit(const char *key)
{
	int i;

	for (i = 0; i < agnUnitGroups; i++) {
		const AG_Unit *group = agUnitGroups[i];
		const AG_Unit *unit;

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
	AG_SetError(_("No such unit: %s"), key);
	return (NULL);
}

/* Return the unit which yields the number with the least figures. */
const AG_Unit *
AG_BestUnit(const AG_Unit ugroup[], double n)
{
	const AG_Unit *unit, *bestunit = NULL;
	double smallest = HUGE_VAL;
	double diff;

	if (n == 0) {
		goto defunit;
	}
	for (unit = &ugroup[0]; unit->key != NULL; unit++) {
		if (n/unit->divider >= 1.0) {
			diff = Fabs((float)(n - unit->divider));
			if (diff < smallest) {
				smallest = diff;
				bestunit = unit;
			}
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

/* Format a number using the unit most suited to its magnitude. */
int
AG_UnitFormat(double n, const AG_Unit ugroup[], char *buf, AG_Size len)
{
	const AG_Unit *ubest;

	ubest = AG_BestUnit(ugroup, n);
	return (Snprintf(buf, len, "%.2f%s", AG_Base2Unit(n, ubest),
	    ubest->abbr[0] != '\0' ? ubest->abbr : ubest->key));
}

/* Default unit */
const AG_Unit agIdentityUnit[] = {
	{ "identity", "",   "",   1.0 },
	{ NULL,	      NULL, NULL, 0   }
};

/* Units of length/distance */
const AG_Unit agLengthUnits[] = {
	{ "Ang", "\xc3\x85", N_("\xc3\x85ngstroms"),	1e-10    },
	{ "fm",  "",         N_("Femtometres"),		1e-15    },
	{ "pm",  "",         N_("Picometres"),		1e-12    },
	{ "nm",  "",         N_("Nanometres"),		1e-9     },
	{ "um",  "\xc2\xb5", N_("Microns"),		1e-6     },
	{ "mil", "",         N_("Mils"),		25e-6    },
	{ "mm",  "",         N_("Millimeters"),		1e-3     },
	{ "in",  "",         N_("Inches"),		0.0254   },
	{ "cm",  "",         N_("Centimeters"),		1e-2     },
	{ "dm",  "",         N_("Decimeters"),		0.1      },
	{ "ft",  "",         N_("Feet"),		0.3048   },
	{ "yd",  "",         N_("Yards"),		0.9144   },
	{ "m",   "",         N_("Meters"),		1.0      },
	{ "km",  "",         N_("Kilometers"),		1000     },
	{ "mi",  "",         N_("Miles"),		1609.344 },
#ifdef ASTRONOMICAL_UNITS
	{ "A.U", "",         N_("Astronomical units"),	159598073000.0    },
	{ "L.Y", "",         N_("Light years"),		946075309081900.0 },
	{ "P.S", "",         N_("Parsecs"),		3085678e10        },
#endif
#ifdef HISTORICAL_UNITS
	{ "N.M",  "", N_("Nautical miles"),	1852       },
	{ "N.L",  "", N_("Nautical leagues"),	5556       },
	{ "lnk",  "", N_("Links"),		0.201168   },
	{ "span", "", N_("Spans"),		0.2286     },
	{ "cbt",  "", N_("Cubits"),		0.4572     },
	{ "var",  "", N_("Varas"),		0.846668   },
	{ "fh",   "", N_("Fathoms"),		1.8288     },
	{ "rod",  "", N_("Rods"),		5.0292     },
	{ "cha",  "", N_("Chains"),		20.1168    },
	{ "fur",  "", N_("Furlongs"),		201.167981 },
	{ "cbl",  "", N_("Cable lengths"),	219.456    },
#endif
	{ NULL,	NULL, NULL,			0          }
};

/* Units of angle */
const AG_Unit agAngleUnits[] = {
	{ "rad",   "",         N_("Radians"),        1.0                    },
	{ "deg",   "\xc2\xb0", N_("Degrees"),        0.01745329251994329577 },
	{ "rev",   "",         N_("Revolutions"),    6.28318530717958647692 },
	{ "grad",  "",         N_("Grads"),          0.01570796326794896619 },
	{ "point", "",         N_("Points"),         0.19634954084936207740 },
	{ "brad",  "",         N_("Binary degrees"), 0.02454369260617025968 },
	{ "HA",    "",         N_("Hour angle"),     0.26179938779914943654 },
	{ NULL,    NULL,       NULL,                 0                      }
};

/* Units of length/distance on a raster display. */
/* TODO resolution-specific functions */
const AG_Unit agVideoUnits[] = {
	{ "px",  "",   N_("Pixels"),	 1.0 },
	{ "kpx", "",   N_("Kilopixels"), 1e3 },
	{ "Mpx", "",   N_("Megapixels"), 1e6 },
	{ NULL,	 NULL, NULL,		 0   }
};

/* Units of area (SI derived) */
const AG_Unit agAreaUnits[] = {
	{ "um^2", "\xc2\xb5\xc2\xb2", N_("Square micrometers"),	1e-6    },
	{ "mm^2", "mm\xc2\xb2",       N_("Square millimeters"),	1e-3    },
	{ "cm^2", "cm\xc2\xb2",       N_("Square centimeters"),	1e-2    },
	{ "in^2", "in\xc2\xb2",       N_("Square inches"),	0.0254  },
	{ "ft^2", "ft\xc2\xb2",       N_("Square feet"),	0.3048  },
	{ "yd^2", "yd\xc2\xb2",       N_("Square yards"),	0.9144  },
	{ "m^2",  "m\xc2\xb2",        N_("Square meters"),	1.0     },
	{ "km^2", "km\xc2\xb2",       N_("Square kilometers"),	1000    },
	{ "mi^2", "mi\xc2\xb2",       N_("Square miles"),	1609.35 },
	{ NULL,   NULL,               NULL,			0       }
};

/* Units of volume (SI derived) */
const AG_Unit agVolumeUnits[] = {
	{ "um^3", "\xc2\xb5m\xc2\xb3", N_("Cubic micrometers"),	1e-6    },
	{ "mm^3", "mm\xc2\xb3",        N_("Cubic millimeters"),	1e-3    },
	{ "cm^3", "cm\xc2\xb3",        N_("Cubic centimeters"),	1e-2    },
	{ "in^3", "in\xc2\xb3",        N_("Cubic inches"),	0.0254  },
	{ "ft^3", "ft\xc2\xb3",        N_("Cubic feet"),	0.3048  },
	{ "yd^3", "yd\xc2\xb3",        N_("Cubic yards"),	0.9144  },
	{ "m^3",  "m\xc2\xb3",         N_("Cubic meters"),	1.0     },
	{ "km^3", "km\xc2\xb3",        N_("Cubic kilometers"),	1e3     },
	{ "mi^3", "mi\xc2\xb3",        N_("Cubic miles"),	1609.35 },
	{ NULL,   NULL,                NULL,			0       }
};

/* Units of speed/velocity (SI derived) */
const AG_Unit agSpeedUnits[] = {
	{ "um/s", "\xc2\xb5m/s", N_("Micrometers per second"),	1e-6    },
	{ "mm/s", "",            N_("Millimeters per second"),	1e-3    },
	{ "cm/s", "",            N_("Centimeters per second"),	1e-2    },
	{ "in/s", "",            N_("Inches per second"),	0.0254  },
	{ "ft/s", "",            N_("Feet per second"),		0.3048  },
	{ "yd/s", "",            N_("Yards per second"),	0.9144  },
	{ "m/s",  "",            N_("Meters per second"),	1.0     },
	{ "km/s", "",            N_("Kilometers per second"),	1e3     },
	{ "mi/s", "",            N_("Miles per second"),	1609.35 },
	{ NULL,   NULL,          NULL,				0       }
};

/* Units of weight */
const AG_Unit agMassUnits[] = {
	{ "ug",   "\xc2\xb5g", N_("Micrograms"),	1e-6    },
	{ "mg",   "",          N_("Milligrams"),	1e-3    },
	{ "cg",   "",          N_("Centigrams"),	1e-2    },
	{ "dg",   "",          N_("Decigrams"),		1e-1    },
	{ "g",    "",          N_("Grams"),		1.0     },
	{ "oz",   "",          N_("Ounces [comm]"),	28.349  },
	{ "lb",   "",          N_("Pounds [comm]"),	453.59  },
	{ "kg",   "",          N_("Kilograms"),		1e3     },
	{ "t(s)", "",          N_("Tons [short]"),	907200  },
	{ "t",    "",          N_("Tons [metric]"),	1e6     },
	{ "t(l)", "",          N_("Tons [long]"),	1016064 },
#ifdef HISTORICAL_UNITS
	{ "grain",   "", N_("Grains"),		0.0648   },
	{ "carat",   "", N_("Carats [troy]"),	0.2      },
	{ "dram(a)", "", N_("Drams [apot]"),	3.888    },
	{ "oz(t)",   "", N_("Ounces [troy]"),	31.104   },
	{ "lb(t)",   "", N_("Pounds [troy]"),	373.248  },
	{ "scruple", "", N_("Scruples [apot]"),	1.296    },
	{ "pennywt", "", N_("Pennyweights"),	1.5552   },
	{ "dram",    "", N_("Drams"),		1.771875 },
	{ "poundal", "", N_("Poundals"),	14.08695 },
	{ "stone",   "", N_("Stones"),		6530.40  },
	{ "quarter", "", N_("Quarters"),	11340    },
	{ "slug",    "", N_("Slugs"),		14605.92 },
	{ "100wt",   "", N_("100 weights"),	45360    },
	{ "batman",  "", N_("Batmans"),		16e6     },
#endif
	{ NULL,      NULL, NULL,                0        }
};

/* Units of time */
const AG_Unit agTimeUnits[] = {
	{ "ns",  "",             N_("Nanoseconds"),	1e-9     },
	{ "us",  "\xc2\xb5s",    N_("Microseconds"),	1e-6     },
	{ "ms",  "",             N_("Milliseconds"),	1e-3     },
	{ "sec", "",             N_("Seconds"),		1.0      },
	{ "min", "",             N_("Minutes"),		60       },
	{ "hr",  "",             N_("Hours"),		3600     },
	{ "day", "",             N_("Days"),		86400    },
	{ "wk",  "",             N_("Weeks"),		604800   },
	{ "yr",  "",             N_("Years"),		31104000 },
	{ NULL,  NULL,           NULL,			0        }
};

/* Units of electrical current */
const AG_Unit agCurrentUnits[] = {
	{ "pA", "",             N_("Picoamperes"),	1e-12 },
	{ "nA", "",             N_("Nanoamperes"),	1e-9  },
	{ "uA", "\xc2\xb5\x41", N_("Microamperes"),	1e-6  },
	{ "mA", "",             N_("Milliamperes"),	1e-3  },
	{ "A",  "",             N_("Amperes"),		1.0   },
	{ "kA", "",             N_("Kiloamperes"),	1e3   },
	{ "MA", "",             N_("Megaamperes"),	1e6   },
	{ NULL, NULL,           NULL,			0     }
};

/* Units of temperature (Kelvin base with nonlinear degC/degF conversions) */
const AG_UnitNL agTemperatureUnits[] = {
	{ { "degC", "\xc2\xb0\x43", N_("Degrees Celsius"),   0    }, AG_UnitCelsius },
	{ { "degF", "\xc2\xb0\x46", N_("Degrees Farenheit"), 0    }, AG_UnitFahrenheit },
	{ { "uk",   "\xc2\xb5k",    N_("Microkelvins"),      1e-6 }, NULL },
	{ { "mk",   "",             N_("Millikelvins"),      1e-3 }, NULL },
	{ { "k",    "",             N_("Kelvins"),           1.0  }, NULL },
	{ { "kk",   "",             N_("Kilokelvins"),       1e3  }, NULL },
	{ { "Mk",   "",             N_("Megakelvins"),       1e6  }, NULL },
	{ { NULL,   NULL,           NULL,                    0    }, NULL }
};

/* Units of substance amount */
const AG_Unit agSubstanceAmountUnits[] = {
	{ "pmol", "",            N_("Picomoles"),  1e-12 },
	{ "umol", "\xc2\xb5mol", N_("Micromoles"), 1e-6  },
	{ "mmol", "",            N_("Millimoles"), 1e-3  },
	{ "mol",  "",            N_("Moles"),      1.0   },
	{ "kmol", "",            N_("Kilomoles"),  1e3   },
	{ "Mmol", "",            N_("Megamoles"),  1e6   },
	{ NULL,   NULL,          NULL,             0     }
};

/* Units of energy per substance amount */
const AG_Unit agEnergyPerSubstanceAmountUnits[] = {
	{ "J/mol",  "",   "Joules per mole",     1e-3 },
	{ "kJ/mol", "",   "Kilojoules per mole", 1.0  },
	{ "MJ/mol", "",   "Megajoules per mole", 1e3  },
	{ NULL,     NULL, NULL,                  0    }
};

/* Units of molar heat capacity */
const AG_Unit agMolarHeatCapacityUnits[] = {
	{ "J/mol.k",  "J/mol\xc2\xb7k",  "Joules per mole kelvin",	1.0 },
	{ "kJ/mol.k", "kJ/mol\xc2\xb7k", "Kilojoules per mole kelvin",	1e3 },
	{ "MJ/mol.k", "MJ/mol\xc2\xb7k", "Megajoules per mole kelvin",	1e6 },
	{ NULL,	NULL, NULL,						0   }
};

/* Units of light measurement */
const AG_Unit agLightUnits[] = {
	{ "ucd", "\xc2\xb5\x63\x64", N_("Microcandelas"),	1e-6 },
	{ "mcd", "",                 N_("Millicandelas"),	1e-3 },
	{ "cd",  "",                 N_("Candelas"),	 	1.0  },
	{ "kcd", "",                 N_("Kilocandelas"), 	1e3  },
	{ "Mcd", "",                 N_("Megacandelas"), 	1e6  },
	{ NULL,  NULL,               NULL,		 	0    }
};

/* Units of power */
const AG_Unit agPowerUnits[] = {
	{ "uW",     "\xc2\xb5W", N_("Microwatts"),       1e-6     },
	{ "mW",     "",          N_("Milliwatts"),       1e-3     },
	{ "BTU/h",  "",          N_("BTU/hr"),           0.292875 },
	{ "f-lb/s", "",          N_("Foot-lbs/sec"),     1.355818 },
	{ "W",      "",          N_("Watts"),            1.0      },
	{ "kC/m",   "",          N_("Kilocalories/min"), 69.733   },
	{ "HP",     "",          N_("Horsepower"),       746.0    },
	{ "kW",     "",          N_("Kilowatts"),        1e3      },
	{ "kC/s",   "",          N_("Kilocalories/sec"), 4183.98  },
	{ "MW",     "",          N_("Megawatts"),        1e6      },
	{ "GW",     "",          N_("Gigawatts"),        1e9      },
	{ NULL,     NULL,        NULL,                   0        }
};

/* Units of electromotive force */
const AG_Unit agEMFUnits[] = {
	{ "uV", "\xc2\xb5V", N_("Microvolts"),	1e-6 },
	{ "mV", "",          N_("Millivolts"),	1e-3 },
	{ "V",  "",          N_("Volts"),	1.0  },
	{ "kV", "",          N_("Kilovolts"),	1e3  },
	{ "MV", "",          N_("Megavolts"),	1e6  },
	{ NULL, NULL,        NULL,		0    }
};

/* Units of electrical resistance */
const AG_Unit agResistanceUnits[] = {
	{ "uohm", "\xc2\xb5\xce\xa9", N_("Micro-ohms"),	1e-6 },
	{ "mohm", "m\xce\xa9",        N_("Milliohms"),	1e-3 },
	{ "ohm",  "\xce\xa9",         N_("Ohms"),	1.0  },
	{ "kohm", "k\xce\xa9",        N_("Kilo-ohms"),	1e3  },
	{ "Mohm", "M\xce\xa9",        N_("Megaohms"),	1e6  },
	{ NULL,   NULL,               NULL,		0    }
};

/* Units of first order temperature coefficients of resistance. */
const AG_Unit agResistanceTC1Units[] = {
	{ "mohm/degC", "m\xce\xa9/\xc2\xb0\x43", N_("Milliohms per \xc2\xb0\x43"), 1e-3 },
	{ "ohms/degC", "\xce\xa9/\xc2\xb0\x43",  N_("Ohms per\xc2\xb0\x43"),       1.0  },
	{ NULL,        NULL,                     NULL,                             0    }
};

/* Units of second order temperature coefficients of resistance. */
const AG_Unit agResistanceTC2Units[] = {
	{ "mohm/degC^2", "m\xce\xa9/\xc2\xb0\x43\xc2\xb2", N_("Milliohms per \xc2\xb0\x43\xc2\xb2"), 1e-3 },
	{ "ohm/degC^2",  "\xce\xa9/\xc2\xb0\x43\xc2\xb2",  N_("Ohms per \xc2\xb0\x43\xc2\xb2"),      1.0  },
	{ NULL,          NULL,                             NULL,                                     0    }
};

/* Units of electrical capacitance */
const AG_Unit agCapacitanceUnits[] = {
	{ "pF", "",             N_("Picofarads"),	1e-12 },
	{ "nF", "",             N_("Nanofarads"),	1e-9  },
	{ "uF", "\xc2\xb5\x46",	N_("Microfarads"),	1e-6  },
	{ "mF", "",             N_("Millifarads"),	1e-3  },
	{ "F",  "",             N_("Farads"),		1.0   },
	{ "kF", "",             N_("Kilofarads"),	1e3   },
	{ NULL,	NULL,           NULL,			0     }
};

/* Units of electrical inductance */
const AG_Unit agInductanceUnits[] = {
	{ "uH", "\xc2\xb5\x48",	N_("Microhenries"),	1e-6 },
	{ "mH", "",             N_("Millihenries"),	1e-3 },
	{ "H",  "",             N_("Henries"),		1.0  },
	{ "kH", "",             N_("Kilohenries"),	1e3  },
	{ NULL, NULL,           NULL,			0    }
};

/* Units of frequency */
const AG_Unit agFrequencyUnits[] = {
	{ "uHz", "\xc2\xb5Hz", N_("Microhertz"),	1e-6 },
	{ "mHz", "",           N_("Millihertz"),	1e-3 },
	{ "Hz",  "",           N_("Hertz"),		1.0  },
	{ "kHz", "",           N_("Kilohertz"),		1e3  },
	{ "MHz", "",           N_("Megahertz"),		1e6  },
	{ "GHz", "",           N_("Gigahertz"),		1e9  },
	{ NULL,  NULL,         NULL,			0    }
};

/* Units of pressure and stress */
const AG_Unit agPressureUnits[] = {
	{ "Pa",        "",                N_("Pascals"),		1.0        },
	{ "mbar",      "",                N_("Millibars"),		1e2        },
	{ "kPa",       "",                N_("Kilopascals"),		1e3        },
	{ "bar",       "",                N_("Bars"),			1e5        },
	{ "MPa",       "",                N_("Megapascals"),		1e6        },
	{ "GPa",       "",                N_("Gigapascals"),		1e9        },
	{ "Kg-f/m^2",  "Kg-f/m\xc2\xb2",  N_("Kg-force per m\xc2\xb2"),	9.80665    },
	{ "cm H2O",    "cm H\xc2\xb2O",   N_("Centimeters of water"),	98.0665    },
	{ "in H2O",    "in H\xc2\xb2O",   N_("Inches of water"),	249.08891  },
	{ "cm Hg",     "",                N_("Centimeters of mercury"),	1333.22    },
	{ "Ft H2O",    "Ft H\xc2\xb2O",   N_("Feet of water"),   	2989.06692 },
	{ "in Hg",     "",                N_("Inches of mercury"),      3386.388   },
	{ "m H2O",     "m H\xc2\xb2O",    N_("Meters of water"),	9806.65    },
	{ "Atm",       "",                N_("Atmospheres"),		101325     },
	{ "Kips/in^2", "Kips/in\xc2\xb2", N_("Kips per in\xc2\xb2"),	6894760    },
	{ NULL,        NULL,              NULL,                         0          }
};

/* Units of vacuum pressure */
const AG_Unit agVacuumUnits[] = {
	{ "ubar",          "\xc2\xb5\x62\x61\x72",           N_("Microbar"),				0.000750062   },
	{ "_Pa",           "Pa",                             N_("Pascals"),				0.007500617   },
	{ "mtorr",         "",                               N_("Millitorr"),				0.001         },
	{ "micron Hg",     "",                               N_("Microns of Mercury"),			0.001         },
	{ "_mbar",         "mbar",                           N_("Millibar"),				0.7503001200  },
	{ "torr",          "",                               N_("Torr"),				1.0           },
	{ "mm Hg",         "",                               N_("Millimeters of Mercury"),		1.0           },
	{ "in H2O(-4C)",   "in H\xc2\xb2O (4\xc2\xb0\x43)",  N_("Inches of Water (4\xc2\xb0\x43)"),	1.868268641   },
	{ "in H2O(-60F)",  "in H\xc2\xb2O (60\xc2\xb0\x46)", N_("Inches of Water (60\xc2\xb0\x46)"),	1.866475993   },
	{ "in Hg(-32F)",   "in Hg (32\xc2\xb0\x46)",         N_("Inches of Mercury (32\xc2\xb0\x46)"),	25.399938811  },
	{ "in Hg(-60F)",   "in Hg (60\xc2\xb0\x46)",         N_("Inches of Mercury (60\xc2\xb0\x46)"),	25.328457932  },
	{ "_psi",          "psi",                            N_("Pounds per in\xc2\xb2"),               51.714932572  },
	{ "_at",           "at",                             N_("Atmospheres technical"),               735.559240069 },
	{ "_kg/cm^2",      "kg/cm\xc2\xb2",                  N_("Kilograms per cm\xc2\xb2"),            735.56454579  },
	{ "_bar",          "bar",                            N_("Bar"),                                 750.30012004  },
	{ "dynes/cm^2",    "dynes/cm\xc2\xb2",               N_("Dynes per cm\xc2\xb2"),                750.33012004  },
	{ "_Atm",          "Atm",                            N_("Standard atmospheres"),                760.0         },
	{ NULL,            NULL,                             NULL,                                      0.0           }
};

const AG_Unit agPercentageUnits[] = {
	{ "%",  "",    N_("Percent"), 1.0 },
	{ NULL, NULL,  NULL,          0.0 }
};

/* Units of resistivity in a material */
const AG_Unit agResistivityUnits[] = {
	{ "uohm/m", "\xc2\xb5\xce\xa9/m", N_("Micro-ohms per meter"), 1e-6 },
	{ "mohm/m", "m\xce\xa9/m",        N_("Milliohms per meter"),  1e-3 },
	{ "ohm/m",  "\xce\xa9/m",         N_("Ohms per meter"),       1.0  },
	{ "kohm/m", "k\xce\xa9/m",        N_("Kilo-ohms per meter"),  1e3  },
	{ "Mohm/m", "M\xce\xa9/m",        N_("Megaohms per meter"),   1e6  },
	{ NULL,      NULL,                NULL,                       0    }
};

/* Units of thermal conductivity of a material */
const AG_Unit agThermalConductivityUnits[] = {
	{ "W/m.k", "W/m\xc2\xb7k", N_("Watts per meter kelvin"), 1.0 },
	{ NULL,    NULL,           NULL,                         0   }
};

/* Units of thermal expansion of a material */
const AG_Unit agThermalExpansionUnits[] = {
	{ "um/m.k", "um/m\xc2\xb7k", N_("Micrometers per meter kelvin"), 1.0 },
	{ "mm/m.k", "mm/m\xc2\xb7k", N_("Millimeters per meter kelvin"), 1e3 },
	{ "m/m.k",  "m/m\xc2\xb7k",  N_("Meters per meter kelvin"),      1e6 },
	{ NULL,     NULL,            NULL,                               0   }
};

/* Units of density of a material */
const AG_Unit agDensityUnits[] = {
	{ "mg/cm^3", "mg/cm\xc2\xb3", N_("Milligrams per cubic centimeter"), 1e-3 },
	{ "g/cm^3",  "g/cm\xc2\xb3",  N_("Grams per cubic centimeter"),      1.0  },
	{ "kg/cm^3", "kg/cm\xc2\xb3", N_("Kilograms per cubic centimeters"), 1e3  },
	{ NULL, NULL, NULL,                                                  0    }
};

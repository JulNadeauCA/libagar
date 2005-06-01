/*	$Csoft: units.h,v 1.13 2004/08/26 06:18:20 vedge Exp $	*/
/*	Public domain	*/

#include <config/historical_units.h>

#ifndef _AGAR_WIDGET_UNITS_H_
#define _AGAR_WIDGET_UNITS_H_
#include "begin_code.h"

struct unit {
	char	 *key;		/* Key */
	char	 *abbr;		/* Symbol */
	char	 *name;		/* Long name */
	double	  divider;	/* Base unit divider (for linear conv) */
	double	(*func)(double, int);	/* For nonlinear conversions */
};

__BEGIN_DECLS
const struct unit *unit_find(const char *);
__inline__ double  unit2base(double, const struct unit *);
double		   base2unit(double, const struct unit *);
__inline__ double  unit2unit(double, const struct unit *, const struct unit *);

const struct unit *unit_best(const struct unit[], double);
const char	  *unit_abbr(const struct unit *);
__inline__ int	   unit_format(double, const struct unit[], char *, size_t);

double	unit_fahrenheit(double, int);
double	unit_celsius(double, int);
#ifdef HISTORICAL_UNITS
double	unit_rankine(double, int);
double	unit_reaumur(double, int);
#endif

#define	unit2basef(n, u) ((float)unit2base((float)(n), (u)))
#define	base2unitf(n, u) ((float)base2unit((float)(n), (u)))
#define	unit2unitf(n, u1, u2) ((float)unit2unit((float)(n), (u1), (u2)))

extern const struct unit *unit_groups[];
extern const int nunit_groups;

extern const struct unit identity_unit[];
extern const struct unit length_units[];
extern const struct unit video_units[];
extern const struct unit area_units[];
extern const struct unit volume_units[];
extern const struct unit speed_units[];
extern const struct unit mass_units[];
extern const struct unit time_units[];
extern const struct unit current_units[];
extern const struct unit temperature_units[];
extern const struct unit substance_amt_units[];
extern const struct unit light_units[];
extern const struct unit power_units[];
extern const struct unit emf_units[];
extern const struct unit resistance_units[];
extern const struct unit resistance_Tcoeff1_units[];
extern const struct unit resistance_Tcoeff2_units[];
extern const struct unit capacitance_units[];
extern const struct unit inductance_units[];
extern const struct unit frequency_units[];
extern const struct unit pressure_units[];
extern const struct unit met_units[];
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_UNITS_H_ */

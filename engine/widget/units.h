/*	$Csoft: units.h,v 1.7 2004/02/24 12:24:50 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_UNITS_H_
#define _AGAR_WIDGET_UNITS_H_
#include "begin_code.h"

struct unit {
	char	 *abbr;		/* Abbreviation/symbol */
	char	 *name;		/* Long name */
	double	  divider;	/* Base unit divider (for linear conv) */
	double	(*func)(double); /* Function (for nonlinear conv) */
};

__BEGIN_DECLS
__inline__ const struct unit *unit(const struct unit[], const char *);
__inline__ double unit2base(double, const struct unit *);
#define		  unit2basef(n, u) ((float)unit2base((float)(n), (u)))
__inline__ double base2unit(double, const struct unit *);
#define		  base2unitf(n, u) ((float)base2unit((float)(n), (u)))

extern const struct unit identity_unit;
extern const struct unit length_units[];
extern const struct unit area_units[];
extern const struct unit volume_units[];
extern const struct unit speed_units[];
extern const struct unit mass_units[];
extern const struct unit time_units[];
extern const struct unit current_units[];
extern const struct unit temperature_units[];
extern const struct unit substance_amount_units[];
extern const struct unit light_units[];
extern const struct unit power_units[];
extern const struct unit emf_units[];
extern const struct unit resistance_units[];
extern const struct unit capacitance_units[];
extern const struct unit inductance_units[];
extern const struct unit frequency_units[];
extern const struct unit pressure_units[];
extern const struct unit metabolic_expenditure_units[];
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_UNITS_H_ */

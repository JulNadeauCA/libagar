/*	$Csoft: units.h,v 1.1 2003/11/15 02:03:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_UNITS_H_
#define _AGAR_WIDGET_UNITS_H_
#include "begin_code.h"

struct unit {
	char	*abbr;		/* Abbreviation */
	char	*name;		/* Long name */
	double	 divider;	/* Divider (with respect to the base unit) */
};

__BEGIN_DECLS
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
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_UNITS_H_ */

/*	$Csoft: graph.h,v 1.17 2003/09/07 07:58:37 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_MGRAPH_H_
#define _AGAR_WIDGET_MGRAPH_H_

#include <engine/widget/widget.h>
#include <engine/widget/label.h>

#include "begin_code.h"

struct mgraph_function {
	char name[LABEL_MAX];			/* Identifier */
	double (*func)(double x);		/* Function */
	Uint32 color;				/* Trace color */
	double *points;				/* Calculated points */
	Uint32 npoints;
	TAILQ_ENTRY(mgraph_function) functions;
};

enum mgraph_style {
	MGRAPH_POINTS,			/* Trace individual points */
	MGRAPH_LINES			/* Trace lines */
};

TAILQ_HEAD(mgraph_functionq, mgraph_function);

struct mgraph {
	struct widget wid;
	char name[LABEL_MAX];
	enum mgraph_style style;
	struct mgraph_functionq functions;
};

__BEGIN_DECLS
struct mgraph *mgraph_new(void *, enum mgraph_style, const char *, ...)
	           FORMAT_ATTRIBUTE(printf, 3, 4)
		   NONNULL_ATTRIBUTE(3);

void	 mgraph_init(struct mgraph *, enum mgraph_style, const char *);
void	 mgraph_destroy(void *);
void	 mgraph_draw(void *);
void	 mgraph_scale(void *, int, int);

struct mgraph_function	*mgraph_add_function(struct mgraph *,
			                     double (*)(double),
					     Uint32, Uint8, Uint8, Uint8,
					     const char *, ...)
			     FORMAT_ATTRIBUTE(printf, 7, 8)
			     NONNULL_ATTRIBUTE(7);

void			 mgraph_function_set_color(struct mgraph_function *,
			                           Uint8, Uint8, Uint8);

void			 mgraph_remove_function(struct mgraph *,
			                        struct mgraph_function *);
void	 		 mgraph_free_functions(struct mgraph *);
void			 mgraph_compute_function(struct mgraph *,
			                         struct mgraph_function *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_MGRAPH_H_ */

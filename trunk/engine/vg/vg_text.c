/*	$Csoft: vg_text.c,v 1.21 2005/06/30 06:26:23 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
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

#ifdef EDITION
#include <engine/map/mapview.h>
#include <engine/map/tool.h>

#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/combo.h>
#endif

#include "vg.h"
#include "vg_text.h"

#include <stdarg.h>

static void
init(struct vg *vg, struct vg_element *vge)
{
	vge->vg_text.su = NULL;
	vge->vg_text.text[0] = '\0';
	vge->vg_text.angle = 0;
	vge->vg_text.align = VG_ALIGN_MC;
	vge->vg_text.nptrs = 0;
}

static void
destroy(struct vg *vg, struct vg_element *vge)
{
	if (vge->vg_text.su != NULL)
		SDL_FreeSurface(vge->vg_text.su);
}

/* Specify the text alignment around the central vertex. */
void
vg_text_align(struct vg *vg, enum vg_alignment align)
{
	vg->cur_vge->vg_text.align = align;
}

/* Specify the angle relative to the central vertex. */
void
vg_text_angle(struct vg *vg, double angle)
{
	vg->cur_vge->vg_text.angle = angle;
}

/* Specify text with polled values. */
void
vg_pprintf(struct vg *vg, const char *fmt, ...)
{
	struct vg_element *vge = vg->cur_vge;
	const char *p;
	va_list ap;
	
	if (fmt != NULL) {
		strlcpy(vge->vg_text.text, fmt, sizeof(vge->vg_text.text));
	} else {
		vge->vg_text.text[0] = '\0';
		return;
	}

	va_start(ap, fmt);
	for (p = &fmt[0]; *p != '\0'; p++) {
		if (*p == '%' && *(p+1) != '\0') {
			switch (*(p+1)) {
			case ' ':
			case '(':
			case ')':
			case '%':
				break;
			default:
				if (vge->vg_text.nptrs >= VG_TEXT_MAX_PTRS) {
					break;
				}
				vge->vg_text.ptrs[vge->vg_text.nptrs++] =
				    va_arg(ap, void *);
				break;
			}
		}
	}
	va_end(ap);
	
	if (vge->vg_text.su != NULL) {
		SDL_FreeSurface(vge->vg_text.su);
		vge->vg_text.su = NULL;
	}
}

/* Specify static text. */
void
vg_printf(struct vg *vg, const char *fmt, ...)
{
	struct vg_element *vge = vg->cur_vge;
	va_list args;

	if (fmt != NULL) {
		va_start(args, fmt);
		vsnprintf(vge->vg_text.text, sizeof(vge->vg_text.text), fmt,
		    args);
		va_end(args);
	} else {
		vge->vg_text.text[0] = '\0';
	}
	if (vge->vg_text.su != NULL) {
		SDL_FreeSurface(vge->vg_text.su);
		vge->vg_text.su = NULL;
	}
	vge->redraw++;
	vg->redraw++;
}

#define TEXT_ARG(_type) (*(_type *)vge->vg_text.ptrs[ri])

static void
omega(char *s, size_t len, int n)
{
	strlcat(s, "omega", len);
}

static const struct {
	char	 *fmt;
	size_t	  fmt_len;
	void	(*func)(char *, size_t, int);
} fmts[] = {
	{ "omega", sizeof("omega"), omega },
};
static const int nfmts = sizeof(fmts) / sizeof(fmts[0]);

static void
align_text(struct vg *vg, struct vg_element *vge, Sint16 *x, Sint16 *y)
{
	SDL_Surface *su = vge->vg_text.su;
	int vx, vy;
	
	vg_rcoords2(vg, vge->vtx[0].x, vge->vtx[0].y, &vx, &vy);

	switch (vge->vg_text.align) {
	case VG_ALIGN_TL:
		*x = (Sint16)vx;
		*y = (Sint16)vy;
		break;
	case VG_ALIGN_TC:
		*x = (Sint16)vx - su->w/2;
		*y = (Sint16)vy;
		break;
	case VG_ALIGN_TR:
		*x = (Sint16)vx - su->w;
		*y = (Sint16)vy;
		break;
	case VG_ALIGN_ML:
		*x = (Sint16)vx;
		*y = (Sint16)vy - su->h/2;
		break;
	case VG_ALIGN_MC:
		*x = (Sint16)vx - su->w/2;
		*y = (Sint16)vy - su->h/2;
		break;
	case VG_ALIGN_MR:
		*x = (Sint16)vx - su->w;
		*y = (Sint16)vy - su->h/2;
		break;
	case VG_ALIGN_BL:
		*x = (Sint16)vx;
		*y = (Sint16)vy - su->h;
		break;
	case VG_ALIGN_BC:
		*x = (Sint16)vx - su->w/2;
		*y = (Sint16)vy - su->h;
		break;
	case VG_ALIGN_BR:
		*x = (Sint16)vx - su->w;
		*y = (Sint16)vy - su->h;
		break;
	default:
		break;
	}
}

static void
render_label(struct vg *vg, struct vg_element *vge)
{
	char s[VG_TEXT_MAX];
	char s2[32];
	SDL_Surface *ts;
	char *fmtp;
	int i, ri = 0;

	if (vge->vg_text.nptrs == 0) {
		if (vge->vg_text.su == NULL) {
			vge->vg_text.su = text_render(
			    vge->text_st.face[0] != '\0' ? vge->text_st.face :
			    NULL, vge->text_st.size, vge->color,
			    vge->vg_text.text);
		}
		return;
	}

	s[0] = '\0';
	s2[0] = '\0';

	for (fmtp = vge->vg_text.text; *fmtp != '\0'; fmtp++) {
		if (*fmtp == '%' && *(fmtp+1) != '\0') {
			switch (*(fmtp+1)) {
			case 'd':
			case 'i':
				snprintf(s2, sizeof(s2), "%d", TEXT_ARG(int));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'o':
				snprintf(s2, sizeof(s2), "%o", TEXT_ARG(u_int));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'u':
				snprintf(s2, sizeof(s2), "%u", TEXT_ARG(u_int));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'x':
				snprintf(s2, sizeof(s2), "%x", TEXT_ARG(u_int));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'X':
				snprintf(s2, sizeof(s2), "%X", TEXT_ARG(u_int));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'c':
				s2[0] = TEXT_ARG(char);
				s2[1] = '\0';
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 's':
				strlcat(s, &TEXT_ARG(char), sizeof(s));
				ri++;
				break;
			case 'p':
				snprintf(s2, sizeof(s2), "%p",
				    TEXT_ARG(void *));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'f':
				snprintf(s2, sizeof(s2), "%.2f",
				    TEXT_ARG(double));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case '[':
				for (i = 0; i < nfmts; i++) {
					if (strncmp(fmts[i].fmt, fmtp+2,
					    fmts[i].fmt_len-1) != 0) {
						continue;
					}
					fmtp += fmts[i].fmt_len;
					fmts[i].func(s2, sizeof(s2), ri);
					strlcat(s, s2, sizeof(s));
					ri++;
					break;
				}
				break;
			case '%':
				s2[0] = '%';
				s2[1] = '\0';
				strlcat(s, s2, sizeof(s));
				break;
			}
			fmtp++;
		} else {
			s2[0] = *fmtp;
			s2[1] = '\0';
			strlcat(s, s2, sizeof(s));
		}
	}
	if (vge->vg_text.su != NULL) {
		SDL_FreeSurface(vge->vg_text.su);
	}
	vge->vg_text.su = text_render(NULL, -1, vge->color, s);
}

static void
render(struct vg *vg, struct vg_element *vge)
{
	SDL_Rect rd;
	
	render_label(vg, vge);
	align_text(vg, vge, &rd.x, &rd.y);
	SDL_BlitSurface(vge->vg_text.su, NULL, vg->su, &rd);
}

static void
extent(struct vg *vg, struct vg_element *vge, struct vg_rect *r)
{
	Sint16 rx, ry;

	render_label(vg, vge);
	align_text(vg, vge, &rx, &ry);
	r->x = VG_VECXF(vg,rx);
	r->y = VG_VECYF(vg,ry);
	r->w = VG_VECLENF(vg,vge->vg_text.su->w);
	r->h = VG_VECLENF(vg,vge->vg_text.su->h);
}

static float
intsect(struct vg *vg, struct vg_element *vge, double x, double y)
{
	if (vge->nvtx < 1)
		return (FLT_MAX);

	if (vge->vg_text.su != NULL) {
		return (x - vge->vtx[0].x) + (y - vge->vtx[0].y);
	} else {
		return (FLT_MAX);
	}
}

const struct vg_element_ops vg_text_ops = {
	N_("Text string"),
	VGTEXT_ICON,
	init,
	destroy,
	render,
	extent,
	intsect
};

#ifdef EDITION
static struct vg_element *cur_text;
static struct vg_vertex *cur_vtx;
static enum vg_alignment cur_align;
static char cur_string[1024];

static void
select_alignment(int argc, union evarg *argv)
{
	struct combo *com = argv[0].p;
	struct tool *t = argv[1].p;
	struct tlist_item *it = argv[2].p;
	char *align = (char *)it->p1;

	switch (align[0]) {
	case 'T':
		switch (align[1]) {
		case 'L':
			cur_align = VG_ALIGN_TL;
			break;
		case 'C':
			cur_align = VG_ALIGN_TC;
			break;
		case 'R':
			cur_align = VG_ALIGN_TR;
			break;
		}
		break;
	case 'M':
		switch (align[1]) {
		case 'L':
			cur_align = VG_ALIGN_ML;
			break;
		case 'C':
			cur_align = VG_ALIGN_MC;
			break;
		case 'R':
			cur_align = VG_ALIGN_MR;
			break;
		}
		break;
	case 'B':
		switch (align[1]) {
		case 'L':
			cur_align = VG_ALIGN_BL;
			break;
		case 'C':
			cur_align = VG_ALIGN_BC;
			break;
		case 'R':
			cur_align = VG_ALIGN_BR;
			break;
		}
		break;
	}
}

static void
string_changed(int argc, union evarg *argv)
{
	struct textbox *tb = argv[0].p;
	struct tool *t = argv[1].p;
	struct vg *vg = t->p;
	struct widget_binding *stringb;
	char *s;
	
	stringb = widget_get_binding(tb, "string", &s);
	if (cur_text != NULL) {
		vg_select_element(vg, cur_text);
		vg_printf(vg, "%s", s);
	}
	widget_binding_unlock(stringb);
}

static void
text_tool_init(void *t)
{
	cur_text = NULL;
	cur_vtx = NULL;
	cur_align = VG_ALIGN_TL;
	cur_string[0] = '\0';
}

static void
text_tool_pane(void *t, void *con)
{
	struct combo *com;
	struct textbox *tb;
	struct tlist_item *it;
	
	tb = textbox_new(con, _("Text: "));
	widget_bind(tb, "string", WIDGET_STRING, cur_string,
	    sizeof(cur_string));
	event_new(tb, "textbox-postchg", string_changed, "%p", t);

	com = combo_new(con, 0, _("Alignment: "));
	it = tlist_insert_item(com->list, ICON(VGTL_ICON), _("Top/left"), "TL");
	tlist_insert_item(com->list, ICON(VGTC_ICON), _("Top/center"), "TC");
	tlist_insert_item(com->list, ICON(VGTR_ICON), _("Top/right"), "TR");
	tlist_insert_item(com->list, ICON(VGML_ICON), _("Middle/left"), "ML");
	tlist_insert_item(com->list, ICON(VGMC_ICON), _("Middle/center"), "MC");
	tlist_insert_item(com->list, ICON(VGMR_ICON), _("Middle/right"), "MR");
	tlist_insert_item(com->list, ICON(VGBL_ICON), _("Bottom/left"), "BL");
	tlist_insert_item(com->list, ICON(VGBC_ICON), _("Bottom/center"), "BC");
	tlist_insert_item(com->list, ICON(VGBR_ICON), _("Bottom/right"), "BR");
	event_new(com, "combo-selected", select_alignment, "%p", t);
	combo_select(com, it);
}

static int
text_mousemotion(void *t, int xmap, int ymap, int xrel, int yrel, int btn)
{
	struct vg *vg = TOOL(t)->p;
	double x, y;
	
	vg_map2vec(vg, xmap, ymap, &x, &y);
	if (cur_vtx != NULL) {
		cur_vtx->x = x;
		cur_vtx->y = y;
	} else {
		vg->origin[2].x = x;
		vg->origin[2].y = y;
	}
	if (cur_text != NULL) {
		cur_text->redraw++;
		vg->redraw++;
	}
	vg->origin[1].x = x;
	vg->origin[1].y = y;
	return (1);
}

static int
text_mousebuttondown(void *t, int xmap, int ymap, int btn)
{
	struct vg *vg = TOOL(t)->p;
	double vx, vy;

	switch (btn) {
	case 1:
		cur_text = vg_begin_element(vg, VG_TEXT);
		vg_printf(vg, "%s", cur_string);
		vg_map2vec(vg, xmap, ymap, &vx, &vy);
		cur_vtx = vg_vertex2(vg, vx, vy);
		vg_text_align(vg, cur_align);
		vg_end_element(vg);

		cur_text->redraw++;
		break;
	default:
		if (cur_text != NULL) {
			vg_destroy_element(vg, cur_text);
		}
		goto finish;
	}
	return (1);
finish:
	cur_text = NULL;
	cur_vtx = NULL;
	return (1);
}

const struct tool_ops vg_text_tool = {
	"Text", N_("Insert text labels."),
	VGTEXT_ICON,
	sizeof(struct tool),
	0,
	text_tool_init,
	NULL,			/* destroy */
	NULL,			/* pane */
	NULL,			/* edit */
	NULL,			/* cursor */
	NULL,			/* effect */

	text_mousemotion,
	text_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

/*	Public domain	*/

#define Uint8 uint8_t
#define Sint8 int8_t
#define Uint16 uint16_t
#define Sint16 int16_t
#define Uint32 uint32_t
#define Sint32 int32_t
#define Uint unsigned
#define Ulong unsigned long

#include <stdio.h>
#include <perlio.h>

#ifndef PERL_REVISION
# ifndef __PATCHLEVEL_H_INCLUDED__
#  include "patchlevel.h"
# endif
# ifndef PERL_REVISION
#  define PERL_REVISION   (5)
#  define PERL_VERSION    PATCHLEVEL
#  define PERL_SUBVERSION SUBVERSION
# endif
#endif
#if (PERL_VERSION == 5) && (PERL_SUBVERSION == 3)
# ifndef PL_na
# define PL_na na
# endif
# ifndef SvPV_nolen
# define SvPV_nolen(sv) SvPV(sv, PL_na)
# endif
#endif
#ifndef PERL_UNUSED_ARG
#define PERL_UNUSED_ARG(x) ((void)sizeof(x))
#endif
#ifndef NVTYPE
typedef double NV;
#endif
#ifndef aTHX_
#define aTHX_
#endif
#ifndef pTHX_
#define pTHX_
#endif
#ifndef mPUSHp
#define mPUSHp(p,l)	PUSHs(sv_2mortal(newSVpvn((p), (l))))
#endif
#ifndef mPUSHi
#define mPUSHi(i)	PUSHs(sv_2mortal(newSViv((i))))
#endif
#ifndef mPUSHn
#define mPUSHn(n)	PUSHs(sv_2mortal(newSVnv((n))))
#endif

/* How many event args belong to perl? */
#define AP_ARGS_MAX 1

/* Mapping of a Agar option flag to a string. */
typedef struct ap_flag_names {
	const char *name;
	Uint bitmask;
} AP_FlagNames;

extern void AP_MapHashToFlags(void *, const AP_FlagNames *, Uint *);
extern int  AP_SetNamedFlag(const char *, const AP_FlagNames *, Uint *);
extern int  AP_UnsetNamedFlag(const char *, const AP_FlagNames *, Uint *);
extern int  AP_GetNamedFlag(const char *, const AP_FlagNames *, Uint, Uint *);

/* Agar-Core */
#include "perl_event.h"
#include "perl_object.h"
#include "perl_config.h"

/* Agar-GUI base */
#include "perl_surface.h"
#include "perl_font.h"
#include "perl_widget.h"
#include "perl_window.h"

/* Agar-GUI widgets */
#include "perl_box.h"
#include "perl_button.h"
#include "perl_checkbox.h"
#include "perl_combo.h"
#include "perl_console.h"
#include "perl_editable.h"
#include "perl_filedlg.h"
#include "perl_fixed.h"
#include "perl_fixedplotter.h"
#include "perl_graph.h"
#include "perl_hsvpal.h"
#include "perl_icon.h"
#include "perl_label.h"
#include "perl_menu.h"
#include "perl_mpane.h"
#include "perl_notebook.h"
#include "perl_numerical.h"
#include "perl_pane.h"
#include "perl_pixmap.h"
#include "perl_progressbar.h"
#include "perl_radio.h"
#include "perl_scrollbar.h"
#include "perl_scrollview.h"
#include "perl_separator.h"
#include "perl_slider.h"
#include "perl_socket.h"
#include "perl_statusbar.h"
#include "perl_table.h"
#include "perl_textbox.h"
#include "perl_tlist.h"
#include "perl_toolbar.h"
#include "perl_ucombo.h"


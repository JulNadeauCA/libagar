/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2004 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/*
 * This file reverses the effects of begin.h and should be included after you
 * finish any function and structure declarations in your headers.
 */

#undef _AGAR_BEGIN_H_

/* Reset structure packing at previous byte alignment. */
#if defined(_MSC_VER) || defined(__MWERKS__) || defined(__WATCOMC__) || \
    defined(__BORLANDC__)
# ifdef __BORLANDC__
#  pragma nopackwarning
# endif
# if (defined(__MWERKS__) && defined(__MACOS__))
#  pragma options align=reset
#  pragma enumsalwaysint reset
# else
#  pragma pack(pop)
# endif
#endif

/* Undo begin.h definitions. */
#ifdef _AGAR_DEFINED_DECLSPEC
# undef _AGAR_DEFINED_DECLSPEC
# undef DECLSPEC
#endif
#ifdef _AGAR_DEFINED_AGARCALL
# undef _AGAR_DEFINED_AGARCALL
# undef AGARCALL
#endif
#ifdef _AGAR_DEFINED_NULL
# undef _AGAR_DEFINED_NULL
# undef NULL
#endif

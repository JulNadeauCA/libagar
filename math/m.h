/*	Public domain	*/

#ifndef _AGAR_MATH_M_H_
#define _AGAR_MATH_M_H_

#include <agar/math/m_begin.h>

#define spREAL M_Real

#include <agar/math/m_math.h>
#include <agar/math/m_int_vector.h>
#include <agar/math/m_complex.h>
#include <agar/math/m_vector.h>
#include <agar/math/m_matrix.h>
#include <agar/math/m_quaternion.h>
#include <agar/math/m_coordinates.h>
#include <agar/math/m_color.h>
#include <agar/math/m_geometry.h>
#include <agar/math/m_gaussj.h>
#include <agar/math/m_lu.h>

#ifdef _M_INTERNAL
# undef _
# undef N_
# define _(s) (s)
# define N_(s) (s)
#endif

#include <agar/math/m_close.h>
#endif /* _AGAR_MATH_M_H_ */

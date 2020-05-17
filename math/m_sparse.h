/*
 *  Revision and copyright information.
 *
 *  Copyright (c) 1985-2003 by Kenneth S. Kundert
 *
 *  $Date: 2003/06/29 04:19:52 $
 *  $Revision: 1.2 $
 *
 * Sparse1.4 is distributed as open-source software under the Berkeley
 * license model. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the following
 * conditions are met:
 * 
 * Redistributions of source code must retain the original copyright notice,
 * this list of conditions and the following disclaimer.  Redistributions
 * in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.  Neither the name of
 * the copyright holder nor the names of the authors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * 
 * This software is provided by the copyright holders and contributors
 * ``as is'' and any express or implied warranties, including, but not
 * limited to, the implied warranties of merchantability and fitness for
 * a particular purpose are disclaimed. In no event shall the copyright
 * owner or contributors be liable for any direct, indirect, incidental,
 * special, exemplary, or consequential damages (including, but not
 * limited to, procurement of substitute goods or services; loss of use,
 * data, or profits; or business interruption) however caused and on any
 * theory of liability, whether in contract, strict liability, or tort
 * (including negligence or otherwise) arising in any way out of the use
 * of this software, even if advised of the possibility of such damage.
 */

#ifndef _AGAR_INTERNAL
#error "m_sparse.h is only for internal backend usage"
#endif

#include <agar/math/begin.h>

/*
 * Configuration Settings
 */

/* Make the matrix expandable before it has been factored. */
#define  EXPANDABLE                     YES

/* Allow the set of external row and column numbers to be non-packed. */
#define  TRANSLATE                      YES

/*
 * For node- and modified-node admittance matrices, try to select pivots
 * from the diagonal when possible.
 */
#define  DIAGONAL_PIVOTING              YES

/* Use modified Markowitz method of pivot selection. */
#define  MODIFIED_MARKOWITZ             NO

/*
 * The routines that allow four related elements to be entered into the
 * matrix at once should be compiled.
 */
#define  QUAD_ELEMENT                   YES

/*
 * Routines that are used to document the matrix, such as spPrint() and
 * spFileMatrix(), should be compiled.
 */
#define  DOCUMENTATION                  YES

/*
 * This specifies that additional error checking will be compiled.
 * The type of error checked are those that are common when the
 * matrix routines are first integrated into a user's program.  Once
 * the routines have been integrated in and are running smoothly, this
 * option should be turned off. YES is recommended.
 */
#define  spDEBUG                          YES

/*
 *  The following options affect Sparse exports and so are exported as a
 *  side effect.  For this reason they use the `sp' prefix.  The boolean
 *  constants YES an NO are not defined in spMatrix.h to avoid conflicts
 *  with user code, so use 0 for NO and 1 for YES.
 */

/*
 *  MATRIX CONSTANTS
 *
 *  These constants are used throughout the sparse matrix routines.  They
 *  should be set to suit the type of matrix being solved.
 *  XXX
 */

/*
 * The relative threshold used if the user enters an invalid
 * threshold.  Also the threshold used by spFactor() when
 * calling spOrderAndFactor().  The default threshold should
 * not be less than or equal to zero nor larger than one.
 * 0.001 is recommended.
 */
#define  DEFAULT_THRESHOLD              1.0e-3

/*
 * This indicates whether spOrderAndFactor() should use diagonal
 * pivoting as default.  This issue only arises when
 * spOrderAndFactor() is called from spFactor(). YES is recommended.
 */
#define  DIAG_PIVOTING_AS_DEFAULT       YES

/*
 * This number multiplied by the size of the matrix equals the number
 * of elements for which memory is initially allocated in spCreate().
 * 6 is recommended.
 */
#define  SPACE_FOR_ELEMENTS             6

/*
 * This number multiplied by the size of the matrix equals the number
 * of elements for which memory is initially allocated and specifically
 * reserved for fill-ins in spCreate(). 4 is recommended.
 */
#define  SPACE_FOR_FILL_INS             4

/*
 * The number of matrix elements requested from the malloc utility on
 * each call to it.  Setting this value greater than 1 reduces the
 * amount of overhead spent in this system call. On a virtual memory
 * machine, its good to allocate slightly less than a page worth of
 * elements at a time (or some multiple thereof).
 * 31 is recommended.
 */
#define  ELEMENTS_PER_ALLOCATION        31

/*
 * The minimum allocated size of a matrix.  Note that this does not
 * limit the minimum size of a matrix.  This just prevents having to
 * resize a matrix many times if the matrix is expandable, large and
 * allocated with an estimated size of zero.  This number should not
 * be less than one.
 */
#define  MINIMUM_ALLOCATED_SIZE         6

/*
 * The amount the allocated size of the matrix is increased when it
 * is expanded.
 */
#define  EXPANSION_FACTOR               1.5

/*
 * Some terminology should be defined.  The Markowitz row count is the number
 * of non-zero elements in a row excluding the one being considered as pivot.
 * There is one Markowitz row count for every row.  The Markowitz column
 * is defined similarly for columns.  The Markowitz product for an element
 * is the product of its row and column counts. It is a measure of how much
 * work would be required on the next step of the factorization if that
 * element were chosen to be pivot.  A small Markowitz product is desirable.
 *
 * This number is used for two slightly different things, both of which
 * relate to the search for the best pivot.  First, it is the maximum
 * number of elements that are Markowitz tied that will be sifted
 * through when trying to find the one that is numerically the best.
 * Second, it creates an upper bound on how large a Markowitz product
 * can be before it eliminates the possibility of early termination
 * of the pivot search.  In other words, if the product of the smallest
 * Markowitz product yet found and TIES_MULTIPLIER is greater than
 * MAX_MARKOWITZ_TIES, then no early termination takes place.
 * Set MAX_MARKOWITZ_TIES to some small value if no early termination of
 * the pivot search is desired. An array of RealNumbers is allocated
 * of size MAX_MARKOWITZ_TIES so it must be positive and shouldn't
 * be too large.  Active when MODIFIED_MARKOWITZ is 1 (YES).
 * 100 is recommended.
 * \see TIES_MULTIPLIER
 */
#define  MAX_MARKOWITZ_TIES             100

/*
 * Specifies the number of Markowitz ties that are allowed to occur
 * before the search for the pivot is terminated early.  Set to some
 * large value if no early termination of the pivot search is desired.
 * This number is multiplied times the Markowitz product to determine
 * how many ties are required for early termination.  This means that
 * more elements will be searched before early termination if a large
 * number of fill-ins could be created by accepting what is currently
 * considered the best choice for the pivot.  Active when
 * MODIFIED_MARKOWITZ is 1 (YES).  Setting this number to zero
 * effectively eliminates all pivoting, which should be avoided.
 * This number must be positive.  TIES_MULTIPLIER is also used when
 * diagonal pivoting breaks down. 5 is recommended.
 * \see MAX_MARKOWITZ_TIES
 */
#define  TIES_MULTIPLIER                5

/*
 * Which partition mode is used by spPartition() as default.
 * Possibilities include spDIRECT_PARTITION (each row used direct
 * addressing, best for a few relatively dense matrices),
 * spINDIRECT_PARTITION (each row used indirect addressing, best
 * for a few very sparse matrices), and spAUTO_PARTITION (direct or
 * indirect addressing is chosen on a row-by-row basis, carries a large
 * overhead, but speeds up both dense and sparse matrices, best if there
 * is a large number of matrices that can use the same ordering.
 */
#define  DEFAULT_PARTITION              spAUTO_PARTITION

/*
 * The number of characters per page width.  Set to 80 for terminal,
 * 132 for line printer. Controls how many columns printed by
 * spPrint() per page width.
 */
#define  PRINTER_WIDTH  80

/*
 *  MACHINE CONSTANTS
 *
 *  These numbers must be updated when the program is ported to a new machine.
 */

/* The largest possible value of shorts. */
#define  LARGEST_SHORT_INTEGER   (0xffff-1)

/* The largest possible value of longs. */
#define  LARGEST_LONG_INTEGER    AG_LONG_MAX

/* Change annotation produced by the matrix routines. */
/* #define ANNOTATE_FULL */

/*
 *  ERROR KEYWORDS
 *
 *  The actual numbers used in the error codes are not sacred, they can be
 *  changed under the condition that the codes for the nonfatal errors are
 *  less than the code for spFATAL and similarly the codes for the fatal
 *  errors are greater than that for spFATAL.
 */

#define  spOKAY		0
#define  spSMALL_PIVOT	1
#define  spZERO_DIAG	2
#define  spSINGULAR	3 
#define  spMANGLED	4 
#define  spNO_MEMORY	5
#define  spPANIC	6 
#define  spFATAL	2

/*
 *  KEYWORD DEFINITIONS
 */

#define  spREAL	M_Real

/*
 *  PARTITION TYPES
 *
 *  When factoring a previously ordered matrix using spFactor(), Sparse
 *  operates on a row-at-a-time basis.  For speed, on each step, the row
 *  being updated is copied into a full vector and the operations are
 *  performed on that vector.  This can be done one of two ways, either
 *  using direct addressing or indirect addressing.  Direct addressing
 *  is fastest when the matrix is relatively dense and indirect addressing
 *  is quite sparse.  The user can select which partitioning mode is used.
 *  The following keywords are passed to spPartition() and indicate that
 *  Sparse should use only direct addressing, only indirect addressing, or
 *  that it should choose the best mode on a row-by-row basis.  The time
 *  required to choose a partition is of the same order of the cost to factor
 *  the matrix.
 *
 *  If you plan to factor a large number of matrices with the same structure,
 *  it is best to let Sparse choose the partition.  Otherwise, you should
 *  choose the partition based on the predicted density of the matrix.
 */

#define spDEFAULT_PARTITION	0
#define spDIRECT_PARTITION	1
#define spINDIRECT_PARTITION	2
#define spAUTO_PARTITION	3

/*
 *  MACRO FUNCTION DEFINITIONS
 */

/* Begin Macros. */
/*
 * Macro function that adds data to a real element in the matrix by a pointer.
 */
#define  spADD_REAL_ELEMENT(element,real)       *(element) += real

/*
 * Macro function that adds data to a imaginary element in the matrix by
 * a pointer.
 */
#define  spADD_IMAG_ELEMENT(element,imag)       *(element+1) += imag

/*
 * Macro function that adds data to a complex element in the matrix by
 * a pointer.
 */
#define  spADD_COMPLEX_ELEMENT(element,real,imag)       \
{   *(element) += real;                                 \
    *(element+1) += imag;                               \
}

/*
 * Macro function that adds data to each of the four real matrix elements
 * specified by the given template.
 */
#define  spADD_REAL_QUAD(template,real)         \
{   *((template).Element1) += real;             \
    *((template).Element2) += real;             \
    *((template).Element3Negated) -= real;      \
    *((template).Element4Negated) -= real;      \
}

/*
 * Macro function that adds data to each of the four imaginary matrix
 * elements specified by the given template.
 */
#define  spADD_IMAG_QUAD(template,imag)         \
{   *((template).Element1+1) += imag;           \
    *((template).Element2+1) += imag;           \
    *((template).Element3Negated+1) -= imag;    \
    *((template).Element4Negated+1) -= imag;    \
}

/*
 * Macro function that adds data to each of the four complex matrix
 * elements specified by the given template.
 */
#define  spADD_COMPLEX_QUAD(template,real,imag) \
{   *((template).Element1) += real;             \
    *((template).Element2) += real;             \
    *((template).Element3Negated) -= real;      \
    *((template).Element4Negated) -= real;      \
    *((template).Element1+1) += imag;           \
    *((template).Element2+1) += imag;           \
    *((template).Element3Negated+1) -= imag;    \
    *((template).Element4Negated+1) -= imag;    \
}

/*
 *   TYPE DEFINITION FOR EXTERNAL MATRIX ELEMENT REFERENCES
 *   External type definitions for Sparse data objects.
 */

/* Declares the type of the a pointer to a matrix. */
typedef void * spMatrix;

/* Declares the type of the a pointer to a matrix element. */
typedef spREAL spElement;

/* Declares the type of the Sparse error codes. */
typedef int spError;

/* TYPE DEFINITION FOR COMPONENT TEMPLATE */
/*
 *   This data structure is used to hold pointers to four related elements in
 *   matrix.  It is used in conjunction with the routines spGetAdmittance(),
 *   spGetQuad(), and spGetOnes().  These routines stuff the structure which
 *   is later used by the spADD_QUAD macro functions above.  It is also
 *   possible for the user to collect four pointers returned by spGetElement()
 *   and stuff them into the template.  The spADD_QUAD routines stuff data
 *   into the matrix in locations specified by Element1 and Element2
 *   without changing the data.  The data is negated before being placed in
 *   Element3 and Element4.
 */

/* Begin `spTemplate'. */
struct  spTemplate
{   spElement	*Element1;
    spElement	*Element2;
    spElement	*Element3Negated;
    spElement	*Element4Negated;
};

__BEGIN_DECLS
void       spClear( spMatrix );
spREAL     spCondition( spMatrix, spREAL, int* );
spMatrix   spCreate( int, int, spError* );
void       spDeleteRowAndCol( spMatrix, int, int );
void       spDestroy( spMatrix );
int        spElementCount( spMatrix );
spError    spErrorState( spMatrix );
void       spErrorMessage( spMatrix, FILE*, char* );

spError    spFactor( spMatrix );
int        spFileMatrix( spMatrix, char*, char*, int, int, int );
int        spFileStats( spMatrix, char*, char* );
int        spFillinCount( spMatrix );
spElement *spFindElement( spMatrix, int, int );
spError    spGetAdmittance( spMatrix, int, int, struct spTemplate* );
spElement *spGetElement( spMatrix, int, int );
void      *spGetInitInfo( spElement* );
spError    spGetOnes( spMatrix, int, int, int, struct spTemplate* );
spError    spGetQuad( spMatrix, int, int, int, int, struct spTemplate* );
int        spGetSize( spMatrix, int );
int        spInitialize( spMatrix, int (*pInit)(spElement *, void *,
                         int, int) );
void       spInstallInitInfo( spElement*, void * );
spREAL     spLargestElement( spMatrix );
void       spMNA_Preorder( spMatrix );
spREAL     spNorm( spMatrix );
spError    spOrderAndFactor( spMatrix, spREAL[], spREAL, spREAL, int );
void       spPartition( spMatrix, int );
void       spPrint( spMatrix, int, int, int );
spREAL     spPseudoCondition( spMatrix );
spREAL     spRoundoff( spMatrix, spREAL );
void       spScale( spMatrix, spREAL[], spREAL[] );
void       spSetComplex( spMatrix );
void       spSetReal( spMatrix );
void       spStripFills( spMatrix );
void       spWhereSingular( spMatrix, int*, int* );
void       spDeterminant( spMatrix, int*, spREAL*, spREAL* );
int        spFileVector( spMatrix, char* , spREAL[] );
void       spMultiply( spMatrix, spREAL[], spREAL[] );
void       spMultTransposed( spMatrix, spREAL[], spREAL[] );
void       spSolve( spMatrix, spREAL[], spREAL[] );
void       spSolveTransposed( spMatrix, spREAL[], spREAL[] );

/* Functions added for Edacious */
void       spAddToReorderedDiag(spMatrix, spREAL);
__END_DECLS

#define  BOOLEAN        int
#define  NO             0
#define  YES            1
#define  NOT            !
#define  AND            &&
#define  OR             ||

/* NULL pointer */
#ifndef  NULL
#define  NULL           0
#endif

/* Define macros for validating matrix. */
#define  SPARSE_ID			0xDeadBeef	/* Arbitrary. */
#define  IS_SPARSE(matrix)		(((matrix) != NULL) AND \
                                	 ((matrix)->ID == SPARSE_ID))
#define  NO_ERRORS(matrix)		(((matrix)->Error >= spOKAY) AND \
				 	 ((matrix)->Error < spFATAL))
#define  IS_FACTORED(matrix)    	((matrix)->Factored AND \
					 NOT (matrix)->NeedsOrdering)

#define  ASSERT_IS_SPARSE(matrix)	vASSERT( IS_SPARSE(matrix), \
					 spcMatrixIsNotValid )
#define  ASSERT_NO_ERRORS(matrix)	vASSERT( NO_ERRORS(matrix), \
					 spcErrorsMustBeCleared )
#define  ASSERT_IS_FACTORED(matrix)	vASSERT( IS_FACTORED(matrix), \
					 spcMatrixMustBeFactored )
#define  ASSERT_IS_NOT_FACTORED(matrix)	vASSERT( NOT (matrix)->Factored, \
					 spcMatrixMustNotBeFactored )

/* Macro commands */

/* Macro function that returns the absolute value of a floating point number. */
#define  ABS(a)             ((a) < 0 ? -(a) : (a))

/* Macro function that returns the square of a number. */
#define  SQR(a)             ((a)*(a))

/* Macro procedure that swaps two entities. */
#define  SWAP(type, a, b)   {type swapx; swapx = a; a = b; b = swapx;}

/*
 * COMPLEX OPERATION MACROS
 */

/* Macro function that returns the approx absolute value of a complex number. */
#define  ELEMENT_MAG(ptr)   (ABS((ptr)->Real) + ABS((ptr)->Imag))

/* Complex assignment statements. */
#define  CMPLX_ASSIGN(to,from)  \
{   (to).Real = (from).Real;    \
    (to).Imag = (from).Imag;    \
}
#define  CMPLX_CONJ_ASSIGN(to,from)     \
{   (to).Real = (from).Real;            \
    (to).Imag = -(from).Imag;           \
}
#define  CMPLX_NEGATE_ASSIGN(to,from)   \
{   (to).Real = -(from).Real;           \
    (to).Imag = -(from).Imag;           \
}
#define  CMPLX_CONJ_NEGATE_ASSIGN(to,from)      \
{   (to).Real = -(from).Real;                   \
    (to).Imag = (from).Imag;                    \
}
#define  CMPLX_CONJ(a)  (a).Imag = -(a).Imag
#define  CMPLX_NEGATE(a)        \
{   (a).Real = -(a).Real;       \
    (a).Imag = -(a).Imag;       \
}

/* Macro that returns the approx magnitude (L-1 norm) of a complex number. */
#define  CMPLX_1_NORM(a)        (ABS((a).Real) + ABS((a).Imag))

/* Macro that returns the approx magnitude (L-infinity norm) of a complex. */
#define  CMPLX_INF_NORM(a)      (MAX (ABS((a).Real),ABS((a).Imag)))

/* Macro function that returns the magnitude (L-2 norm) of a complex number. */
#define  CMPLX_2_NORM(a)        (sqrt((a).Real*(a).Real + (a).Imag*(a).Imag))

/* Macro function that performs complex addition. */
#define  CMPLX_ADD(to,from_a,from_b)            \
{   (to).Real = (from_a).Real + (from_b).Real;  \
    (to).Imag = (from_a).Imag + (from_b).Imag;  \
}

/* Macro function that performs complex subtraction. */
#define  CMPLX_SUBT(to,from_a,from_b)           \
{   (to).Real = (from_a).Real - (from_b).Real;  \
    (to).Imag = (from_a).Imag - (from_b).Imag;  \
}

/* Macro function that is equivalent to += operator for complex numbers. */
#define  CMPLX_ADD_ASSIGN(to,from)      \
{   (to).Real += (from).Real;           \
    (to).Imag += (from).Imag;           \
}

/* Macro function that is equivalent to -= operator for complex numbers. */
#define  CMPLX_SUBT_ASSIGN(to,from)     \
{   (to).Real -= (from).Real;           \
    (to).Imag -= (from).Imag;           \
}

/* Macro function that multiplies a complex number by a scalar. */
#define  SCLR_MULT(to,sclr,cmplx)       \
{   (to).Real = (sclr) * (cmplx).Real;  \
    (to).Imag = (sclr) * (cmplx).Imag;  \
}

/* Macro function that multiply-assigns a complex number by a scalar. */
#define  SCLR_MULT_ASSIGN(to,sclr)      \
{   (to).Real *= (sclr);                \
    (to).Imag *= (sclr);                \
}

/* Macro function that multiplies two complex numbers. */
#define  CMPLX_MULT(to,from_a,from_b)           \
{   (to).Real = (from_a).Real * (from_b).Real - \
                (from_a).Imag * (from_b).Imag;  \
    (to).Imag = (from_a).Real * (from_b).Imag + \
                (from_a).Imag * (from_b).Real;  \
}

/* Macro function that implements to *= from for complex numbers. */
#define  CMPLX_MULT_ASSIGN(to,from)             \
{   RealNumber to_real_ = (to).Real;            \
    (to).Real = to_real_ * (from).Real -        \
                (to).Imag * (from).Imag;        \
    (to).Imag = to_real_ * (from).Imag +        \
                (to).Imag * (from).Real;        \
}

/* Macro function that multiplies two complex numbers, the first of which is
 * conjugated. */
#define  CMPLX_CONJ_MULT(to,from_a,from_b)      \
{   (to).Real = (from_a).Real * (from_b).Real + \
                (from_a).Imag * (from_b).Imag;  \
    (to).Imag = (from_a).Real * (from_b).Imag - \
                (from_a).Imag * (from_b).Real;  \
}

/* Macro function that multiplies two complex numbers and then adds them
 * to another. to = add + mult_a * mult_b */
#define  CMPLX_MULT_ADD(to,mult_a,mult_b,add)                   \
{   (to).Real = (mult_a).Real * (mult_b).Real -                 \
                (mult_a).Imag * (mult_b).Imag + (add).Real;     \
    (to).Imag = (mult_a).Real * (mult_b).Imag +                 \
                (mult_a).Imag * (mult_b).Real + (add).Imag;     \
}

/* Macro function that subtracts the product of two complex numbers from
 * another.  to = subt - mult_a * mult_b */
#define  CMPLX_MULT_SUBT(to,mult_a,mult_b,subt)                 \
{   (to).Real = (subt).Real - (mult_a).Real * (mult_b).Real +   \
                              (mult_a).Imag * (mult_b).Imag;    \
    (to).Imag = (subt).Imag - (mult_a).Real * (mult_b).Imag -   \
                              (mult_a).Imag * (mult_b).Real;    \
}

/* Macro function that multiplies two complex numbers and then adds them
 * to another. to = add + mult_a* * mult_b where mult_a* represents mult_a
 * conjugate. */
#define  CMPLX_CONJ_MULT_ADD(to,mult_a,mult_b,add)              \
{   (to).Real = (mult_a).Real * (mult_b).Real +                 \
                (mult_a).Imag * (mult_b).Imag + (add).Real;     \
    (to).Imag = (mult_a).Real * (mult_b).Imag -                 \
                (mult_a).Imag * (mult_b).Real + (add).Imag;     \
}

/* Macro function that multiplies two complex numbers and then adds them
 * to another. to += mult_a * mult_b */
#define  CMPLX_MULT_ADD_ASSIGN(to,from_a,from_b)        \
{   (to).Real += (from_a).Real * (from_b).Real -        \
                 (from_a).Imag * (from_b).Imag;         \
    (to).Imag += (from_a).Real * (from_b).Imag +        \
                 (from_a).Imag * (from_b).Real;         \
}

/* Macro function that multiplies two complex numbers and then subtracts them
 * from another. */
#define  CMPLX_MULT_SUBT_ASSIGN(to,from_a,from_b)       \
{   (to).Real -= (from_a).Real * (from_b).Real -        \
                 (from_a).Imag * (from_b).Imag;         \
    (to).Imag -= (from_a).Real * (from_b).Imag +        \
                 (from_a).Imag * (from_b).Real;         \
}

/* Macro function that multiplies two complex numbers and then adds them
 * to the destination. to += from_a* * from_b where from_a* represents from_a
 * conjugate. */
#define  CMPLX_CONJ_MULT_ADD_ASSIGN(to,from_a,from_b)   \
{   (to).Real += (from_a).Real * (from_b).Real +        \
                 (from_a).Imag * (from_b).Imag;         \
    (to).Imag += (from_a).Real * (from_b).Imag -        \
                 (from_a).Imag * (from_b).Real;         \
}

/* Macro function that multiplies two complex numbers and then subtracts them
 * from the destination. to -= from_a* * from_b where from_a* represents from_a
 * conjugate. */
#define  CMPLX_CONJ_MULT_SUBT_ASSIGN(to,from_a,from_b)  \
{   (to).Real -= (from_a).Real * (from_b).Real +        \
                 (from_a).Imag * (from_b).Imag;         \
    (to).Imag -= (from_a).Real * (from_b).Imag -        \
                 (from_a).Imag * (from_b).Real;         \
}

/*
 * Macro functions that provide complex division.
 */

/* Complex division:  to = num / den */
#define CMPLX_DIV(to,num,den)                                           \
{   RealNumber  r_, s_;                                                 \
    if (((den).Real >= (den).Imag AND (den).Real > -(den).Imag) OR      \
        ((den).Real < (den).Imag AND (den).Real <= -(den).Imag))        \
    {   r_ = (den).Imag / (den).Real;                                   \
        s_ = (den).Real + r_*(den).Imag;                                \
        (to).Real = ((num).Real + r_*(num).Imag)/s_;                    \
        (to).Imag = ((num).Imag - r_*(num).Real)/s_;                    \
    }                                                                   \
    else                                                                \
    {   r_ = (den).Real / (den).Imag;                                   \
        s_ = (den).Imag + r_*(den).Real;                                \
        (to).Real = (r_*(num).Real + (num).Imag)/s_;                    \
        (to).Imag = (r_*(num).Imag - (num).Real)/s_;                    \
    }                                                                   \
}

/* Complex division and assignment:  num /= den */
#define CMPLX_DIV_ASSIGN(num,den)                                       \
{   RealNumber  r_, s_, t_;                                             \
    if (((den).Real >= (den).Imag AND (den).Real > -(den).Imag) OR      \
        ((den).Real < (den).Imag AND (den).Real <= -(den).Imag))        \
    {   r_ = (den).Imag / (den).Real;                                   \
        s_ = (den).Real + r_*(den).Imag;                                \
        t_ = ((num).Real + r_*(num).Imag)/s_;                           \
        (num).Imag = ((num).Imag - r_*(num).Real)/s_;                   \
        (num).Real = t_;                                                \
    }                                                                   \
    else                                                                \
    {   r_ = (den).Real / (den).Imag;                                   \
        s_ = (den).Imag + r_*(den).Real;                                \
        t_ = (r_*(num).Real + (num).Imag)/s_;                           \
        (num).Imag = (r_*(num).Imag - (num).Real)/s_;                   \
        (num).Real = t_;                                                \
    }                                                                   \
}

/* Complex reciprocation:  to = 1.0 / den */
#define CMPLX_RECIPROCAL(to,den)                                        \
{   RealNumber  r_;                                                     \
    if (((den).Real >= (den).Imag AND (den).Real > -(den).Imag) OR      \
        ((den).Real < (den).Imag AND (den).Real <= -(den).Imag))        \
    {   r_ = (den).Imag / (den).Real;                                   \
        (to).Imag = -r_*((to).Real = 1.0/((den).Real + r_*(den).Imag)); \
    }                                                                   \
    else                                                                \
    {   r_ = (den).Real / (den).Imag;                                   \
        (to).Real = -r_*((to).Imag = -1.0/((den).Imag + r_*(den).Real));\
    }                                                                   \
}

/*
 *  ASSERT and ABORT
 *
 *  Macro used to assert that if the code is working correctly, then 
 *  a condition must be true.  If not, then execution is terminated
 *  and an error message is issued stating that there is an internal
 *  error and giving the file and line number.  These assertions are
 *  not evaluated unless the spDEBUG flag is true.
 */

#if spDEBUG
# define ASSERT(condition) \
    { if (NOT(condition)) { AG_FatalError("SPARSE Assertion Failed"); } }
#else
# define ASSERT(condition)
#endif

#if spDEBUG
# define vASSERT(condition,message) { \
if (NOT(condition))			\
	vABORT(message);		\
}
#else
# define vASSERT(condition,message)
#endif

#define vABORT(message) { AG_FatalError("SPARSE Internal Error"); }
#define ABORT() { AG_FatalError("SPARSE Internal Error"); }

/*
 * MEMORY ALLOCATION
 */

#define ALLOC(type,number) \
	((type *)AG_Malloc((sizeof(type)*(number))))
#define REALLOC(ptr,type,number) \
        ptr = (type *)AG_Realloc((char *)ptr,(sizeof(type)*(number)))
#define FREE(ptr) \
	AG_Free(ptr)

/* Calloc that properly handles allocating a cleared vector. */
#define CALLOC(ptr,type,number)                         \
{   int i; ptr = ALLOC(type, number);                   \
    if (ptr != (type *)NULL)                            \
        for(i=(number)-1;i>=0; i--) ptr[i] = (type) 0;  \
}

/*
 * Utility Functions
 */
/*
 * Compute the product of two intergers while avoiding overflow.
 * Used when computing Markowitz products.
 */

#define spcMarkoProd(product, op1, op2) \
        if (( (op1) > LARGEST_SHORT_INTEGER AND (op2) != 0) OR \
            ( (op2) > LARGEST_SHORT_INTEGER AND (op1) != 0)) \
        {   double fProduct = (double)(op1) * (double)(op2); \
            if (fProduct >= LARGEST_LONG_INTEGER) \
                (product) = LARGEST_LONG_INTEGER; \
            else \
                (product) = (long)fProduct; \
        } \
        else (product) = (op1)*(op2);

/* Real and complex numbers */
typedef spREAL RealNumber, *RealVector;
typedef struct {
	RealNumber  Real;
	RealNumber  Imag;
} ComplexNumber, *ComplexVector;


/*
 *  MATRIX ELEMENT DATA STRUCTURE
 *
 *  Every nonzero element in the matrix is stored in a dynamically allocated
 *  MatrixElement structure.  These structures are linked together in an
 *  orthogonal linked list.  Two different MatrixElement structures exist.
 *  One is used when only real matrices are expected, it is missing an entry
 *  for imaginary data.  The other is used if complex matrices are expected.
 *  It contains an entry for imaginary data.
 *
 *  >>> Structure fields:
 *  Real  (RealNumber)
 *      The real portion of the value of the element.  Real must be the first
 *      field in this structure.
 *  Imag  (RealNumber)
 *      The imaginary portion of the value of the element. If the matrix
 *      routines are not compiled to handle complex matrices, then this
 *      field does not exist.  If it exists, it must follow immediately after
 *      Real.
 *  Row  (int)
 *      The row number of the element.
 *  Col  (int)
 *      The column number of the element.
 *  NextInRow  (struct MatrixElement *)
 *      NextInRow contains a pointer to the next element in the row to the
 *      right of this element.  If this element is the last nonzero in the
 *      row then NextInRow contains NULL.
 *  NextInCol  (struct MatrixElement *)
 *      NextInCol contains a pointer to the next element in the column below
 *      this element.  If this element is the last nonzero in the column then
 *      NextInCol contains NULL.
 *  pInitInfo  (void *)
 *      Pointer to user data used for initialization of the matrix element.
 *      Initialized to NULL.
 *
 *  >>> Type definitions:
 *  ElementPtr
 *      A pointer to a MatrixElement.
 *  ArrayOfElementPtrs
 *      An array of ElementPtrs.  Used for FirstInRow, FirstInCol and
 *      Diag pointer arrays.
 */

/* Begin `MatrixElement'. */

struct  MatrixElement
{   RealNumber   Real;
    RealNumber   Imag;
    int          Row;
    int          Col;
    struct MatrixElement  *NextInRow;
    struct MatrixElement  *NextInCol;
    void * pInitInfo;
};

typedef  struct MatrixElement  *ElementPtr;
typedef  ElementPtr  *ArrayOfElementPtrs;

/*
 *  ALLOCATION DATA STRUCTURE
 *
 *  The sparse matrix routines keep track of all memory that is allocated by
 *  the operating system so the memory can later be freed.  This is done by
 *  saving the pointers to all the chunks of memory that are allocated to a
 *  particular matrix in an allocation list.  That list is organized as a
 *  linked list so that it can grow without a priori bounds.
 *
 *  >>> Structure fields:
 *  AllocatedPtr  (void *)
 *      Pointer to chunk of memory that has been allocated for the matrix.
 *  NextRecord  (struct  AllocationRecord *)
 *      Pointer to the next allocation record.
 */

/* Begin `AllocationRecord'. */
struct AllocationRecord
{   void *AllocatedPtr;
    struct AllocationRecord *NextRecord;
};

typedef struct AllocationRecord *AllocationListPtr;

/*
 *  FILL-IN LIST DATA STRUCTURE
 *
 *  The sparse matrix routines keep track of all fill-ins separately from
 *  user specified elements so they may be removed by spStripFills().  Fill-ins
 *  are allocated in bunched in what is called a fill-in lists.  The data
 *  structure defined below is used to organize these fill-in lists into a
 *  linked-list.
 *
 *  >>> Structure fields:
 *  pFillinList  (ElementPtr)
 *      Pointer to a fill-in list, or a bunch of fill-ins arranged contiguously
 *      in memory.
 *  NumberOfFillinsInList  (int)
 *      Seems pretty self explanatory to me.
 *  Next  (struct  FillinListNodeStruct *)
 *      Pointer to the next fill-in list structures.
 */

/* Begin `FillinListNodeStruct'. */
struct FillinListNodeStruct
{   ElementPtr  pFillinList;
    int         NumberOfFillinsInList;
    Uint32 _pad;
    struct      FillinListNodeStruct  *Next;
};

/*
 *  MATRIX FRAME DATA STRUCTURE
 *
 *  This structure contains all the pointers that support the orthogonal
 *  linked list that contains the matrix elements.  Also included in this
 *  structure are other numbers and pointers that are used globally by the
 *  sparse matrix routines and are associated with one particular matrix.
 *
 *  >>> Type definitions:
 *  MatrixPtr
 *      A pointer to MatrixFrame.  Essentially, a pointer to the matrix.
 *
 *  >>> Structure fields:
 *  AbsThreshold  (RealNumber)
 *      The absolute magnitude an element must have to be considered as a
 *      pivot candidate, except as a last resort.
 *  AllocatedExtSize  (int)
 *      The allocated size of the arrays used to translate external row and
 *      column numbers to their internal values.
 *  AllocatedSize  (int)
 *      The currently allocated size of the matrix; the size the matrix can
 *      grow to when EXPANDABLE is set true and AllocatedSize is the largest
 *      the matrix can get without requiring that the matrix frame be
 *      reallocated.
 *  Complex  (BOOLEAN)
 *      The flag which indicates whether the matrix is complex (true) or
 *      real.
 *  CurrentSize  (int)
 *      This number is used during the building of the matrix when the
 *      TRANSLATE option is set true.  It indicates the number of internal
 *      rows and columns that have elements in them.
 *  Diag  (ArrayOfElementPtrs)
 *      Array of pointers that points to the diagonal elements.
 *  DoCmplxDirect  (BOOLEAN *)
 *      Array of flags, one for each column in matrix.  If a flag is true
 *      then corresponding column in a complex matrix should be eliminated
 *      in spFactor() using direct addressing (rather than indirect
 *      addressing).
 *  DoRealDirect  (BOOLEAN *)
 *      Array of flags, one for each column in matrix.  If a flag is true
 *      then corresponding column in a real matrix should be eliminated
 *      in spFactor() using direct addressing (rather than indirect
 *      addressing).
 *  Elements  (int)
 *      The number of original elements (total elements minus fill ins)
 *      present in matrix.
 *  Error  (int)
 *      The error status of the sparse matrix package.
 *  ExtSize  (int)
 *      The value of the largest external row or column number encountered.
 *  ExtToIntColMap  (int [])
 *      An array that is used to convert external columns number to internal
 *      external column numbers.  Present only if TRANSLATE option is set true.
 *  ExtToIntRowMap  (int [])
 *      An array that is used to convert external row numbers to internal
 *      external row numbers.  Present only if TRANSLATE option is set true.
 *  Factored  (BOOLEAN)
 *      Indicates if matrix has been factored.  This flag is set true in
 *      spFactor() and spOrderAndFactor() and set false in spCreate()
 *      and spClear().
 *  Fillins  (int)
 *      The number of fill-ins created during the factorization the matrix.
 *  FirstInCol  (ArrayOfElementPtrs)
 *      Array of pointers that point to the first nonzero element of the
 *      column corresponding to the index.
 *  FirstInRow  (ArrayOfElementPtrs)
 *      Array of pointers that point to the first nonzero element of the row
 *      corresponding to the index.
 *  ID  (unsigned long int)
 *      A constant that provides the sparse data structure with a signature.
 *      When spDEBUG is true, all externally available sparse routines check
 *      this signature to assure they are operating on a valid matrix.
 *  Intermediate  (RealVector)
 *      Temporary storage used in the spSolve routines. Intermediate is an
 *      array used during forward and backward substitution.  It is
 *      commonly called y when the forward and backward substitution process is
 *      denoted  Ax = b => Ly = b and Ux = y.
 *  InternalVectorsAllocated  (BOOLEAN)
 *      A flag that indicates whether theMmarkowitz vectors and the
 *      Intermediate vector have been created.
 *      These vectors are created in spcCreateInternalVectors().
 *  IntToExtColMap  (int [])
 *      An array that is used to convert internal column numbers to external
 *      external column numbers.
 *  IntToExtRowMap  (int [])
 *      An array that is used to convert internal row numbers to external
 *      external row numbers.
 *  MarkowitzCol  (int [])
 *      An array that contains the count of the non-zero elements excluding
 *      the pivots for each column. Used to generate and update MarkowitzProd.
 *  MarkowitzProd  (long [])
 *      The array of the products of the Markowitz row and column counts. The
 *      element with the smallest product is the best pivot to use to maintain
 *      sparsity.
 *  MarkowitzRow  (int [])
 *      An array that contains the count of the non-zero elements excluding
 *      the pivots for each row. Used to generate and update MarkowitzProd.
 *  MaxRowCountInLowerTri  (int)
 *      The maximum number of off-diagonal element in the rows of L, the
 *      lower triangular matrix.  This quantity is used when computing an
 *      estimate of the roundoff error in the matrix.
 *  NeedsOrdering  (BOOLEAN)
 *      This is a flag that signifies that the matrix needs to be ordered
 *      or reordered.  NeedsOrdering is set true in spCreate() and
 *      spGetElement() or spGetAdmittance() if new elements are added to the
 *      matrix after it has been previously factored.  It is set false in
 *      spOrderAndFactor().
 *  NumberOfInterchangesIsOdd  (BOOLEAN)
 *      Flag that indicates the sum of row and column interchange counts
 *      is an odd number.  Used when determining the sign of the determinant.
 *  Partitioned  (BOOLEAN)
 *      This flag indicates that the columns of the matrix have been 
 *      partitioned into two groups.  Those that will be addressed directly
 *      and those that will be addressed indirectly in spFactor().
 *  PivotsOriginalCol  (int)
 *      Column pivot was chosen from.
 *  PivotsOriginalRow  (int)
 *      Row pivot was chosen from.
 *  PivotSelectionMethod  (char)
 *      Character that indicates which pivot search method was successful.
 *  PreviousMatrixWasComplex  (BOOLEAN)
 *      This flag in needed to determine how to clear the matrix.  When
 *      dealing with real matrices, it is important that the imaginary terms
 *      in the matrix elements be zero.  Thus, if the previous matrix was
 *      complex, then the current matrix will be cleared as if it were complex
 *      even if it is real.
 *  RelThreshold  (RealNumber)
 *      The magnitude an element must have relative to others in its row
 *      to be considered as a pivot candidate, except as a last resort.
 *  Reordered  (BOOLEAN)
 *      This flag signifies that the matrix has been reordered.  It
 *      is cleared in spCreate(), set in spMNA_Preorder() and
 *      spOrderAndFactor() and is used in spPrint().
 *  RowsLinked  (BOOLEAN)
 *      A flag that indicates whether the row pointers exist.  The AddByIndex
 *      routines do not generate the row pointers, which are needed by some
 *      of the other routines, such as spOrderAndFactor() and spScale().
 *      The row pointers are generated in the function spcLinkRows().
 *  SingularCol  (int)
 *      Normally zero, but if matrix is found to be singular, SingularCol is
 *      assigned the external column number of pivot that was zero.
 *  SingularRow  (int)
 *      Normally zero, but if matrix is found to be singular, SingularRow is
 *      assigned the external row number of pivot that was zero.
 *  Singletons  (int)
 *      The number of singletons available for pivoting.  Note that if row I
 *      and column I both contain singletons, only one of them is counted.
 *  Size  (int)
 *      Number of rows and columns in the matrix.  Does not change as matrix
 *      is factored.
 *  TrashCan  (MatrixElement)
 *      This is a dummy MatrixElement that is used to by the user to stuff
 *      data related to the zero row or column.  In other words, when the user
 *      adds an element in row zero or column zero, then the matrix returns
 *      a pointer to TrashCan.  In this way the user can have a uniform way
 *      data into the matrix independent of whether a component is connected
 *      to ground.
 *
 *  >>> The remaining fields are related to memory allocation.
 *  TopOfAllocationList  (AllocationListPtr)
 *      Pointer which points to the top entry in a list. The list contains
 *      all the pointers to the segments of memory that have been allocated
 *      to this matrix. This is used when the memory is to be freed on
 *      deallocation of the matrix.
 *  RecordsRemaining  (int)
 *      Number of slots left in the list of allocations.
 *  NextAvailElement  (ElementPtr)
 *      Pointer to the next available element which has been allocated but as
 *      yet is unused. Matrix elements are allocated in groups of
 *      ELEMENTS_PER_ALLOCATION in order to speed element allocation and
 *      freeing.
 *  ElementsRemaining  (int)
 *      Number of unused elements left in last block of elements allocated.
 *  NextAvailFillin  (ElementPtr)
 *      Pointer to the next available fill-in which has been allocated but
 *      as yet is unused.  Fill-ins are allocated in a group in order to keep
 *      them physically close in memory to the rest of the matrix.
 *  FillinsRemaining  (int)
 *      Number of unused fill-ins left in the last block of fill-ins
 *      allocated.
 *  FirstFillinListNode  (FillinListNodeStruct *)
 *      A pointer to the head of the linked-list that keeps track of the
 *      lists of fill-ins.
 *  LastFillinListNode  (FillinListNodeStruct *)
 *      A pointer to the tail of the linked-list that keeps track of the
 *      lists of fill-ins.
 */

/* Begin `MatrixFrame'. */
struct  MatrixFrame
{   RealNumber                   AbsThreshold;
    int                          AllocatedSize;
    int                          AllocatedExtSize;
    BOOLEAN                      Complex;
    int                          CurrentSize;
    ArrayOfElementPtrs           Diag;
    BOOLEAN                     *DoCmplxDirect;
    BOOLEAN                     *DoRealDirect;
    int                          Elements;
    int                          Error;
    int                          ExtSize;
    Uint32 _pad1;
    int                         *ExtToIntColMap;
    int                         *ExtToIntRowMap;
    BOOLEAN                      Factored;
    int                          Fillins;
    ArrayOfElementPtrs           FirstInCol;
    ArrayOfElementPtrs           FirstInRow;
    unsigned long                ID;
    RealVector                   Intermediate;
    BOOLEAN                      InternalVectorsAllocated;
    Uint32 _pad2;
    int                         *IntToExtColMap;
    int                         *IntToExtRowMap;
    int                         *MarkowitzRow;
    int                         *MarkowitzCol;
    long                        *MarkowitzProd;
    int                          MaxRowCountInLowerTri;
    BOOLEAN                      NeedsOrdering;
    BOOLEAN                      NumberOfInterchangesIsOdd;
    BOOLEAN                      Partitioned;
    int                          PivotsOriginalCol;
    int                          PivotsOriginalRow;
    char                         PivotSelectionMethod;
    Uint8 _pad3[3];
    BOOLEAN                      PreviousMatrixWasComplex;
    RealNumber                   RelThreshold;
    BOOLEAN                      Reordered;
    BOOLEAN                      RowsLinked;
    int                          SingularCol;
    int                          SingularRow;
    int                          Singletons;
    int                          Size;
    struct MatrixElement         TrashCan;

    AllocationListPtr            TopOfAllocationList;
    int                          RecordsRemaining;
    Uint32 _pad4;
    ElementPtr                   NextAvailElement;
    int                          ElementsRemaining;
    Uint32 _pad5;
    ElementPtr                   NextAvailFillin;
    int                          FillinsRemaining;
    Uint32 _pad6;
    struct FillinListNodeStruct *FirstFillinListNode;
    struct FillinListNodeStruct *LastFillinListNode;
};
typedef struct MatrixFrame *MatrixPtr;

__BEGIN_DECLS
extern const char *spcMatrixIsNotValid;
extern const char *spcErrorsMustBeCleared;
extern const char *spcMatrixMustBeFactored;
extern const char *spcMatrixMustNotBeFactored;

ElementPtr spcGetElement( MatrixPtr );
ElementPtr spcGetFillin( MatrixPtr );
ElementPtr spcFindDiag( MatrixPtr, int );
ElementPtr spcCreateElement( MatrixPtr, int, int, ElementPtr*, ElementPtr*,
                             int );
void spcCreateInternalVectors( MatrixPtr );
void spcLinkRows( MatrixPtr );
void spcColExchange( MatrixPtr, int, int );
void spcRowExchange( MatrixPtr, int, int );
__END_DECLS

#include <agar/math/close.h>

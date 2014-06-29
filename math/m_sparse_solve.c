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
/*
 *  MATRIX SOLVE MODULE
 *
 *  Author:                     Advising professor:
 *      Kenneth S. Kundert          Alberto Sangiovanni-Vincentelli
 *      UC Berkeley
 *
 *  This file contains the forward and backward substitution routines for
 *  the sparse matrix routines.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>
#include <agar/math/m_sparse.h>

static void SolveComplexMatrix( MatrixPtr, RealVector, RealVector );
static void SolveComplexTransposedMatrix( MatrixPtr, RealVector, RealVector );

/*!
 *  Performs forward elimination and back substitution to find the
 *  unknown vector from the \a RHS vector and factored matrix.  This
 *  routine assumes that the pivots are associated with the lower
 *  triangular matrix and that the diagonal of the upper triangular
 *  matrix consists of ones.  This routine arranges the computation
 *  in different way than is traditionally used in order to exploit the
 *  sparsity of the right-hand side.  See the reference in spRevision.
 *
 *  \param eMatrix
 *      Pointer to matrix.
 *  \param RHS
 *      \a RHS is the input data array, the right hand side. This data is
 *      undisturbed and may be reused for other solves.
 *  \param Solution
 *      \a Solution is the output data array. This routine is constructed
 *      such that \a RHS and \a Solution can be the same array.
 */
/*  >>> Local variables:
 *  Intermediate  (RealVector)
 *      Temporary storage for use in forward elimination and backward
 *      substitution.  Commonly referred to as c, when the LU factorization
 *      equations are given as  Ax = b, Lc = b, Ux = c Local version of
 *      Matrix->Intermediate, which was created during the initial
 *      factorization in function spcCreateInternalVectors() in the matrix
 *      factorization module.
 *  pElement  (ElementPtr)
 *      Pointer used to address elements in both the lower and upper triangle
 *      matrices.
 *  pExtOrder  (int *)
 *      Pointer used to sequentially access each entry in IntToExtRowMap
 *      and IntToExtColMap arrays.  Used to quickly scramble and unscramble
 *      RHS and Solution to account for row and column interchanges.
 *  pPivot  (ElementPtr)
 *      Pointer that points to current pivot or diagonal element.
 *  Size  (int)
 *      Size of matrix. Made local to reduce indirection.
 *  Temp  (RealNumber)
 *      Temporary storage for entries in arrays.
 */

/*VARARGS3*/

void
spSolve(
    spMatrix eMatrix,
    spREAL RHS[],
    spREAL Solution[]
)
{
MatrixPtr  Matrix = (MatrixPtr)eMatrix;
register  ElementPtr  pElement;
register  RealVector  Intermediate;
register  RealNumber  Temp;
register  int  I, *pExtOrder, Size;
ElementPtr  pPivot;

/* Begin `spSolve'. */
    ASSERT_IS_SPARSE( Matrix );
    ASSERT_NO_ERRORS( Matrix );
    ASSERT_IS_FACTORED( Matrix );

    if (Matrix->Complex)
    {   SolveComplexMatrix( Matrix, RHS, Solution );
        return;
    }

    Intermediate = Matrix->Intermediate;
    Size = Matrix->Size;

/* Initialize Intermediate vector. */
    pExtOrder = &Matrix->IntToExtRowMap[Size];
    for (I = Size; I > 0; I--)
        Intermediate[I] = RHS[*(pExtOrder--)];

/* Forward elimination. Solves Lc = b.*/
    for (I = 1; I <= Size; I++)
    {   
/* This step of the elimination is skipped if Temp equals zero. */
        if ((Temp = Intermediate[I]) != 0.0)
        {   pPivot = Matrix->Diag[I];
            Intermediate[I] = (Temp *= pPivot->Real);

            pElement = pPivot->NextInCol;
            while (pElement != NULL)
            {   Intermediate[pElement->Row] -= Temp * pElement->Real;
                pElement = pElement->NextInCol;
            }
        }
    }

/* Backward Substitution. Solves Ux = c.*/
    for (I = Size; I > 0; I--)
    {   Temp = Intermediate[I];
        pElement = Matrix->Diag[I]->NextInRow;
        while (pElement != NULL)
        {   Temp -= pElement->Real * Intermediate[pElement->Col];
            pElement = pElement->NextInRow;
        }
        Intermediate[I] = Temp;
    }

/* Unscramble Intermediate vector while placing data in to Solution vector. */
    pExtOrder = &Matrix->IntToExtColMap[Size];
    for (I = Size; I > 0; I--)
        Solution[*(pExtOrder--)] = Intermediate[I];

    return;
}

/*!
 *  Performs forward elimination and back substitution to find the
 *  unknown vector from the RHS vector and factored matrix.  This
 *  routine assumes that the pivots are associated with the lower
 *  triangular matrix and that the diagonal of the upper triangular
 *  matrix consists of ones.  This routine arranges the computation
 *  in different way than is traditionally used in order to exploit the
 *  sparsity of the right-hand side.  See the reference in spRevision.
 *
 *  \param Matrix
 *      Pointer to matrix.
 *  \param RHS
 *      RHS is the real portion of the input data array, the right hand
 *      side. This data is undisturbed and may be reused for other solves.
 *  \param Solution
 *      Solution is the real portion of the output data array. This routine
 *      is constructed such that RHS and Solution can be the same
 *      array.
 */
/*  >>> Local variables:
 *  Intermediate  (ComplexVector)
 *      Temporary storage for use in forward elimination and backward
 *      substitution.  Commonly referred to as c, when the LU factorization
 *      equations are given as  Ax = b, Lc = b, Ux = c.
 *      Local version of Matrix->Intermediate, which was created during
 *      the initial factorization in function spcCreateInternalVectors() in the
 *      matrix factorization module.
 *  pElement  (ElementPtr)
 *      Pointer used to address elements in both the lower and upper triangle
 *      matrices.
 *  pExtOrder  (int *)
 *      Pointer used to sequentially access each entry in IntToExtRowMap
 *      and IntToExtColMap arrays.  Used to quickly scramble and unscramble
 *      RHS and Solution to account for row and column interchanges.
 *  pPivot  (ElementPtr)
 *      Pointer that points to current pivot or diagonal element.
 *  Size  (int)
 *      Size of matrix. Made local to reduce indirection.
 *  Temp  (ComplexNumber)
 *      Temporary storage for entries in arrays.
 */

static void
SolveComplexMatrix(
    MatrixPtr Matrix,
    RealVector RHS,
    RealVector Solution
)
{
register  ElementPtr  pElement;
register  ComplexVector  Intermediate;
register  int  I, *pExtOrder, Size;
ElementPtr  pPivot;
register ComplexVector  ExtVector;
ComplexNumber  Temp;

/* Begin `SolveComplexMatrix'. */

    Size = Matrix->Size;
    Intermediate = (ComplexVector)Matrix->Intermediate;

/* Initialize Intermediate vector. */
    pExtOrder = &Matrix->IntToExtRowMap[Size];

    ExtVector = (ComplexVector)RHS;
    for (I = Size; I > 0; I--)
        Intermediate[I] = ExtVector[*(pExtOrder--)];

/* Forward substitution. Solves Lc = b.*/
    for (I = 1; I <= Size; I++)
    {   Temp = Intermediate[I];

/* This step of the substitution is skipped if Temp equals zero. */
        if ((Temp.Real != 0.0) OR (Temp.Imag != 0.0))
        {   pPivot = Matrix->Diag[I];
/* Cmplx expr: Temp *= (1.0 / Pivot). */
            CMPLX_MULT_ASSIGN(Temp, *pPivot);
            Intermediate[I] = Temp;
            pElement = pPivot->NextInCol;
            while (pElement != NULL)
            {
/* Cmplx expr: Intermediate[Element->Row] -= Temp * *Element. */
                CMPLX_MULT_SUBT_ASSIGN(Intermediate[pElement->Row],
                                       Temp, *pElement);
                pElement = pElement->NextInCol;
            }
        }
    }

/* Backward Substitution. Solves Ux = c.*/
    for (I = Size; I > 0; I--)
    {   Temp = Intermediate[I];
        pElement = Matrix->Diag[I]->NextInRow;

        while (pElement != NULL)
        {
/* Cmplx expr: Temp -= *Element * Intermediate[Element->Col]. */
            CMPLX_MULT_SUBT_ASSIGN(Temp, *pElement,Intermediate[pElement->Col]);
            pElement = pElement->NextInRow;
        }
        Intermediate[I] = Temp;
    }

/* Unscramble Intermediate vector while placing data in to Solution vector. */
    pExtOrder = &Matrix->IntToExtColMap[Size];

    ExtVector = (ComplexVector)Solution;
    for (I = Size; I > 0; I--)
        ExtVector[*(pExtOrder--)] = Intermediate[I];

    return;
}

/*!
 *  Performs forward elimination and back substitution to find the
 *  unknown vector from the RHS vector and transposed factored
 *  matrix. This routine is useful when performing sensitivity analysis
 *  on a circuit using the adjoint method.  This routine assumes that
 *  the pivots are associated with the untransposed lower triangular
 *  matrix and that the diagonal of the untransposed upper
 *  triangular matrix consists of ones.
 *
 *  \param eMatrix
 *      Pointer to matrix.
 *  \param RHS
 *      \a RHS is the input data array, the right hand side. This data is
 *      undisturbed and may be reused for other solves.
 *  \param Solution
 *      \a Solution is the output data array. This routine is constructed
 *      such that \a RHS and \a Solution can be the same array.
 */
/*  >>> Local variables:
 *  Intermediate  (RealVector)
 *      Temporary storage for use in forward elimination and backward
 *      substitution.  Commonly referred to as c, when the LU factorization
 *      equations are given as  Ax = b, Lc = b, Ux = c.  Local version of
 *      Matrix->Intermediate, which was created during the initial
 *      factorization in function spcCreateInternalVectors() in the matrix
 *      factorization module.
 *  pElement  (ElementPtr)
 *      Pointer used to address elements in both the lower and upper triangle
 *      matrices.
 *  pExtOrder  (int *)
 *      Pointer used to sequentially access each entry in IntToExtRowMap
 *      and IntToExtRowMap arrays.  Used to quickly scramble and unscramble
 *      RHS and Solution to account for row and column interchanges.
 *  pPivot  (ElementPtr)
 *      Pointer that points to current pivot or diagonal element.
 *  Size  (int)
 *      Size of matrix. Made local to reduce indirection.
 *  Temp  (RealNumber)
 *      Temporary storage for entries in arrays.
 */

/*VARARGS3*/

void
spSolveTransposed(
    spMatrix eMatrix,
    spREAL RHS[],
    spREAL Solution[]
)
{
MatrixPtr  Matrix = (MatrixPtr)eMatrix;
register  ElementPtr  pElement;
register  RealVector  Intermediate;
register  int  I, *pExtOrder, Size;
ElementPtr  pPivot;
RealNumber  Temp;

/* Begin `spSolveTransposed'. */
    ASSERT_IS_SPARSE( Matrix );
    ASSERT_NO_ERRORS( Matrix );
    ASSERT_IS_FACTORED( Matrix );

    if (Matrix->Complex)
    {   SolveComplexTransposedMatrix( Matrix, RHS, Solution );
        return;
    }

    Size = Matrix->Size;
    Intermediate = Matrix->Intermediate;

/* Initialize Intermediate vector. */
    pExtOrder = &Matrix->IntToExtColMap[Size];
    for (I = Size; I > 0; I--)
        Intermediate[I] = RHS[*(pExtOrder--)];

/* Forward elimination. */
    for (I = 1; I <= Size; I++)
    {   
/* This step of the elimination is skipped if Temp equals zero. */
        if ((Temp = Intermediate[I]) != 0.0)
        {   pElement = Matrix->Diag[I]->NextInRow;
            while (pElement != NULL)
            {   Intermediate[pElement->Col] -= Temp * pElement->Real;
                pElement = pElement->NextInRow;
            }

        }
    }

/* Backward Substitution. */
    for (I = Size; I > 0; I--)
    {   pPivot = Matrix->Diag[I];
        Temp = Intermediate[I];
        pElement = pPivot->NextInCol;
        while (pElement != NULL)
        {   Temp -= pElement->Real * Intermediate[pElement->Row];
            pElement = pElement->NextInCol;
        }
        Intermediate[I] = Temp * pPivot->Real;
    }

/* Unscramble Intermediate vector while placing data in to Solution vector. */
    pExtOrder = &Matrix->IntToExtRowMap[Size];
    for (I = Size; I > 0; I--)
        Solution[*(pExtOrder--)] = Intermediate[I];

    return;
}

/*!
 *  Performs forward elimination and back substitution to find the
 *  unknown vector from the RHS vector and transposed factored
 *  matrix. This routine is useful when performing sensitivity analysis
 *  on a circuit using the adjoint method.  This routine assumes that
 *  the pivots are associated with the untransposed lower triangular
 *  matrix and that the diagonal of the untransposed upper
 *  triangular matrix consists of ones.
 *
 *  \param Matrix
 *      Pointer to matrix.
 *  \param RHS
 *      \a RHS is the input data array, the right hand
 *      side. This data is undisturbed and may be reused for other solves.
 *      This vector is only the real portion if the matrix is complex
 *  \param Solution
 *      \a Solution is the real portion of the output data array. This routine
 *      is constructed such that \a RHS and \a Solution can be the same array.
 */
/*  >>> Local variables:
 *  Intermediate  (ComplexVector)
 *      Temporary storage for use in forward elimination and backward
 *      substitution.  Commonly referred to as c, when the LU factorization
 *      equations are given as  Ax = b, Lc = b, Ux = c.  Local version of
 *      Matrix->Intermediate, which was created during
 *      the initial factorization in function spcCreateInternalVectors() in the
 *      matrix factorization module.
 *  pElement  (ElementPtr)
 *      Pointer used to address elements in both the lower and upper triangle
 *      matrices.
 *  pExtOrder  (int *)
 *      Pointer used to sequentially access each entry in IntToExtRowMap
 *      and IntToExtColMap arrays.  Used to quickly scramble and unscramble
 *      RHS and Solution to account for row and column interchanges.
 *  pPivot  (ElementPtr)
 *      Pointer that points to current pivot or diagonal element.
 *  Size  (int)
 *      Size of matrix. Made local to reduce indirection.
 *  Temp  (ComplexNumber)
 *      Temporary storage for entries in arrays.
 */

static void
SolveComplexTransposedMatrix(
    MatrixPtr  Matrix,
    RealVector  RHS,
    RealVector  Solution
)
{
register  ElementPtr  pElement;
register  ComplexVector  Intermediate;
register  int  I, *pExtOrder, Size;
register  ComplexVector  ExtVector;
ElementPtr  pPivot;
ComplexNumber  Temp;

/* Begin `SolveComplexTransposedMatrix'. */

    Size = Matrix->Size;
    Intermediate = (ComplexVector)Matrix->Intermediate;

/* Initialize Intermediate vector. */
    pExtOrder = &Matrix->IntToExtColMap[Size];

    ExtVector = (ComplexVector)RHS;
    for (I = Size; I > 0; I--)
        Intermediate[I] = ExtVector[*(pExtOrder--)];

/* Forward elimination. */
    for (I = 1; I <= Size; I++)
    {   Temp = Intermediate[I];

/* This step of the elimination is skipped if Temp equals zero. */
        if ((Temp.Real != 0.0) OR (Temp.Imag != 0.0))
        {   pElement = Matrix->Diag[I]->NextInRow;
            while (pElement != NULL)
            {
/* Cmplx expr: Intermediate[Element->Col] -= Temp * *Element. */
                CMPLX_MULT_SUBT_ASSIGN( Intermediate[pElement->Col],
                                        Temp, *pElement);
                pElement = pElement->NextInRow;
            }
        }
    }

/* Backward Substitution. */
    for (I = Size; I > 0; I--)
    {   pPivot = Matrix->Diag[I];
        Temp = Intermediate[I];
        pElement = pPivot->NextInCol;

        while (pElement != NULL)
        {
/* Cmplx expr: Temp -= Intermediate[Element->Row] * *Element. */
            CMPLX_MULT_SUBT_ASSIGN(Temp,Intermediate[pElement->Row],*pElement);

            pElement = pElement->NextInCol;
        }
/* Cmplx expr: Intermediate = Temp * (1.0 / *pPivot). */
        CMPLX_MULT(Intermediate[I], Temp, *pPivot);
    }

/* Unscramble Intermediate vector while placing data in to Solution vector. */
    pExtOrder = &Matrix->IntToExtRowMap[Size];

    ExtVector = (ComplexVector)Solution;
    for (I = Size; I > 0; I--)
        ExtVector[*(pExtOrder--)] = Intermediate[I];

    return;
}

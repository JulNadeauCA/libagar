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
 *  MATRIX OUTPUT MODULE
 *
 *  Author:                     Advisor:
 *      Kenneth S. Kundert          Alberto Sangiovanni-Vincentelli
 *      UC Berkeley
 * 
 *  This file contains the output-to-file and output-to-screen routines for
 *  the matrix package.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>
#include <agar/math/m_sparse.h>

#if DOCUMENTATION

/*!
 *  Formats and send the matrix to standard output.  Some elementary
 *  statistics are also output.  The matrix is output in a format that is
 *  readable by people.
 *
 *  \param eMatrix
 *      Pointer to matrix.
 *  \param PrintReordered
 *      Indicates whether the matrix should be printed out in its original
 *      form, as input by the user, or whether it should be printed in its
 *      reordered form, as used by the matrix routines.  A zero indicates that
 *      the matrix should be printed as inputed, a one indicates that it
 *      should be printed reordered.
 *  \param Data
 *      Boolean flag that when false indicates that output should be
 *      compressed such that only the existence of an element should be
 *      indicated rather than giving the actual value.  Thus 11 times as
 *      many can be printed on a row.  A zero signifies that the matrix
 *      should be printed compressed. A one indicates that the matrix
 *      should be printed in all its glory.
 *  \param Header
 *      Flag indicating that extra information should be given, such as row
 *      and column numbers.
 */
/*  >>> Local variables:
 *  Col  (int)
 *      Column being printed.
 *  ElementCount  (int)
 *      Variable used to count the number of nonzero elements in the matrix.
 *  LargestElement  (RealNumber)
 *      The magnitude of the largest element in the matrix.
 *  LargestDiag  (RealNumber)
 *      The magnitude of the largest diagonal in the matrix.
 *  Magnitude  (RealNumber)
 *      The absolute value of the matrix element being printed.
 *  PrintOrdToIntColMap  (int [])
 *      A translation array that maps the order that columns will be
 *      printed in (if not PrintReordered) to the internal column numbers.
 *  PrintOrdToIntRowMap  (int [])
 *      A translation array that maps the order that rows will be
 *      printed in (if not PrintReordered) to the internal row numbers.
 *  pElement  (ElementPtr)
 *      Pointer to the element in the matrix that is to be printed.
 *  pImagElements  (ElementPtr [ ])
 *      Array of pointers to elements in the matrix.  These pointers point
 *      to the elements whose real values have just been printed.  They are
 *      used to quickly access those same elements so their imaginary values
 *      can be printed.
 *  Row  (int)
 *      Row being printed.
 *  Size  (int)
 *      The size of the matrix.
 *  SmallestDiag  (RealNumber)
 *      The magnitude of the smallest diagonal in the matrix.
 *  SmallestElement  (RealNumber)
 *      The magnitude of the smallest element in the matrix excluding zero
 *      elements.
 *  StartCol  (int)
 *      The column number of the first column to be printed in the group of
 *      columns currently being printed.
 *  StopCol  (int)
 *      The column number of the last column to be printed in the group of
 *      columns currently being printed.
 *  Top  (int)
 *      The largest expected external row or column number.
 */

void
spPrint(
    spMatrix eMatrix,
    int PrintReordered,
    int Data,
    int Header
)
{
MatrixPtr  Matrix = (MatrixPtr)eMatrix;
register  int  J = 0;
int I, Row, Col, Size, Top, StartCol = 1, StopCol, Columns, ElementCount = 0;
double  Magnitude, SmallestDiag = 0.0, SmallestElement = 0.0;
double  LargestElement = 0.0, LargestDiag = 0.0;
ElementPtr  pElement, pImagElements[PRINTER_WIDTH/10+1];
int  *PrintOrdToIntRowMap, *PrintOrdToIntColMap;

/* Begin `spPrint'. */
    ASSERT_IS_SPARSE( Matrix );
    Size = Matrix->Size;

/* Create a packed external to internal row and column translation array. */
# if TRANSLATE
    Top = Matrix->AllocatedExtSize;
#else
    Top = Matrix->AllocatedSize;
#endif
    CALLOC( PrintOrdToIntRowMap, int, Top + 1 );
    CALLOC( PrintOrdToIntColMap, int, Top + 1 );
    if ( PrintOrdToIntRowMap == NULL OR PrintOrdToIntColMap == NULL)
    {   Matrix->Error = spNO_MEMORY;
        return;
    }
    for (I = 1; I <= Size; I++)
    {   PrintOrdToIntRowMap[ Matrix->IntToExtRowMap[I] ] = I;
        PrintOrdToIntColMap[ Matrix->IntToExtColMap[I] ] = I;
    }

/* Pack the arrays. */
    for (J = 1, I = 1; I <= Top; I++)
    {   if (PrintOrdToIntRowMap[I] != 0)
            PrintOrdToIntRowMap[ J++ ] = PrintOrdToIntRowMap[ I ];
    }
    for (J = 1, I = 1; I <= Top; I++)
    {   if (PrintOrdToIntColMap[I] != 0)
            PrintOrdToIntColMap[ J++ ] = PrintOrdToIntColMap[ I ];
    }

/* Print header. */
    if (Header)
    {   printf("MATRIX SUMMARY\n\n");
        printf("Size of matrix = %1d x %1d.\n", Size, Size);
        if ( Matrix->Reordered AND PrintReordered )
            printf("Matrix has been reordered.\n");
        putchar('\n');

        if ( Matrix->Factored )
            printf("Matrix after factorization:\n");
        else
            printf("Matrix before factorization:\n");

        SmallestElement = M_NUMMAX;
        SmallestDiag = SmallestElement;
    }
    if (Size == 0) return;

/* Determine how many columns to use. */
    Columns = PRINTER_WIDTH;
    if (Header) Columns -= 5;
    if (Data) Columns = (Columns+1) / 10;

/*
 * Print matrix by printing groups of complete columns until all the columns
 * are printed.
 */
    J = 0;
    while ( J <= Size )

/* Calculate index of last column to printed in this group. */
    {   StopCol = StartCol + Columns - 1;
        if (StopCol > Size)
            StopCol = Size;

/* Label the columns. */
        if (Header)
        {   if (Data)
            {   printf("    ");
                for (I = StartCol; I <= StopCol; I++)
                {   if (PrintReordered)
                        Col = I;
                    else
                        Col = PrintOrdToIntColMap[I];
                    printf(" %9d", Matrix->IntToExtColMap[ Col ]);
                }
                printf("\n\n");
            }
            else
            {   if (PrintReordered)
                    printf("Columns %1d to %1d.\n",StartCol,StopCol);
                else
                {   printf("Columns %1d to %1d.\n",
                        Matrix->IntToExtColMap[ PrintOrdToIntColMap[StartCol] ],
                        Matrix->IntToExtColMap[ PrintOrdToIntColMap[StopCol] ]);
                }
            }
        }

/* Print every row ...  */
        for (I = 1; I <= Size; I++)
        {   if (PrintReordered)
                Row = I;
            else
                Row = PrintOrdToIntRowMap[I];

            if (Header)
            {   if (PrintReordered AND NOT Data)
                    printf("%4d", I);
                else
                    printf("%4d", Matrix->IntToExtRowMap[ Row ]);
                if (NOT Data) putchar(' ');
            }

/* ... in each column of the group. */
            for (J = StartCol; J <= StopCol; J++)
            {   if (PrintReordered)
                    Col = J;
                else
                    Col = PrintOrdToIntColMap[J];

                pElement = Matrix->FirstInCol[Col];
                while(pElement != NULL AND pElement->Row != Row)
                    pElement = pElement->NextInCol;

                if (Data)
                    pImagElements[J - StartCol] = pElement;

                if (pElement != NULL)

/* Case where element exists */
                {   if (Data)
                        printf(" %9.3g", (double)pElement->Real);
                    else
                        putchar('x');

/* Update status variables */
                    if ( (Magnitude = ELEMENT_MAG(pElement)) > LargestElement )
                        LargestElement = Magnitude;
                    if ((Magnitude < SmallestElement) AND (Magnitude != 0.0))
                        SmallestElement = Magnitude;
                    ElementCount++;
                }

/* Case where element is structurally zero */
                else
                {   if (Data)
                        printf("       ...");
                    else
                        putchar('.');
                }
            }
            putchar('\n');

            if (Matrix->Complex AND Data)
            {   if (Header)
		    printf("    ");
                for (J = StartCol; J <= StopCol; J++)
                {   if (pImagElements[J - StartCol] != NULL)
                    {   printf(" %8.2gj",
                               (double)pImagElements[J-StartCol]->Imag);
                    }
                    else printf("          ");
                }
                putchar('\n');
            }
        }

/* Calculate index of first column in next group. */
        StartCol = StopCol;
        StartCol++;
        putchar('\n');
    }
    if (Header)
    {   printf("\nLargest element in matrix = %-1.4g.\n", LargestElement);
        printf("Smallest element in matrix = %-1.4g.\n", SmallestElement);

/* Search for largest and smallest diagonal values */
        for (I = 1; I <= Size; I++)
        {   if (Matrix->Diag[I] != NULL)
            {   Magnitude = ELEMENT_MAG( Matrix->Diag[I] );
                if ( Magnitude > LargestDiag ) LargestDiag = Magnitude;
                if ( Magnitude < SmallestDiag ) SmallestDiag = Magnitude;
            }
        }

    /* Print the largest and smallest diagonal values */
        if ( Matrix->Factored )
        {   printf("\nLargest diagonal element = %-1.4g.\n", LargestDiag);
            printf("Smallest diagonal element = %-1.4g.\n", SmallestDiag);
        }
        else
        {   printf("\nLargest pivot element = %-1.4g.\n", LargestDiag);
            printf("Smallest pivot element = %-1.4g.\n", SmallestDiag);
        }

    /* Calculate and print sparsity and number of fill-ins created. */
	printf("\nDensity = %2.2f%%.\n", ((double)ElementCount * 100.0)
					 / (((double)Size * (double)Size)));
        if (NOT Matrix->NeedsOrdering)
            printf("Number of fill-ins = %1d.\n", Matrix->Fillins);
    }
    putchar('\n');
    (void)fflush(stdout);

    FREE(PrintOrdToIntColMap);
    FREE(PrintOrdToIntRowMap);
    return;
}











/*!
 *  Writes matrix to file in format suitable to be read back in by the
 *  matrix test program.
 *
 *  \return
 *  One is returned if routine was successful, otherwise zero is returned.
 *  The calling function can query \a errno (the system global error variable)
 *  as to the reason why this routine failed.
 *
 *  \param eMatrix
 *      Pointer to matrix.
 *  \param File
 *      Name of file into which matrix is to be written.
 *  \param Label
 *      String that is transferred to file and is used as a label.
 *  \param Reordered
 *      Specifies whether matrix should be output in reordered form,
 *      or in original order.
 *  \param Data
 *      Indicates that the element values should be output along with
 *      the indices for each element.  This parameter must be true if
 *      matrix is to be read by the sparse test program.
 *  \param Header
 *      Indicates that header is desired.  This parameter must be true if
 *      matrix is to be read by the sparse test program.
 */
/*  >>> Local variables:
 *  Col  (int)
 *      The original column number of the element being output.
 *  pElement  (ElementPtr)
 *      Pointer to an element in the matrix.
 *  pMatrixFile  (FILE *)
 *      File pointer to the matrix file.
 *  Row  (int)
 *      The original row number of the element being output.
 *  Size  (int)
 *      The size of the matrix.
 */

int
spFileMatrix(
    spMatrix eMatrix,
    char *File,
    char *Label,
    int Reordered,
    int Data,
    int Header
)
{
MatrixPtr  Matrix = (MatrixPtr)eMatrix;
register  int  I, Size;
register  ElementPtr  pElement;
int  Row, Col, Err;
FILE  *pMatrixFile;

/* Begin `spFileMatrix'. */
    ASSERT_IS_SPARSE( Matrix );

/* Open file matrix file in write mode. */
    if ((pMatrixFile = fopen(File, "w")) == NULL)
        return 0;

/* Output header. */
    Size = Matrix->Size;
    if (Header)
    {   if (Matrix->Factored AND Data)
        {   Err = fprintf
            (   pMatrixFile,
                "Warning : The following matrix is factored in to LU form.\n"
            );
	    if (Err < 0) return 0;
        }
        if (fprintf(pMatrixFile, "%s\n", Label) < 0) return 0;
        Err = fprintf( pMatrixFile, "%d\t%s\n", Size,
                                    (Matrix->Complex ? "complex" : "real"));
        if (Err < 0) return 0;
    }
    if (Size == 0) return 1;

/* Output matrix. */
    if (NOT Data)
    {   for (I = 1; I <= Size; I++)
        {   pElement = Matrix->FirstInCol[I];
            while (pElement != NULL)
            {   if (Reordered)
                {   Row = pElement->Row;
                    Col = I;
                }
                else
                {   Row = Matrix->IntToExtRowMap[pElement->Row];
                    Col = Matrix->IntToExtColMap[I];
                }
                pElement = pElement->NextInCol;
                if (fprintf(pMatrixFile, "%d\t%d\n", Row, Col) < 0) return 0;
            }
        }
/* Output terminator, a line of zeros. */
        if (Header)
            if (fprintf(pMatrixFile, "0\t0\n") < 0) return 0;
    }

    if (Data AND Matrix->Complex)
    {   for (I = 1; I <= Size; I++)
        {   pElement = Matrix->FirstInCol[I];
            while (pElement != NULL)
            {   if (Reordered)
                {   Row = pElement->Row;
                    Col = I;
                }
                else
                {   Row = Matrix->IntToExtRowMap[pElement->Row];
                    Col = Matrix->IntToExtColMap[I];
                }
                Err = fprintf
                (   pMatrixFile,"%d\t%d\t%-.15g\t%-.15g\n",
                    Row, Col, (double)pElement->Real, (double)pElement->Imag
                );
                if (Err < 0) return 0;
                pElement = pElement->NextInCol;
            }
        }
/* Output terminator, a line of zeros. */
        if (Header)
            if (fprintf(pMatrixFile,"0\t0\t0.0\t0.0\n") < 0) return 0;

    }

    if (Data AND NOT Matrix->Complex)
    {   for (I = 1; I <= Size; I++)
        {   pElement = Matrix->FirstInCol[I];
            while (pElement != NULL)
            {   Row = Matrix->IntToExtRowMap[pElement->Row];
                Col = Matrix->IntToExtColMap[I];
                Err = fprintf
                (   pMatrixFile,"%d\t%d\t%-.15g\n",
                    Row, Col, (double)pElement->Real
                );
                if (Err < 0) return 0;
                pElement = pElement->NextInCol;
            }
        }
/* Output terminator, a line of zeros. */
        if (Header)
            if (fprintf(pMatrixFile,"0\t0\t0.0\n") < 0) return 0;

    }

/* Close file. */
    if (fclose(pMatrixFile) < 0) return 0;
    return 1;
}

/*!
 *  Writes vector to file in format suitable to be read back in by the
 *  matrix test program.  This routine should be executed after the function
 *  spFileMatrix.
 *
 *  \return
 *  One is returned if routine was successful, otherwise zero is returned.
 *  The calling function can query \a errno (the system global error variable)
 *  as to the reason why this routine failed.
 *
 *  \param eMatrix
 *      Pointer to matrix.
 *  \param File
 *      Name of file into which matrix is to be written.
 *  \param RHS
 *      Right-hand side vector.
 */
/*  >>> Local variables:
 *  pMatrixFile  (FILE *)
 *      File pointer to the matrix file.
 *  Size  (int)
 *      The size of the matrix.
 */

int
spFileVector(
    spMatrix eMatrix,
    char *File,
    spREAL RHS[]
)
{
MatrixPtr  Matrix = (MatrixPtr)eMatrix;
register  int  I, Size, Err;
FILE  *pMatrixFile;

/* Begin `spFileVector'. */
    ASSERT_IS_SPARSE( Matrix );
    vASSERT( RHS != NULL, "Vector missing" );

/* Open File in append mode. */
    if ((pMatrixFile = fopen(File,"a")) == NULL)
        return 0;

/* Output vector. */
    Size = Matrix->Size;
    if (Size == 0) return 1;

    if (Matrix->Complex)
    {
        for (I = 1; I <= Size; I++)
        {   Err = fprintf
            (   pMatrixFile, "%-.15g\t%-.15g\n",
                (double)RHS[2*I], (double)RHS[2*I+1]
            );
            if (Err < 0) return 0;
        }
    }
    else
    {   for (I = 1; I <= Size; I++)
        {   if (fprintf(pMatrixFile, "%-.15g\n", (double)RHS[I]) < 0)
                return 0;
        }
    }

/* Close file. */
    if (fclose(pMatrixFile) < 0) return 0;
    return 1;
}









/*!
 *  Writes useful information concerning the matrix to a file.  Should be
 *  executed after the matrix is factored.
 * 
 *  \return
 *  One is returned if routine was successful, otherwise zero is returned.
 *  The calling function can query \a errno (the system global error variable)
 *  as to the reason why this routine failed.
 *
 *  \param eMatrix
 *      Pointer to matrix.
 *  \param File
 *      Name of file into which matrix is to be written.
 *  \param Label
 *      String that is transferred to file and is used as a label.
 */
/*  >>> Local variables:
 *  Data  (RealNumber)
 *      The value of the matrix element being output.
 *  LargestElement  (RealNumber)
 *      The largest element in the matrix.
 *  NumberOfElements  (int)
 *      Number of nonzero elements in the matrix.
 *  pElement  (ElementPtr)
 *      Pointer to an element in the matrix.
 *  pStatsFile  (FILE *)
 *      File pointer to the statistics file.
 *  Size  (int)
 *      The size of the matrix.
 *  SmallestElement  (RealNumber)
 *      The smallest element in the matrix excluding zero elements.
 */

int
spFileStats(
    spMatrix eMatrix,
    char *File,
    char *Label
)
{
MatrixPtr  Matrix = (MatrixPtr)eMatrix;
register  int  Size, I;
register  ElementPtr  pElement;
int NumberOfElements;
RealNumber  Data, LargestElement, SmallestElement;
FILE  *pStatsFile;

/* Begin `spFileStats'. */
    ASSERT_IS_SPARSE( Matrix );

/* Open File in append mode. */
    if ((pStatsFile = fopen(File, "a")) == NULL)
        return 0;

/* Output statistics. */
    Size = Matrix->Size;
    if (NOT Matrix->Factored)
        fprintf(pStatsFile, "Matrix has not been factored.\n");
    fprintf(pStatsFile, "|||  Starting new matrix  |||\n");
    fprintf(pStatsFile, "%s\n", Label);
    if (Matrix->Complex)
        fprintf(pStatsFile, "Matrix is complex.\n");
    else
        fprintf(pStatsFile, "Matrix is real.\n");
    fprintf(pStatsFile,"     Size = %d\n",Size);
    if (Size == 0) return 1;

/* Search matrix. */
    NumberOfElements = 0;
    LargestElement = 0.0;
    SmallestElement = M_NUMMAX;

    for (I = 1; I <= Size; I++)
    {   pElement = Matrix->FirstInCol[I];
        while (pElement != NULL)
        {   NumberOfElements++;
            Data = ELEMENT_MAG(pElement);
            if (Data > LargestElement)
                LargestElement = Data;
            if (Data < SmallestElement AND Data != 0.0)
                SmallestElement = Data;
            pElement = pElement->NextInCol;
        }
    }

    SmallestElement = MIN( SmallestElement, LargestElement );

/* Output remaining statistics. */
    fprintf(pStatsFile, "     Initial number of elements = %d\n",
            NumberOfElements - Matrix->Fillins);
    fprintf(pStatsFile,
            "     Initial average number of elements per row = %f\n",
            (double)(NumberOfElements - Matrix->Fillins) / (double)Size);
    fprintf(pStatsFile, "     Fill-ins = %d\n",Matrix->Fillins);
    fprintf(pStatsFile, "     Average number of fill-ins per row = %f%%\n",
            (double)Matrix->Fillins / (double)Size);
    fprintf(pStatsFile, "     Total number of elements = %d\n",
            NumberOfElements);
    fprintf(pStatsFile, "     Average number of elements per row = %f\n",
            (double)NumberOfElements / (double)Size);
    fprintf(pStatsFile,"     Density = %f%%\n",
	    (100.0*(double)NumberOfElements)/((double)Size*(double)Size));
    fprintf(pStatsFile,"     Relative Threshold = %e\n", Matrix->RelThreshold);
    fprintf(pStatsFile,"     Absolute Threshold = %e\n", Matrix->AbsThreshold);
    fprintf(pStatsFile,"     Largest Element = %e\n", LargestElement);
    fprintf(pStatsFile,"     Smallest Element = %e\n\n\n", SmallestElement);

/* Close file. */
    (void)fclose(pStatsFile);
    return 1;
}
#endif /* DOCUMENTATION */

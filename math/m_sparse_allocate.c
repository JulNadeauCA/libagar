/*
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
 *  MATRIX ALLOCATION MODULE
 *
 *  Author:                     Advising professor:
 *      Kenneth S. Kundert          Alberto Sangiovanni-Vincentelli
 *      UC Berkeley
 *
 *  Functions for allocating and freeing matrices, configuring them, and for
 *  accessing global information about the matrix (size, error status, etc.).
 */

#include <agar/core/core.h>
#include <agar/math/m.h>
#include <agar/math/m_sparse.h>

const char *spcMatrixIsNotValid = "Matrix passed to Sparse is not valid";
const char *spcErrorsMustBeCleared = "Error not cleared";
const char *spcMatrixMustBeFactored = "Matrix must be factored";
const char *spcMatrixMustNotBeFactored = "Matrix must not be factored";

static void InitializeElementBlocks( MatrixPtr, int, int );
static void RecordAllocation( MatrixPtr, void* );
static void AllocateBlockOfAllocationList( MatrixPtr );


/*!
 *  Allocates and initializes the data structures associated with a matrix.
 *
 *  \return
 *  A pointer to the matrix is returned cast into \a spMatrix (typically a
 *  pointer to a void).  This pointer is then passed and used by the other
 *  matrix routines to refer to a particular matrix.  If an error occurs,
 *  the \a NULL pointer is returned.
 *
 *  \param Size
 *      Size of matrix or estimate of size of matrix if matrix is \a EXPANDABLE.
 *  \param Complex
 *      Type of matrix.  If \a Complex is 0 then the matrix is real, otherwise
 *      the matrix will be complex.  Note that if the routines are not set up
 *      to handle the type of matrix requested, then an \a spPANIC error will occur.
 *      Further note that if a matrix will be both real and complex, it must
 *      be specified here as being complex.
 *  \param pError
 *      Returns error flag, needed because function \a spErrorState() will
 *      not work correctly if \a spCreate() returns \a NULL. Possible errors
 *      include \a  spNO_MEMORY and \a spPANIC.
 */
/*  >>> Local variables:
 *  AllocatedSize  (int)
 *      The size of the matrix being allocated.
 *  Matrix  (MatrixPtr)
 *      A pointer to the matrix frame being created.
 */

spMatrix
spCreate(
    int  Size,
    int  Complex,
    spError *pError
)
{
register  unsigned  SizePlusOne;
register  MatrixPtr  Matrix;
register  int  I;
int  AllocatedSize;

/* Begin `spCreate'. */
/* Clear error flag. */
    *pError = spOKAY;

/* Test for valid size. */
    vASSERT( (Size >= 0) AND (Size != 0 OR EXPANDABLE), "Invalid size" );

/* Create Matrix. */
    AllocatedSize = MAX( Size, MINIMUM_ALLOCATED_SIZE );
    SizePlusOne = (unsigned)(AllocatedSize + 1);

    if ((Matrix = ALLOC(struct MatrixFrame, 1)) == NULL)
    {   *pError = spNO_MEMORY;
        return NULL;
    }

/* Initialize matrix */
    Matrix->ID = SPARSE_ID;
    Matrix->Complex = Complex;
    Matrix->PreviousMatrixWasComplex = Complex;
    Matrix->Factored = NO;
    Matrix->Elements = 0;
    Matrix->Error = *pError;
    Matrix->Fillins = 0;
    Matrix->Reordered = NO;
    Matrix->NeedsOrdering = YES;
    Matrix->NumberOfInterchangesIsOdd = NO;
    Matrix->Partitioned = NO;
    Matrix->RowsLinked = NO;
    Matrix->InternalVectorsAllocated = NO;
    Matrix->SingularCol = 0;
    Matrix->SingularRow = 0;
    Matrix->Size = Size;
    Matrix->AllocatedSize = AllocatedSize;
    Matrix->ExtSize = Size;
    Matrix->AllocatedExtSize = AllocatedSize;
    Matrix->CurrentSize = 0;
    Matrix->ExtToIntColMap = NULL;
    Matrix->ExtToIntRowMap = NULL;
    Matrix->IntToExtColMap = NULL;
    Matrix->IntToExtRowMap = NULL;
    Matrix->MarkowitzRow = NULL;
    Matrix->MarkowitzCol = NULL;
    Matrix->MarkowitzProd = NULL;
    Matrix->DoCmplxDirect = NULL;
    Matrix->DoRealDirect = NULL;
    Matrix->Intermediate = NULL;
    Matrix->RelThreshold = DEFAULT_THRESHOLD;
    Matrix->AbsThreshold = 0.0;

    Matrix->TopOfAllocationList = NULL;
    Matrix->RecordsRemaining = 0;
    Matrix->ElementsRemaining = 0;
    Matrix->FillinsRemaining = 0;

    RecordAllocation( Matrix, (void *)Matrix );
    if (Matrix->Error == spNO_MEMORY) goto MemoryError;

/* Take out the trash. */
    Matrix->TrashCan.Real = 0.0;
    Matrix->TrashCan.Imag = 0.0;
    Matrix->TrashCan.Row = 0;
    Matrix->TrashCan.Col = 0;
    Matrix->TrashCan.NextInRow = NULL;
    Matrix->TrashCan.NextInCol = NULL;
    Matrix->TrashCan.pInitInfo = NULL;

/* Allocate space in memory for Diag pointer vector. */
    CALLOC( Matrix->Diag, ElementPtr, SizePlusOne);
    if (Matrix->Diag == NULL)
        goto MemoryError;

/* Allocate space in memory for FirstInCol pointer vector. */
    CALLOC( Matrix->FirstInCol, ElementPtr, SizePlusOne);
    if (Matrix->FirstInCol == NULL)
        goto MemoryError;

/* Allocate space in memory for FirstInRow pointer vector. */
    CALLOC( Matrix->FirstInRow, ElementPtr, SizePlusOne);
    if (Matrix->FirstInRow == NULL)
        goto MemoryError;

/* Allocate space in memory for IntToExtColMap vector. */
    if (( Matrix->IntToExtColMap = ALLOC(int, SizePlusOne)) == NULL)
        goto MemoryError;

/* Allocate space in memory for IntToExtRowMap vector. */
    if (( Matrix->IntToExtRowMap = ALLOC(int, SizePlusOne)) == NULL)
        goto MemoryError;

/* Initialize MapIntToExt vectors. */
    for (I = 1; I <= AllocatedSize; I++)
    {   Matrix->IntToExtRowMap[I] = I;
        Matrix->IntToExtColMap[I] = I;
    }

#if TRANSLATE
/* Allocate space in memory for ExtToIntColMap vector. */
    if (( Matrix->ExtToIntColMap = ALLOC(int, SizePlusOne)) == NULL)
        goto MemoryError;

/* Allocate space in memory for ExtToIntRowMap vector. */
    if (( Matrix->ExtToIntRowMap = ALLOC(int, SizePlusOne)) == NULL)
        goto MemoryError;

/* Initialize MapExtToInt vectors. */
    for (I = 1; I <= AllocatedSize; I++)
    {   Matrix->ExtToIntColMap[I] = -1;
        Matrix->ExtToIntRowMap[I] = -1;
    }
    Matrix->ExtToIntColMap[0] = 0;
    Matrix->ExtToIntRowMap[0] = 0;
#endif

/* Allocate space for fill-ins and initial set of elements. */
    InitializeElementBlocks( Matrix, SPACE_FOR_ELEMENTS*AllocatedSize,
                                     SPACE_FOR_FILL_INS*AllocatedSize );
    if (Matrix->Error == spNO_MEMORY)
        goto MemoryError;

    return (char *)Matrix;

MemoryError:

/* Deallocate matrix and return no pointer to matrix if there is not enough
   memory. */
    *pError = spNO_MEMORY;
    spDestroy( (char *)Matrix);
    return NULL;
}









/*
 *  ELEMENT ALLOCATION
 *
 *  This routine allocates space for matrix elements. It requests large blocks
 *  of storage from the system and doles out individual elements as required.
 *  This technique, as opposed to allocating elements individually, tends to
 *  speed the allocation process.
 *
 *  >>> Returned:
 *  A pointer to an element.
 *
 *  >>> Arguments:
 *  Matrix  <input>  (MatrixPtr)
 *      Pointer to matrix.
 *
 *  >>> Local variables:
 *  pElement  (ElementPtr)
 *      A pointer to the first element in the group of elements being allocated.
 *
 *  >>> Possible errors:
 *  spNO_MEMORY
 */

ElementPtr
spcGetElement( MatrixPtr Matrix )
{
ElementPtr  pElement;

/* Begin `spcGetElement'. */

/* Allocate block of MatrixElements if necessary. */
    if (Matrix->ElementsRemaining == 0)
    {   pElement = ALLOC(struct MatrixElement, ELEMENTS_PER_ALLOCATION);
        RecordAllocation( Matrix, (void *)pElement );
        if (Matrix->Error == spNO_MEMORY) return NULL;
        Matrix->ElementsRemaining = ELEMENTS_PER_ALLOCATION;
        Matrix->NextAvailElement = pElement;
    }

/* Update Element counter and return pointer to Element. */
    Matrix->ElementsRemaining--;
    return Matrix->NextAvailElement++;
}








/*
 *  ELEMENT ALLOCATION INITIALIZATION
 *
 *  This routine allocates space for matrix fill-ins and an initial set of
 *  elements.  Besides being faster than allocating space for elements one
 *  at a time, it tends to keep the fill-ins physically close to the other
 *  matrix elements in the computer memory.  This keeps virtual memory paging
 *  to a minimum.
 *
 *  >>> Arguments:
 *  Matrix  <input>    (MatrixPtr)
 *      Pointer to the matrix.
 *  InitialNumberOfElements  <input> (int)
 *      This number is used as the size of the block of memory, in
 *      MatrixElements, reserved for elements. If more than this number of
 *      elements are generated, then more space is allocated later.
 *  NumberOfFillinsExpected  <input> (int)
 *      This number is used as the size of the block of memory, in
 *      MatrixElements, reserved for fill-ins. If more than this number of
 *      fill-ins are generated, then more space is allocated, but they may
 *      not be physically close in computer's memory.
 *
 *  >>> Local variables:
 *  pElement  (ElementPtr)
 *      A pointer to the first element in the group of elements being allocated.
 *
 *  >>> Possible errors:
 *  spNO_MEMORY
 */

static void
InitializeElementBlocks(
    MatrixPtr Matrix,
    int InitialNumberOfElements,
    int NumberOfFillinsExpected
)
{
ElementPtr  pElement;

/* Begin `InitializeElementBlocks'. */

/* Allocate block of MatrixElements for elements. */
    pElement = ALLOC(struct MatrixElement, InitialNumberOfElements);
    RecordAllocation( Matrix, (void *)pElement );
    if (Matrix->Error == spNO_MEMORY) return;
    Matrix->ElementsRemaining = InitialNumberOfElements;
    Matrix->NextAvailElement = pElement;

/* Allocate block of MatrixElements for fill-ins. */
    pElement = ALLOC(struct MatrixElement, NumberOfFillinsExpected);
    RecordAllocation( Matrix, (void *)pElement );
    if (Matrix->Error == spNO_MEMORY) return;
    Matrix->FillinsRemaining = NumberOfFillinsExpected;
    Matrix->NextAvailFillin = pElement;

/* Allocate a fill-in list structure. */
    Matrix->FirstFillinListNode = ALLOC(struct FillinListNodeStruct,1);
    RecordAllocation( Matrix, (void *)Matrix->FirstFillinListNode );
    if (Matrix->Error == spNO_MEMORY) return;
    Matrix->LastFillinListNode = Matrix->FirstFillinListNode;

    Matrix->FirstFillinListNode->pFillinList = pElement;
    Matrix->FirstFillinListNode->NumberOfFillinsInList =NumberOfFillinsExpected;
    Matrix->FirstFillinListNode->Next = NULL;

    return;
}










/*
 *  FILL-IN ALLOCATION
 *
 *  This routine allocates space for matrix fill-ins. It requests large blocks
 *  of storage from the system and doles out individual elements as required.
 *  This technique, as opposed to allocating elements individually, tends to
 *  speed the allocation process.
 *
 *  >>> Returned:
 *  A pointer to the fill-in.
 *
 *  >>> Arguments:
 *  Matrix  <input>  (MatrixPtr)
 *      Pointer to matrix.
 *
 *  >>> Possible errors:
 *  spNO_MEMORY
 */

ElementPtr
spcGetFillin( MatrixPtr Matrix )
{
struct FillinListNodeStruct *pListNode;
ElementPtr  pFillins;

/* Begin `spcGetFillin'. */

    if (Matrix->FillinsRemaining == 0)
    {   pListNode = Matrix->LastFillinListNode;

/* First see if there are any stripped fill-ins left. */
        if (pListNode->Next != NULL)
        {   Matrix->LastFillinListNode = pListNode = pListNode->Next;
            Matrix->FillinsRemaining = pListNode->NumberOfFillinsInList;
            Matrix->NextAvailFillin = pListNode->pFillinList;
        }
        else
        {
/* Allocate block of fill-ins. */
            pFillins = ALLOC(struct MatrixElement, ELEMENTS_PER_ALLOCATION);
            RecordAllocation( Matrix, (void *)pFillins );
            if (Matrix->Error == spNO_MEMORY) return NULL;
            Matrix->FillinsRemaining = ELEMENTS_PER_ALLOCATION;
            Matrix->NextAvailFillin = pFillins;

/* Allocate a fill-in list structure. */
            pListNode->Next = ALLOC(struct FillinListNodeStruct,1);
            RecordAllocation( Matrix, (void *)pListNode->Next );
            if (Matrix->Error == spNO_MEMORY) return NULL;
            Matrix->LastFillinListNode = pListNode = pListNode->Next;

            pListNode->pFillinList = pFillins;
            pListNode->NumberOfFillinsInList = ELEMENTS_PER_ALLOCATION;
            pListNode->Next = NULL;
        }
    }

/* Update Fill-in counter and return pointer to Fill-in. */
    Matrix->FillinsRemaining--;
    return Matrix->NextAvailFillin++;
}

/*
 *  RECORD A MEMORY ALLOCATION
 *
 *  This routine is used to record all memory allocations so that the memory
 *  can be freed later.
 *
 *  >>> Arguments:
 *  Matrix  <input>    (MatrixPtr)
 *      Pointer to the matrix.
 *  AllocatedPtr  <input>  (void *)
 *      The pointer returned by malloc.  These pointers are saved in
 *      a list so that they can be easily freed.
 *
 *  >>> Possible errors:
 *  spNO_MEMORY
 */

static void
RecordAllocation(
    MatrixPtr Matrix,
    void *AllocatedPtr
)
{
/* Begin `RecordAllocation'. */
/*
 * If Allocated pointer is NULL, assume that malloc returned a NULL pointer,
 * which indicates a spNO_MEMORY error.
 */
    if (AllocatedPtr == NULL)
    {   Matrix->Error = spNO_MEMORY;
        return;
    }

/* Allocate block of MatrixElements if necessary. */
    if (Matrix->RecordsRemaining == 0)
    {   AllocateBlockOfAllocationList( Matrix );
        if (Matrix->Error == spNO_MEMORY)
        {   FREE(AllocatedPtr);
            return;
        }
    }

/* Add Allocated pointer to Allocation List. */
    (++Matrix->TopOfAllocationList)->AllocatedPtr = AllocatedPtr;
    Matrix->RecordsRemaining--;
    return;

}








/*
 *  ADD A BLOCK OF SLOTS TO ALLOCATION LIST     
 *
 *  This routine increases the size of the allocation list.
 *
 *  >>> Arguments:
 *  Matrix  <input>    (MatrixPtr)
 *      Pointer to the matrix.
 *
 *  >>> Local variables:
 *  ListPtr  (AllocationListPtr)
 *      Pointer to the list that contains the pointers to segments of memory
 *      that were allocated by the operating system for the current matrix.
 *
 *  >>> Possible errors:
 *  spNO_MEMORY
 */

static void
AllocateBlockOfAllocationList( MatrixPtr Matrix )
{
register  int  I;
register  AllocationListPtr  ListPtr;

/* Begin `AllocateBlockOfAllocationList'. */
/* Allocate block of records for allocation list. */
    ListPtr = ALLOC(struct AllocationRecord, (ELEMENTS_PER_ALLOCATION+1));
    if (ListPtr == NULL)
    {   Matrix->Error = spNO_MEMORY;
        return;
    }

/* String entries of allocation list into singly linked list.  List is linked
   such that any record points to the one before it. */

    ListPtr->NextRecord = Matrix->TopOfAllocationList;
    Matrix->TopOfAllocationList = ListPtr;
    ListPtr += ELEMENTS_PER_ALLOCATION;
    for (I = ELEMENTS_PER_ALLOCATION; I > 0; I--)
    {    ListPtr->NextRecord = ListPtr - 1;
         ListPtr--;
    }

/* Record allocation of space for allocation list on allocation list. */
    Matrix->TopOfAllocationList->AllocatedPtr = (void *)ListPtr;
    Matrix->RecordsRemaining = ELEMENTS_PER_ALLOCATION;

    return;
}








/*!
 *  Destroys a matrix and frees all memory associated with it.
 *
 *  \param eMatrix
 *      Pointer to the matrix frame which is to be destroyed.
 */
/*  >>> Local variables:
 *  ListPtr  (AllocationListPtr)
 *      Pointer into the linked list of pointers to allocated data structures.
 *      Points to pointer to structure to be freed.
 *  NextListPtr  (AllocationListPtr)
 *      Pointer into the linked list of pointers to allocated data structures.
 *      Points to the next pointer to structure to be freed.  This is needed
 *      because the data structure to be freed could include the current node
 *      in the allocation list.
 */

void
spDestroy( spMatrix eMatrix )
{
MatrixPtr Matrix = (MatrixPtr)eMatrix;
register  AllocationListPtr  ListPtr, NextListPtr;

/* Begin `spDestroy'. */
    ASSERT_IS_SPARSE( Matrix );

/* Deallocate the vectors that are located in the matrix frame. */
    FREE( Matrix->IntToExtColMap );
    FREE( Matrix->IntToExtRowMap );
    FREE( Matrix->ExtToIntColMap );
    FREE( Matrix->ExtToIntRowMap );
    FREE( Matrix->Diag );
    FREE( Matrix->FirstInRow );
    FREE( Matrix->FirstInCol );
    FREE( Matrix->MarkowitzRow );
    FREE( Matrix->MarkowitzCol );
    FREE( Matrix->MarkowitzProd );
    FREE( Matrix->DoCmplxDirect );
    FREE( Matrix->DoRealDirect );
    FREE( Matrix->Intermediate );

/* Sequentially step through the list of allocated pointers freeing pointers
 * along the way. */
    ListPtr = Matrix->TopOfAllocationList;
    while (ListPtr != NULL)
    {   NextListPtr = ListPtr->NextRecord;
        Free( ListPtr->AllocatedPtr );
        ListPtr = NextListPtr;
    }
    return;
}







/*!
 *  This function returns the error status of the given matrix.
 *
 *  \return
 *      The error status of the given matrix.
 *
 *  \param eMatrix
 *      The pointer to the matrix for which the error status is desired.
 */

spError
spErrorState( spMatrix eMatrix )
{
/* Begin `spErrorState'. */

    if (eMatrix != NULL)
    {   ASSERT_IS_SPARSE( (MatrixPtr)eMatrix );
        return ((MatrixPtr)eMatrix)->Error;
    }
    else return spNO_MEMORY;   /* This error may actually be spPANIC,
                                * no way to tell. */
}









/*!
 *  This function returns the row and column number where the matrix was
 *  detected as singular (if pivoting was allowed on the last factorization)
 *  or where a zero was detected on the diagonal (if pivoting was not
 *  allowed on the last factorization). Pivoting is performed only in
 *  spOrderAndFactor().
 *
 *  \param eMatrix
 *      The matrix for which the error status is desired.
 *  \param pRow
 *      The row number.
 *  \param pCol
 *      The column number.
 */

void
spWhereSingular(
    spMatrix eMatrix,
    int *pRow,
    int *pCol
)
{
MatrixPtr Matrix = (MatrixPtr)eMatrix;

/* Begin `spWhereSingular'. */
    ASSERT_IS_SPARSE( Matrix );

    if (Matrix->Error == spSINGULAR OR Matrix->Error == spZERO_DIAG)
    {   *pRow = Matrix->SingularRow;
        *pCol = Matrix->SingularCol;
    }
    else *pRow = *pCol = 0;
    return;
}






/*!
 *  Returns the size of the matrix.  Either the internal or external size of
 *  the matrix is returned.
 *
 *  \param eMatrix
 *      Pointer to matrix.
 *  \param External
 *      If \a External is set true, the external size , i.e., the value of the
 *      largest external row or column number encountered is returned.
 *      Otherwise the true size of the matrix is returned.  These two sizes
 *      may differ if the \a TRANSLATE option is set true.
 */

int
spGetSize(
    spMatrix eMatrix,
    int External
)
{
MatrixPtr Matrix = (MatrixPtr)eMatrix;

/* Begin `spGetSize'. */
    ASSERT_IS_SPARSE( Matrix );

#if TRANSLATE
    if (External)
        return Matrix->ExtSize;
    else
        return Matrix->Size;
#else
    return Matrix->Size;
#endif
}








/*!
 *  Forces matrix to be real.
 *
 *  \param eMatrix
 *      Pointer to matrix.
 */

void
spSetReal( spMatrix eMatrix )
{
/* Begin `spSetReal'. */

    ASSERT_IS_SPARSE( (MatrixPtr)eMatrix );
    ((MatrixPtr)eMatrix)->Complex = NO;
    return;
}


/*!
 *  Forces matrix to be complex.
 *
 *  \param eMatrix
 *      Pointer to matrix.
 */

void
spSetComplex( spMatrix eMatrix )
{
/* Begin `spSetComplex'. */

    ASSERT_IS_SPARSE( (MatrixPtr)eMatrix );
    ((MatrixPtr)eMatrix)->Complex = YES;
    return;
}









/*!
 *  This function returns the number of fill-ins that currently exists in a matrix.
 *
 *  \param eMatrix
 *      Pointer to matrix.
 */

int
spFillinCount( spMatrix eMatrix )
{
/* Begin `spFillinCount'. */

    ASSERT_IS_SPARSE( (MatrixPtr)eMatrix );
    return ((MatrixPtr)eMatrix)->Fillins;
}


/*!
 *  This function returns the total number of elements (including fill-ins) that currently exists in a matrix.
 *
 *  \param eMatrix
 *      Pointer to matrix.
 */

int
spElementCount( spMatrix eMatrix )
{
/* Begin `spElementCount'. */

    ASSERT_IS_SPARSE( (MatrixPtr)eMatrix );
    return ((MatrixPtr)eMatrix)->Elements;
}

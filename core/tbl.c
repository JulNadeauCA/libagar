/*
 * Copyright (c) 2009-2018 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Implementation of a generic hash table of AG_Variable(3) items.
 */

#include <agar/core/core.h>

/* Import inlinables */
#undef AG_INLINE_HEADER
#include <agar/core/inline_tbl.h>

/* Allocate and initialize a table. */
AG_Tbl *
AG_TblNew(Uint nBuckets, Uint flags)
{
	AG_Tbl *t;

	t = Malloc(sizeof(AG_Tbl));
	AG_TblInit(t, nBuckets, flags);
	return (t);
}

/* Initialize a table structure. */
void
AG_TblInit(AG_Tbl *tbl, Uint nBuckets, Uint flags)
{
	Uint i;

	tbl->flags = flags;
	tbl->nBuckets = nBuckets;
	tbl->buckets = Malloc(nBuckets*sizeof(AG_TblBucket));

	for (i = 0; i < nBuckets; i++) {
		AG_TblBucket *buck = &tbl->buckets[i];
		buck->keys = NULL;
		buck->ents = NULL;
		buck->nEnts = 0;
	}
}

/* Release the resources allocated by a table. */
void
AG_TblDestroy(AG_Tbl *t)
{
	Uint i, j;

	for (i = 0; i < t->nBuckets; i++) {
		AG_TblBucket *buck = &t->buckets[i];

		for (j = 0; j < buck->nEnts; j++) {
			free(buck->keys[j]);
			AG_FreeVariable(&buck->ents[j]);
		}
		free(buck->keys);
		free(buck->ents);
	}
	free(t->buckets);
}

/* Look up a named table entry. */
AG_Variable *
AG_TblLookupHash(AG_Tbl *tbl, Uint h, const char *key)
{
	AG_TblBucket *buck = &tbl->buckets[h];
	Uint i;

	for (i = 0; i < buck->nEnts; i++) {
		if (strcmp(buck->keys[i], key) == 0)
			break;
	}
	if (i == buck->nEnts) {
		return (NULL);
	}
	return (&buck->ents[i]);
}

/* Evaluate whether a table entry exists. */
int
AG_TblExistsHash(AG_Tbl *tbl, Uint h, const char *key)
{
	AG_TblBucket *buck = &tbl->buckets[h];
	Uint i;
	
	for (i = 0; i < buck->nEnts; i++) {
		if (strcmp(buck->keys[i], key) == 0)
			return (1);
	}
	return (0);
}

/*
 * Insert a new table entry.
 * The Variable contents are duplicated.
 */
int
AG_TblInsertHash(AG_Tbl *tbl, Uint h, const char *key, const AG_Variable *V)
{
	AG_TblBucket *buck = &tbl->buckets[h];
	AG_Variable *entsNew;
	char **keysNew;
	Uint i;

	for (i = 0; i < buck->nEnts; i++) {
		if (strcmp(buck->keys[i], key) == 0)
			break;
	}
	if (!(tbl->flags & AG_TBL_DUPLICATES) && i < buck->nEnts) {
		AG_SetErrorV("E27", "Table entry exists");
		return (-1);
	}

	if ((entsNew = TryRealloc(buck->ents,
	    (buck->nEnts+1)*sizeof(AG_Variable))) == NULL) {
		return (-1);
	}
	if ((keysNew = TryRealloc(buck->keys,
	    (buck->nEnts+1)*sizeof(char *))) == NULL) {
		free(entsNew);
		return (-1);
	}
	buck->ents = entsNew;
	buck->keys = keysNew;
	buck->keys[buck->nEnts] = Strdup(key);
	AG_CopyVariable(&buck->ents[buck->nEnts], V);
	buck->nEnts++;
	return (0);
}

/* Remove a named table entry. */
int
AG_TblDeleteHash(AG_Tbl *tbl, Uint h, const char *key)
{
	AG_TblBucket *buck = &tbl->buckets[h];
	Uint i;

	for (i = 0; i < buck->nEnts; i++) {
		if (strcmp(buck->keys[i], key) == 0)
			break;
	}
	if (i == buck->nEnts) {
		AG_SetErrorV("E28", "No such table entry");
		return (-1);
	}

	free(buck->keys[i]);
	AG_FreeVariable(&buck->ents[i]);

	if (i < buck->nEnts-1) {
		memmove(&buck->ents[i], &buck->ents[i+1],
		    (buck->nEnts - i - 1)*sizeof(AG_Variable));
		memmove(&buck->keys[i], &buck->keys[i+1],
		    (buck->nEnts - i - 1)*sizeof(char *));
	}
	buck->nEnts--;
	return (0);
}

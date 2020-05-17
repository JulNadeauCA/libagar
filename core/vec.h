/** 
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _AGAR_CORE_VEC_H_
#define _AGAR_CORE_VEC_H_
#include <agar/core/begin.h>

/**
 * Copyright (c) 2014 rxi
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define AG_VEC_VERSION "0.2.1"

#define AG_VEC_UNPACK_(v)\
  (char**)&(v)->data, &(v)->length, &(v)->capacity, sizeof(*(v)->data)

#define AG_VEC_HEAD(T)\
  struct { T *data; int length, capacity; }

#define AG_VEC_INIT(v)\
  memset((v), 0, sizeof(*(v)))

#define AG_VEC_DESTROY(v)\
  ( free((v)->data),\
    AG_VEC_INIT(v) ) 

#define AG_VEC_PUSH(v, val)\
  ( ag_vec_expand_(AG_VEC_UNPACK_(v)) ? -1 :\
    ((v)->data[(v)->length++] = (val), 0) )

#define AG_VEC_POP(v)\
  (v)->data[--(v)->length]

#define AG_VEC_SPLICE(v, start, count)\
  ( ag_vec_splice_(AG_VEC_UNPACK_(v), start, count),\
    (v)->length -= (count) )

#if 0
#define AG_VEC_SWAPSPLICE(v, start, count)\
  ( ag_vec_swapsplice_(AG_VEC_UNPACK_(v), start, count),\
    (v)->length -= (count) )
#endif

#define AG_VEC_INSERT(v, idx, val)\
  ( ag_vec_insert_(AG_VEC_UNPACK_(v), idx) ? -1 :\
    ((v)->data[idx] = (val), 0), (v)->length++, 0 )
    
#define AG_VEC_SORT(v, fn)\
  qsort((v)->data, (v)->length, sizeof(*(v)->data), fn)

#if 0
#define AG_VEC_SWAP(v, idx1, idx2)\
  ag_vec_swap_(AG_VEC_UNPACK_(v), idx1, idx2)
#endif

#define AG_VEC_TRUNCATE(v, len)\
  ((v)->length = (len) < (v)->length ? (len) : (v)->length)

#define AG_VEC_CLEAR(v)\
  ((v)->length = 0)

#define AG_VEC_FIRST(v)\
  (v)->data[0]

#define AG_VEC_LAST(v)\
  (v)->data[(v)->length - 1]

#define AG_VEC_RESERVE(v, n)\
  ag_vec_reserve_(AG_VEC_UNPACK_(v), n)

#if 0
#define AG_VEC_COMPACT(v)\
  ag_vec_compact_(AG_VEC_UNPACK_(v))
#endif

#define AG_VEC_PUSHARR(v, arr, count)\
  do {\
    int i__, n__ = (count);\
    if (ag_vec_reserve_po2_(AG_VEC_UNPACK_(v), (v)->length + n__) != 0) break;\
    for (i__ = 0; i__ < n__; i__++) {\
      (v)->data[(v)->length++] = (arr)[i__];\
    }\
  } while (0)

#define AG_VEC_EXTEND(v, v2)\
  AG_VEC_PUSHARR((v), (v2)->data, (v2)->length)

#define AG_VEC_FIND(v, val, idx)\
  do {\
    for ((idx) = 0; (idx) < (v)->length; (idx)++) {\
      if ((v)->data[(idx)] == (val)) break;\
    }\
    if ((idx) == (v)->length) (idx) = -1;\
  } while (0)

#define AG_VEC_REMOVE(v, val)\
  do {\
    int idx__;\
    ag_VEC_FIND(v, val, idx__);\
    if (idx__ != -1) AG_VEC_SPLICE(v, idx__, 1);\
  } while (0)

#if 0
#define AG_VEC_REVERSE(v)\
  do {\
    int i__ = (v)->length / 2;\
    while (i__--) {\
      AG_VEC_SWAP((v), i__, (v)->length - (i__ + 1));\
    }\
  } while (0)
#endif

#define AG_VEC_FOREACH(v, var, iter)\
  if  ( (v)->length > 0 )\
  for ( (iter) = 0;\
        (iter) < (v)->length && (((var) = (v)->data[(iter)]), 1);\
        ++(iter))

#define AG_VEC_FOREACH_REVERSE(v, var, iter)\
  if  ( (v)->length > 0 )\
  for ( (iter) = (v)->length - 1;\
        (iter) >= 0 && (((var) = (v)->data[(iter)]), 1);\
        --(iter))

#define AG_VEC_FOREACH_PTR(v, var, iter)\
  if  ( (v)->length > 0 )\
  for ( (iter) = 0;\
        (iter) < (v)->length && (((var) = &(v)->data[(iter)]), 1);\
        ++(iter))

#define AG_VEC_FOREACH_PTR_REVERSE(v, var, iter)\
  if  ( (v)->length > 0 )\
  for ( (iter) = (v)->length - 1;\
        (iter) >= 0 && (((var) = &(v)->data[(iter)]), 1);\
        --(iter))

__BEGIN_DECLS

int ag_vec_expand_(char *_Nonnull *_Nullable, int *_Nonnull, int *_Nonnull, int);
int ag_vec_reserve_(char *_Nonnull *_Nullable, int *_Nonnull, int *_Nonnull, int, int);
int ag_vec_reserve_po2_(char *_Nonnull *_Nullable, int *_Nonnull, int *_Nonnull, int, int);
int ag_vec_compact_(char *_Nonnull *_Nullable, int *_Nonnull, int *_Nonnull, int);
int ag_vec_insert_(char *_Nonnull *_Nullable, int *_Nonnull, int *_Nonnull, int, int);
void ag_vec_splice_(char *_Nonnull *_Nullable, int *_Nonnull, int *_Nonnull, int, int, int);
#if 0
void ag_vec_swapsplice_(char *_Nonnull *_Nullable, int *_Nonnull, int *_Nonnull, int, int, int);
void ag_vec_swap_(char *_Nonnull *_Nullable, int *_Nonnull, int *_Nonnull, int, int,int);
#endif
__END_DECLS

typedef AG_VEC_HEAD(void*)  AG_VecVoid;
typedef AG_VEC_HEAD(char*)  AG_VecString;
typedef AG_VEC_HEAD(int)    AG_VecInt;
typedef AG_VEC_HEAD(char)   AG_VecChar;
#ifdef AG_HAVE_FLOAT
typedef AG_VEC_HEAD(float)  AG_VecFloat;
typedef AG_VEC_HEAD(double) AG_VecDouble;
#endif

#include <agar/core/close.h>
#endif /* _AGAR_CORE_VEC_H_ */

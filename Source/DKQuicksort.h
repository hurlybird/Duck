// =======================================================================================
//
// DKQuicksort.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_QUICK_SORT_H_
#define _DK_QUICK_SORT_H_

#ifdef __cplusplus
extern "C"
{
#endif


// Note: It'd be nice to use qsort_s (C11), but that doesn't seem to be commonly available
DK_API void DKQuicksort( void * ptr, size_t count, size_t size, int (*cmp)(const void *, const void *, void *), void * context );


// Specialized quicksort for objects
DK_API void DKQuicksortObjects( DKObjectRef objects[], size_t count, DKCompareFunction cmp );


#ifdef __cplusplus
}
#endif

#endif // _DK_QUICK_SORT_H_

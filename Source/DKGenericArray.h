// =======================================================================================
//
// DKGenericArray.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_GENERIC_ARRAY_H_
#define _DK_GENERIC_ARRAY_H_

#ifdef __cplusplus
extern "C"
{
#endif


// DKGenericArray DOES NOT guarantee that its elements are stored in a contiguous C array,
// though in the current implementation that happens to be true. Change this definition
// if that changes to highlight code that relies on this behaviour.
#define DKGenericArrayHasContiguousElements 1

// Possible performance improvements:
//
// - Keep some empty space at the beginning of the memory buffer for faster insert/remove
//   at the start of the array.
//
// - Use a ringbuffer for storage to both improve insert/remove at the start of the array
//   and make full use of the current size before reallocating.

typedef struct
{
    uint8_t * elements;
    DKIndex elementSize;
    DKIndex length;
    DKIndex maxLength;

} DKGenericArray;


DK_API void DKGenericArrayInit( DKGenericArray * array, DKIndex elementSize );

DK_API void DKGenericArrayInitWithExternalStorage( DKGenericArray * array, const void * elements, DKIndex elementSize, DKIndex length );
DK_API int  DKGenericArrayHasExternalStorage( DKGenericArray * array );

DK_API void DKGenericArrayFinalize( DKGenericArray * array );

DK_API void DKGenericArrayReserve( DKGenericArray * array, DKIndex length );

#define DKGenericArrayGetLength( array ) ((array)->length)
DK_API void DKGenericArraySetLength( DKGenericArray * array, DKIndex length );

DK_API void * DKGenericArrayGetPointerToElementAtIndex( DKGenericArray * array, DKIndex index );

#define DKGenericArrayElementAtIndex( array, index, type )                              \
    *((type *)DKGenericArrayGetPointerToElementAtIndex( (array), (index) ))

#define DKGenericArrayLastElement( array, type )                                        \
    *((type *)DKGenericArrayGetPointerToElementAtIndex( (array), DKGenericArrayGetLength( array ) - 1 ))

DK_API void DKGenericArrayReplaceElements( DKGenericArray * array, DKRange range, const void * elements, DKIndex length );
DK_API void DKGenericArrayAppendElements( DKGenericArray * array, const void * elements, DKIndex length );

DK_API void DKGenericArraySort( DKGenericArray * array, int (*cmp)(const void *, const void *) );
DK_API void DKGenericArraySortEx( DKGenericArray * array, int (*cmp)(const void *, const void *, void *), void * context );
DK_API void DKGenericArraySortObjects( DKGenericArray * array, DKCompareFunction cmp );

DK_API void DKGenericArrayReverse( DKGenericArray * array );
DK_API void DKGenericArrayShuffle( DKGenericArray * array );

#define DKGenericArrayPush( array, elem )   DKGenericArrayAppendElements( (array), (elem), 1 )
#define DKGenericArrayPop( array )          DKGenericArrayReplaceElements( (array), DKRangeMake( (array)->length - 1, 1 ), NULL, 0 )
#define DKGenericArrayPeek( array, index )  DKGenericArrayGetPointerToElementAtIndex( (array), DKGenericArrayGetLength( array ) - 1 - (index) )
#define DKGenericArrayTop( array )          DKGenericArrayGetPointerToElementAtIndex( (array), DKGenericArrayGetLength( array ) - 1 )


#ifdef __cplusplus
}
#endif

#endif // _DK_GENERIC_ARRAY_H_

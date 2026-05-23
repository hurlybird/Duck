// =======================================================================================
//
// DKByteArray.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_BYTE_ARRAY_H_
#define _DK_BYTE_ARRAY_H_

#ifdef __cplusplus
extern "C"
{
#endif


// DKByteArray MUST guarantee that its storage is a contiguous C array of bytes (DKData
// and DKString rely on this behaviour).

// Note: DKByteArray internally stores four '\0' bytes (i.e. a UTF32 NULL) at data[length]
// to make storing strings safer, regardless of encoding. The NULLs aren't included in the
// length or maxLength of the array.

typedef struct
{
    uint8_t * bytes;
    DKIndex length;
    DKIndex maxLength;

} DKByteArray;


DK_API void DKByteArrayInit( DKByteArray * array );

DK_API void DKByteArrayInitWithExternalStorage( DKByteArray * array, const uint8_t bytes[], DKIndex length );
DK_API bool DKByteArrayHasExternalStorage( DKByteArray * array );

DK_API void DKByteArrayFinalize( DKByteArray * array );

DK_API void DKByteArrayReserve( DKByteArray * array, DKIndex length );

#define DKByteArrayGetLength( array )       ((array)->length)
#define DKByteArrayGetBytePtr( array, i )   ((void *)&((array)->bytes[i]))

DK_API void DKByteArraySetLength( DKByteArray * array, DKIndex length );

DK_API void DKByteArrayReplaceBytes( DKByteArray * array, DKRange range, const uint8_t bytes[], DKIndex length );
DK_API void DKByteArrayAppendBytes( DKByteArray * array, const uint8_t bytes[], DKIndex length );

DK_API DKIndex DKByteArrayAlignLength( DKByteArray * array, DKIndex byteAlignment );


#ifdef __cplusplus
}
#endif

#endif // _DK_BYTE_ARRAY_H_

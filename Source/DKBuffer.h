// =======================================================================================
//
// DKBuffer.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_BUFFER_H_
#define _DK_BUFFER_H_

#ifdef __cplusplus
extern "C"
{
#endif


DK_API DKDeclareInterfaceSelector( Buffer );


typedef DKObjectRef DKBufferRef;


typedef DKIndex      (*DKBufferGetLengthMethod)( DKObjectRef _self );
typedef const void * (*DKBufferGetBytePtrMethod)( DKObjectRef _self, DKIndex index );

typedef void         (*DKBufferSetLengthMethod)( DKObjectRef _self, DKIndex length );
typedef void *       (*DKBufferGetMutableBytePtrMethod)( DKObjectRef _self, DKIndex index );


struct DKBufferInterface
{
    const DKInterface _interface;
    
    DKBufferGetLengthMethod         getLength;
    DKBufferGetBytePtrMethod        getBytePtr;

    DKBufferSetLengthMethod         setLength;
    DKBufferGetMutableBytePtrMethod getMutableBytePtr;
};

typedef const struct DKBufferInterface * DKBufferInterfaceRef;


// Wrappers
DK_API DKIndex      DKBufferGetLength( DKObjectRef _self );
DK_API const void * DKBufferGetBytePtr( DKObjectRef _self, DKIndex index );

DK_API void         DKBufferSetLength( DKObjectRef _self, DKIndex length );
DK_API void *       DKBufferGetMutableBytePtr( DKObjectRef _self, DKIndex index );



#ifdef __cplusplus
}
#endif

#endif // _DK_BUFFER_H_


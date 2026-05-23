// =======================================================================================
//
// DKArray.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_ARRAY_H_
#define _DK_ARRAY_H_

#ifdef __cplusplus
extern "C"
{
#endif


// typedef struct DKArray * DKArrayRef; -- Declared in DKPlatform.h
// typedef struct DKArray * DKMutableArrayRef; -- Declared in DKPlatform.h


DK_API DKClassRef  DKArrayClass( void );
DK_API DKClassRef  DKMutableArrayClass( void );

#define     DKEmptyArray()          DKAutorelease( DKNew( DKArrayClass() ) )
#define     DKMutableArray()        DKAutorelease( DKNew( DKMutableArrayClass() ) )

#define     DKArrayWithCArray( objects, count )  DKArrayInitWithObjects( DKAlloc( DKArrayClass() ), objects, count )
#define     DKArrayWithCollection( collection )  DKArrayInitWithCollection( DKAlloc( DKArrayClass() ), collection )

#define     DKNewMutableArray()     DKNew( DKMutableArrayClass() )

DK_API DKObjectRef DKArrayInitWithVAObjects( DKArrayRef _self, va_list objects );
DK_API DKObjectRef DKArrayInitWithCArray( DKArrayRef _self, DKObjectRef objects[], DKIndex count );
DK_API DKObjectRef DKArrayInitWithCollection( DKArrayRef _self, DKObjectRef collection );
DK_API DKObjectRef DKArrayInitWithCArrayNoCopy( DKArrayRef _self, DKObjectRef objects[], DKIndex count );

DK_API DKArrayRef  DKArrayCopy( DKArrayRef _self );
DK_API DKMutableArrayRef DKArrayMutableCopy( DKArrayRef _self );

DK_API DKIndex     DKArrayGetCount( DKArrayRef _self );

DK_API DKObjectRef DKArrayGetObjectAtIndex( DKArrayRef _self, DKIndex index );
DK_API DKIndex     DKArrayGetObjectsInRange( DKArrayRef _self, DKRange range, DKObjectRef objects[] );

DK_API void        DKArrayReserve( DKMutableArrayRef _self, size_t reserve );
DK_API void        DKArrayAppendObject( DKMutableArrayRef _self, DKObjectRef object );
DK_API void        DKArrayAppendCArray( DKMutableArrayRef _self, DKObjectRef objects[], DKIndex count );
DK_API void        DKArrayAppendCollection( DKMutableArrayRef _self, DKObjectRef srcCollection );

DK_API void        DKArrayReplaceRangeWithCArray( DKMutableArrayRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
DK_API void        DKArrayReplaceRangeWithCollection( DKMutableArrayRef _self, DKRange range, DKObjectRef collection );

DK_API void        DKArraySort( DKMutableArrayRef _self, DKCompareFunction cmp );
DK_API void        DKArrayReverse( DKMutableArrayRef _self );
DK_API void        DKArrayShuffle( DKMutableArrayRef _self );

DK_API int         DKArrayApplyFunction( DKArrayRef _self, DKApplierFunction callback, void * context );



#ifdef __cplusplus
}
#endif

#endif // _DK_ARRAY_H_











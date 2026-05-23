// =======================================================================================
//
// DKStruct.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_STRUCT_H_
#define _DK_STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DKStruct * DKStructRef;


DK_API DKClassRef  DKStructClass( void );

#define            DKSemantic( type )                      DKSTR( #type )

#define            DKStruct( semantic, bytes, size )       DKAutorelease( DKStructInit( DKAlloc( DKStructClass() ), semantic, bytes, size ) )
#define            DKStructWithType( ptr, type )           DKAutorelease( DKStructInit( DKAlloc( DKStructClass() ), DKSTR( #type ), ptr, sizeof(type) ) )

#define            DKNewStruct( semantic, bytes, size )    DKStructInit( DKAlloc( DKStructClass() ), semantic, bytes, size )
#define            DKNewStructWithType( ptr, type )        DKStructInit( DKAlloc( DKStructClass() ), DKSTR( #type ), ptr, sizeof(type) )

DK_API DKStructRef DKStructInit( DKObjectRef _self, DKStringRef semantic, const void * bytes, size_t size );

DK_API bool        DKStructEqual( DKStructRef _self, DKStructRef other );
DK_API int         DKStructCompare( DKStructRef _self, DKStructRef other );
DK_API DKHashCode  DKStructHash( DKStructRef _self );

DK_API DKStringRef DKStructGetSemantic( DKStructRef _self );
DK_API size_t      DKStructGetSize( DKStructRef _self );
DK_API const void * DKStructGetValuePtr( DKStructRef _self );
DK_API size_t      DKStructGetValue( DKStructRef _self, DKStringRef semantic, void * bytes, size_t size );

#define            DKStructGetValueAsType( _self, dst, type ) DKStructGetValue( _self, DKSTR( #type ), dst, sizeof(type) )


#ifdef __cplusplus
}
#endif

#endif // _DK_STRUCT_H_

// =======================================================================================
//
// DKConversion.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_CONVERSION_H_
#define _DK_CONVERSION_H_

#ifdef __cplusplus
extern "C"
{
#endif


DK_API DKDeclareInterfaceSelector( Conversion );


typedef DKStringRef (*DKGetStringMethod)( DKObjectRef _self );
typedef bool        (*DKGetBoolMethod)( DKObjectRef _self );
typedef int32_t     (*DKGetInt32Method)( DKObjectRef _self );
typedef int64_t     (*DKGetInt64Method)( DKObjectRef _self );
typedef float       (*DKGetFloatMethod)( DKObjectRef _self );
typedef double      (*DKGetDoubleMethod)( DKObjectRef _self );


struct DKConversionInterface
{
    const DKInterface _interface;
    
    DKGetStringMethod   getString;
    DKGetBoolMethod     getBool;
    DKGetInt32Method    getInt32;
    DKGetInt64Method    getInt64;
    DKGetFloatMethod    getFloat;
    DKGetDoubleMethod   getDouble;
};

typedef const struct DKConversionInterface * DKConversionInterfaceRef;


DK_API DKStringRef DKGetString( DKObjectRef _self );
DK_API bool        DKGetBool( DKObjectRef _self );
DK_API int32_t     DKGetInt32( DKObjectRef _self );
DK_API int64_t     DKGetInt64( DKObjectRef _self );
DK_API float       DKGetFloat( DKObjectRef _self );
DK_API double      DKGetDouble( DKObjectRef _self );


#define DKGetInt( _self )       DKGetInt32( _self )
#define DKGetLongLong( _self )  DKGetInt64( _self )



#ifdef __cplusplus
}
#endif

#endif // _DK_CONVERSION_H_




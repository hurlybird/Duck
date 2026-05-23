// =======================================================================================
//
// DKDescription.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_DESCRIPTION_H_
#define _DK_DESCRIPTION_H_

#ifdef __cplusplus
extern "C"
{
#endif


DK_API DKDeclareInterfaceSelector( Description );


typedef DKStringRef (*DKGetDescriptionMethod)( DKObjectRef _self );
typedef size_t      (*DKGetSizeInBytesMethod)( DKObjectRef _self );

struct DKDescriptionInterface
{
    const DKInterface _interface;
    
    DKGetDescriptionMethod getDescription;
    DKGetSizeInBytesMethod getSizeInBytes;
};

typedef const struct DKDescriptionInterface * DKDescriptionInterfaceRef;


// Default description interface. This is used by the root classes so it's defined in
// DKRuntime.c.
DK_API DKInterfaceRef DKDefaultDescription( void );


// A default copyDescription method that returns the class name
DK_API DKStringRef DKDefaultGetDescription( DKObjectRef _self );
DK_API size_t      DKDefaultGetSizeInBytes( DKObjectRef _self );


DK_API DKStringRef DKGetDescription( DKObjectRef _self );
DK_API size_t      DKGetSizeInBytes( DKObjectRef _self );



#ifdef __cplusplus
}
#endif

#endif // _DK_DESCRIPTION_H_




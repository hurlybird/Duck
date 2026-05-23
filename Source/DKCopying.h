// =======================================================================================
//
// DKCopying.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_COPYING_H_
#define __Duck__DKCopying__

#ifdef __cplusplus
extern "C"
{
#endif


DK_API DKDeclareInterfaceSelector( Copying );


enum
{
    DKDeepCopyMutableContainers =   (1 << 0),
    DKDeepCopyMutableObjects =      (1 << 1)
};


typedef DKObjectRef (*DKCopyMethod)( DKObjectRef );
typedef DKObjectRef (*DKDeepCopyMethod)( DKObjectRef, int options );

struct DKCopyingInterface
{
    const DKInterface _interface;

    DKCopyMethod        copy;
    DKCopyMethod        mutableCopy;
    DKDeepCopyMethod    deepCopy;
};

typedef const struct DKCopyingInterface * DKCopyingInterfaceRef;


// Default copying interface that retains and returns the object. This is used by the
// root classes so it's defined in DKRuntime.c.
DK_API DKInterfaceRef DKDefaultCopying( void );

// A default deepCopy method that calls DKCopy and DKMutableCopy as needed
DK_API DKObjectRef DKDefaultDeepCopy( DKObjectRef object, int options );

DK_API DKObjectRef DKCopy( DKObjectRef _self );
DK_API DKObjectRef DKMutableCopy( DKObjectRef _self );
DK_API DKObjectRef DKDeepCopy( DKObjectRef _self, int options );


#ifdef __cplusplus
}
#endif

#endif // _DK_COPYING_H_

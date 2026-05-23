// =======================================================================================
//
// DKLocking.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_LOCKING_H_
#define _DK_LOCKING_H_

#ifdef __cplusplus
extern "C"
{
#endif


DK_API DKDeclareInterfaceSelector( Locking );


typedef void (*DKLockMethod)( DKObjectRef _self );
typedef void (*DKUnlockMethod)( DKObjectRef _self );


struct DKLockingInterface
{
    const DKInterface _interface;

    DKLockMethod    lock;
    DKUnlockMethod  unlock;
};

typedef const struct DKLockingInterface * DKLockingInterfaceRef;


// Default locking interface that associates aretains and returns the object. This is used by the
// root classes so it's defined in DKRuntime.c.
DK_API DKInterfaceRef DKDefaultLocking( void );


DK_API void DKLock( DKObjectRef _self );
DK_API void DKUnlock( DKObjectRef _self );



#ifdef __cplusplus
}
#endif

#endif // _DK_LOCKING_H_

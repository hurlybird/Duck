// =======================================================================================
//
// DKReadWriteLock.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_READWRITE_LOCK_H_
#define _DK_READWRITE_LOCK_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DKReadWriteLock * DKReadWriteLockRef;


DK_API DKClassRef DKReadWriteLockClass( void );

#define DKNewReadWriteLock()    DKNew( DKReadWriteLockClass() )


DK_API void DKReadWriteLockLock( DKReadWriteLockRef _self, bool readwrite );
DK_API bool DKReadWriteLockTryLock( DKReadWriteLockRef _self, bool readwrite );
DK_API void DKReadWriteLockUnlock( DKReadWriteLockRef _self );


#ifdef __cplusplus
}
#endif

#endif // _DK_READWRITE_LOCK_H_


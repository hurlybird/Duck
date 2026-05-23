// =======================================================================================
//
// DKSemaphore.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_SEMAPHORE_H_
#define _DK_SEMAPHORE_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DKSemaphore * DKSemaphoreRef;


DK_API DKClassRef DKSemaphoreClass( void );

#define DKNewSemaphore()        DKNew( DKSemaphoreClass() )


DK_API void DKSemaphoreIncrement( DKSemaphoreRef _self, uint32_t value );
DK_API void DKSemaphoreDecrement( DKSemaphoreRef _self, uint32_t value );
DK_API void DKSemaphoreWait( DKSemaphoreRef _self, uint32_t value );


#ifdef __cplusplus
}
#endif

#endif // _DK_MUTEX_H_


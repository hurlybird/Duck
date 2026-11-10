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


// counter += value
DK_API void DKSemaphoreIncrement( DKSemaphoreRef _self, unsigned int value );

// counter -= value
DK_API void DKSemaphoreDecrement( DKSemaphoreRef _self, unsigned int value );

// counter = value
DK_API void DKSemaphoreSignal( DKSemaphoreRef _self, unsigned int value );

// Wait until counter > 0, then decrements the counter by 1
DK_API void DKSemaphoreWait( DKSemaphoreRef _self );

// Wait until counter == test. If reset >= 0, set the counter to the reset value.
DK_API void DKSemaphoreWaitUntil( DKSemaphoreRef _self, unsigned int test, int reset );

// Wait until counter != test and return the counter. If reset >= 0, set the counter to the reset value.
DK_API unsigned int DKSemaphoreWaitWhile( DKSemaphoreRef _self, unsigned int test, int reset );


#ifdef __cplusplus
}
#endif

#endif // _DK_MUTEX_H_


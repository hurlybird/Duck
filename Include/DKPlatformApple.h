//
//  DKPlatformApple.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-13.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_ENV_APPLE_H_
#define _DK_ENV_APPLE_H_

#include <libkern/OSAtomic.h>


typedef OSSpinLock DKSpinLock;

#define DKSpinLockInit              OS_SPINLOCK_INIT

#define DKSpinLockLock( s )         OSSpinLockLock( s )
#define DKSpinLockUnlock( s )       OSSpinLockUnlock( s )


// GCC and Clang built-in syntax
//#define DKAtomicIncrement32( ptr )    __sync_add_and_fetch( ptr, 1 )
//#define DKAtomicDecrement32( ptr )    __sync_sub_and_fetch( ptr, 1 )
//#define DKAtomicCmpAndSwapPtr( val, old, new )    __sync_bool_compare_and_swap( val, old, new )

#define DKAtomicIncrement32( ptr )    OSAtomicIncrement32Barrier( ptr )
#define DKAtomicDecrement32( ptr )    OSAtomicDecrement32Barrier( ptr )
#define DKAtomicCmpAndSwapPtr( val, old, new )  OSAtomicCompareAndSwapPtrBarrier( old, new, val )


#endif // _DK_ENV_APPLE_H_

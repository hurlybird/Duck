//
//  DKEnvApple.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-13.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_ENV_APPLE_H_
#define _DK_ENV_APPLE_H_

#include <libkern/OSAtomic.h>


// Atomic Integers
//typedef int32_t DKAtomicInt;

//#define DKAtomicIncrement( ptr )    __sync_add_and_fetch( ptr, 1 )
//#define DKAtomicDecrement( ptr )    __sync_sub_and_fetch( ptr, 1 )

typedef int32_t DKAtomicInt;

#define DKAtomicIncrement( ptr )    OSAtomicIncrement32( ptr )
#define DKAtomicDecrement( ptr )    OSAtomicDecrement32( ptr )



#endif // _DK_ENV_APPLE_H_

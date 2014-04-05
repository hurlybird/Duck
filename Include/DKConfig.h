//
//  DKConfig.h
//  Duck
//
//  Created by Derek Nylen on 2014-04-05.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_CONFIG_H_
#define _DK_CONFIG_H_



// Release ===============================================================================
#ifdef NDEBUG

// Enable warnings
#ifndef DK_RUNTIME_WARNINGS
#define DK_RUNTIME_WARNINGS 0
#endif

// Enable assertions. Failed assertions raise fatal errors.
#ifndef DK_RUNTIME_ASSERTIONS
#define DK_RUNTIME_ASSERTIONS 0
#endif

// Enable extra type checking. Failed checks raise errors.
#ifndef DK_RUNTIME_TYPE_CHECKS
#define DK_RUNTIME_TYPE_CHECKS 1
#endif

// Enable range checks on arrays, lists, buffers, etc. Failed checks raise errors.
#ifndef DK_RUNTIME_RANGE_CHECKS
#define DK_RUNTIME_RANGE_CHECKS 1
#endif

// Enable internal consistency checks. THIS IS VERY SLOW
#ifndef DK_RUNTIME_INTEGRITY_CHECKS
#define DK_RUNTIME_INTEGRITY_CHECKS 0
#endif




// Debug =================================================================================
#else

#ifndef DK_RUNTIME_WARNINGS
#define DK_RUNTIME_WARNINGS 1
#endif

#ifndef DK_RUNTIME_ASSERTIONS
#define DK_RUNTIME_ASSERTIONS 1
#endif

#ifndef DK_RUNTIME_TYPE_CHECKS
#define DK_RUNTIME_TYPE_CHECKS  1
#endif

#ifndef DK_RUNTIME_RANGE_CHECKS
#define DK_RUNTIME_RANGE_CHECKS 1
#endif

#ifndef DK_RUNTIME_INTEGRITY_CHECKS
#define DK_RUNTIME_INTEGRITY_CHECKS 0
#endif

#endif // NDEBUG



// Platform ==============================================================================

// These are mainly for reference -- they don't do much yet
#ifndef DK_PLATFORM_APPLE
#define DK_PLATFORM_APPLE       1
#endif

#ifndef DK_PLATFORM_LINUX
#define DK_PLATFORM_LINUX       0
#endif

#ifndef DK_PLATFORM_ANDROID
#define DK_PLATFORM_ANDROID     0
#endif

#ifndef DK_PLATFORM_POSIX
#define DK_PLATFORM_POSIX       1
#endif






#endif // _DK_CONFIG_H_



/*****************************************************************************************

  DKConfig.h

  Copyright (c) 2014 Derek W. Nylen

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*****************************************************************************************/

#ifndef _DK_CONFIG_H_
#define _DK_CONFIG_H_




// Debug =================================================================================
#ifdef DEBUG

// Enable warnings
#ifndef DK_RUNTIME_WARNINGS
#define DK_RUNTIME_WARNINGS 1
#endif

// Enable assertions. Failed assertions raise fatal errors.
#ifndef DK_RUNTIME_ASSERTIONS
#define DK_RUNTIME_ASSERTIONS 1
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

// Enable malloc tracing
#ifndef DK_MALLOC_TRACE
#define DK_MALLOC_TRACE 0
#endif




// Release ===============================================================================
#else

#ifndef DK_RUNTIME_WARNINGS
#define DK_RUNTIME_WARNINGS 1
#endif

#ifndef DK_RUNTIME_ASSERTIONS
#define DK_RUNTIME_ASSERTIONS 0
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

#ifndef DK_MALLOC_TRACE
#define DK_MALLOC_TRACE 0
#endif

#endif // DEBUG




// Platform ==============================================================================

#ifndef DK_PLATFORM_APPLE
#define DK_PLATFORM_APPLE       1
#endif

#ifndef DK_PLATFORM_BSD
#define DK_PLATFORM_BSD         1
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




// Settings ==============================================================================
#define DK_AUTORELEASE_POOL_STACK_SIZE  8



#endif // _DK_CONFIG_H_



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


// Platform ==============================================================================

// #define DK_PLATFORM_APPLE            -- Apple MacOS or iOS (Darwin)
// #define DK_PLATFORM_LINUX            -- Linux
// #define DK_PLATFORM_UNIX             -- Other Unix
// #define DK_PLATFORM_WINDOWS			-- Windows
// #define DK_PLATFORM_ANDROID			-- Android

// These are configured based on the above platforms
// #define DK_PLATFORM_POSIX            -- POSIX available
// #define DK_PLATFORM_GCC_INTRINSICS   -- Use GCC __sync* and __builtin_bswap* intrinsics


// Apple
#if DK_PLATFORM_APPLE
#define DK_PLATFORM_POSIX           1
#define DK_PLATFORM_GCC_INTRINSICS  1 // Clang supports GCC-style sync and swap intrinsics


// Linux
#elif DK_PLATFORM_LINUX
#define DK_PLATFORM_POSIX           1
#define DK_PLATFORM_GCC_INTRINSICS  1


// Generic Unix
#elif DK_PLATFORM_UNIX
#define DK_PLATFORM_POSIX           1
#define DK_PLATFORM_GCC_INTRINSICS  1


// Android
#elif DK_PLATFORM_ANDROID
#define DK_PLATFORM_POSIX           1
#define DK_PLATFORM_ANDROID_NDK     1
#define DK_PLATFORM_GCC_INTRINSICS  1


// Windows
#elif DK_PLATFORM_WINDOWS


// Nintendo Switch
#elif DK_PLATFORM_NX
#define DK_PLATFORM_POSIX           1
#define DK_PLATFORM_GCC_INTRINSICS  1


// PlayStation 5
#elif DK_PLATFORM_PS5
#define DK_PLATFORM_POSIX           1
#define DK_PLATFORM_GCC_INTRINSICS  1


// Autodetect Apple
#elif __APPLE__
#define DK_PLATFORM_APPLE           1
#define DK_PLATFORM_POSIX           1
#define DK_PLATFORM_GCC_INTRINSICS  1


// Autodetect Android
#elif __ANDROID__
#define DK_PLATFORM_ANDROID         1
#define DK_PLATFORM_POSIX           1
#define DK_PLATFORM_GCC_INTRINSICS  1


// Autodetect Nintendo Switch
#elif NX
#define DK_PLATFORM_NX              1
#define DK_PLATFORM_POSIX           1
#define DK_PLATFORM_GCC_INTRINSICS  1


// Autodetect PlayStation 5
#elif PS5
#define DK_PLATFORM_PS5             1
#define DK_PLATFORM_POSIX           1
#define DK_PLATFORM_GCC_INTRINSICS  1


// Autodetect Linux
#elif __linux__
#define DK_PLATFORM_LINUX           1
#define DK_PLATFORM_POSIX           1
#define DK_PLATFORM_GCC_INTRINSICS  1


// Autodetect Other Unix
#elif __unix__
#define DK_PLATFORM_UNIX            1
#define DK_PLATFORM_POSIX           1
#define DK_PLATFORM_GCC_INTRINSICS  1


// Autodetect Windows
#elif defined(_WIN32) || defined(_WIN64)
#define DK_PLATFORM_WINDOWS         1


// Unknown Platform
#else
#error "Duck: Unknown or missing platform definition"

#endif




// Debug =================================================================================
#if DK_PLATFORM_WINDOWS && defined(_DEBUG) && !defined(DEBUG)
#define DEBUG 1
#endif

#ifdef DEBUG

// Enable warnings
#ifndef DK_RUNTIME_WARNINGS
#define DK_RUNTIME_WARNINGS 1
#endif

// Enable assertions. Failed assertions raise fatal errors.
#ifndef DK_RUNTIME_ASSERTIONS
#define DK_RUNTIME_ASSERTIONS 1
#endif

// Enable runtime logic checking. Failed checks raise errors.
#ifndef DK_RUNTIME_LOGIC_CHECKS
#define DK_RUNTIME_LOGIC_CHECKS 1
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

#ifndef DK_RUNTIME_LOGIC_CHECKS
#define DK_RUNTIME_LOGIC_CHECKS 1
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




// Settings ==============================================================================

// Max nested autorelease pools
#ifndef DK_AUTORELEASE_POOL_STACK_SIZE
#define DK_AUTORELEASE_POOL_STACK_SIZE  8
#endif

// Remove extra trailing zeroes from %f and %lf float formats in DKSPrintf
#ifndef DK_PRETTY_PRINT_FLOATS
#define DK_PRETTY_PRINT_FLOATS  1
#endif


#endif // _DK_CONFIG_H_



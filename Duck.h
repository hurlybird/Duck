/*****************************************************************************************

  Duck.h

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

#ifndef _DUCK_LIBRARY_H_
#define _DUCK_LIBRARY_H_

//! Project version number for Duck.
//FOUNDATION_EXPORT double DuckVersionNumber;

//! Project version string for Duck.
//FOUNDATION_EXPORT const unsigned char DuckVersionString[];

#if __APPLE__
#define DK_EXCLUDE_NONMODULAR_HEADERS
#endif


#include <Duck/DKConfig.h>
#include <Duck/DKPlatform.h>
#include <Duck/DKEncoding.h>

#include <Duck/DKByteArray.h>
#include <Duck/DKGenericArray.h>
#include <Duck/DKGenericHashTable.h>
#include <Duck/DKNodePool.h>
#include <Duck/DKUnicode.h>

#include <Duck/DKRuntime.h>

#include <Duck/DKThread.h>
#include <Duck/DKThreadPool.h>
#include <Duck/DKMutex.h>
#include <Duck/DKCondition.h>
#include <Duck/DKReadWriteLock.h>
#include <Duck/DKSemaphore.h>

#include <Duck/DKAllocation.h>
#include <Duck/DKBuffer.h>
#include <Duck/DKComparison.h>
#include <Duck/DKCopying.h>
#include <Duck/DKDescription.h>
#include <Duck/DKLocking.h>
#include <Duck/DKStream.h>
#include <Duck/DKConversion.h>

#include <Duck/DKBitList.h>
#include <Duck/DKBoolean.h>
#include <Duck/DKData.h>
#include <Duck/DKNumber.h>
#include <Duck/DKPair.h>
#include <Duck/DKString.h>
#include <Duck/DKStruct.h>

#include <Duck/DKEnum.h>
#include <Duck/DKPredicate.h>

#include <Duck/DKFile.h>
#include <Duck/DKEgg.h>
#include <Duck/DKShell.h>
#include <Duck/DKJSON.h>
#include <Duck/DKXML.h>

#include <Duck/DKCollection.h>
#include <Duck/DKList.h>
#include <Duck/DKDictionary.h>
#include <Duck/DKSet.h>

#include <Duck/DKArray.h>
#include <Duck/DKLinkedList.h>
#include <Duck/DKBinaryTree.h>
#include <Duck/DKHashTable.h>
#include <Duck/DKGraph.h>


#endif

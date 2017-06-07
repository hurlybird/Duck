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


#ifdef __cplusplus
extern "C"
{
#endif

#define DK_EXCLUDE_NONMODULAR_HEADERS

#include "DKConfig.h"
#include "DKPlatform.h"

#include "DKByteArray.h"
#include "DKGenericArray.h"
#include "DKGenericHashTable.h"
#include "DKNodePool.h"
#include "DKUnicode.h"

#include "DKRuntime.h"
#include "DKThread.h"
#include "DKThreadPool.h"
#include "DKMutex.h"
#include "DKCondition.h"
#include "DKReadWriteLock.h"

#include "DKAllocation.h"
#include "DKComparison.h"
#include "DKCopying.h"
#include "DKDescription.h"
#include "DKLocking.h"
#include "DKStream.h"
#include "DKConversion.h"

#include "DKBoolean.h"
#include "DKData.h"
#include "DKNumber.h"
#include "DKPair.h"
#include "DKString.h"
#include "DKStruct.h"

#include "DKEncoding.h"
#include "DKEnum.h"
#include "DKModifier.h"
#include "DKPredicate.h"

#include "DKFile.h"
#include "DKEgg.h"
#include "DKShell.h"
#include "DKJSON.h"
#include "DKXML.h"

#include "DKCollection.h"
#include "DKList.h"
#include "DKDictionary.h"
#include "DKSet.h"

#include "DKArray.h"
#include "DKLinkedList.h"
#include "DKBinaryTree.h"
#include "DKHashTable.h"
#include "DKGraph.h"


#ifdef __cplusplus
}
#endif


#endif

// =======================================================================================
//
// Duck.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
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
#include <Duck/DKObjectPool.h>
#include <Duck/DKUnicode.h>
#include <Duck/DKCharacterSet.h>

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
#include <Duck/DKMember.h>
#include <Duck/DKNumber.h>
#include <Duck/DKPair.h>
#include <Duck/DKString.h>
#include <Duck/DKStruct.h>

#include <Duck/DKEnum.h>
#include <Duck/DKPredicate.h>

#include <Duck/DKFile.h>
#include <Duck/DKEgg.h>
#include <Duck/DKQuicksort.h>
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

// =======================================================================================
//
// DKRuntime+Symbols.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#define DK_RUNTIME_PRIVATE 1

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKGenericArray.h"
#include "DKGenericHashTable.h"
#include "DKRuntime.h"

#include "DKAllocation.h"
#include "DKBuffer.h"
#include "DKComparison.h"
#include "DKCopying.h"
#include "DKDescription.h"
#include "DKLocking.h"
#include "DKStream.h"

#include "DKArray.h"
#include "DKBinaryTree.h"
#include "DKCollection.h"
#include "DKDictionary.h"
#include "DKHashTable.h"
#include "DKGraph.h"
#include "DKLinkedList.h"
#include "DKList.h"
#include "DKSet.h"

#include "DKBoolean.h"
#include "DKData.h"
#include "DKNumber.h"
#include "DKPair.h"
#include "DKString.h"
#include "DKStruct.h"

#include "DKThread.h"
#include "DKThreadPool.h"
#include "DKMutex.h"
#include "DKCondition.h"
#include "DKReadWriteLock.h"
#include "DKSemaphore.h"

#include "DKEgg.h"
#include "DKEnum.h"
#include "DKFile.h"
#include "DKPredicate.h"


// Poke the built-in classes and selectors to make them available for symbolic lookup from
// DKClassFromString() and DKSelectorFromString(), which is often needed by script plugins
// or serialization.

///
//  DKRuntimeInitSymbols()
//
void DKRuntimeInitSymbols( void )
{
    // Interfaces
    DKSelector( Allocation );
    DKSelector( Buffer );
    DKSelector( Comparison );
    DKSelector( Copying );
    DKSelector( Description );
    DKSelector( Locking );
    DKSelector( Stream );
    
    // Collection Types
    DKArrayClass();
    DKMutableArrayClass();
    DKBinaryTreeClass();
    DKMutableBinaryTreeClass();
    DKSelector( Collection );
    DKSelector( KeyedCollection );
    DKSelector( Dictionary );
    DKHashTableClass();
    DKMutableHashTableClass();
    DKGraphClass();
    DKGraphEdgeClass();
    DKLinkedListClass();
    DKMutableLinkedListClass();
    DKSelector( List );
    DKSelector( Set );
    
    // Data Types
    DKBooleanClass();
    DKDataClass();
    DKMutableDataClass();
    DKNumberClass();
    DKPairClass();
    DKStringClass();
    DKConstantStringClass();
    DKMutableStringClass();
    DKStructClass();
    
    // Thread Synchronization
    DKThreadClass();
    DKThreadPoolClass();
    DKMutexClass();
    DKConditionClass();
    DKReadWriteLockClass();
    DKSemaphoreClass();
    
    // Other
    DKEggArchiverClass();
    DKEggUnarchiverClass();
    DKEnumClass();
    DKFileClass();
    DKPredicateClass();
}




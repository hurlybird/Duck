/*******************************************************************************

  DKDictionary.h

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

*******************************************************************************/

#ifndef _DK_DICTIONARY_H_
#define _DK_DICTIONARY_H_

#include "DKRuntime.h"
#include "DKList.h"


DKDeclareInterfaceSelector( Dictionary );


typedef const void * DKDictionaryRef;
typedef void * DKMutableDictionaryRef;


typedef int (*DKDictionaryApplierFunction)( DKObjectRef key, DKObjectRef value, void * context );


typedef enum
{
    DKDictionaryInsertAlways,
    DKDictionaryInsertIfFound,
    DKDictionaryInsertIfNotFound
    
} DKDictionaryInsertPolicy;

typedef DKIndex     (*DKDictionaryGetCountMethod)( DKDictionaryRef _self );
typedef DKObjectRef (*DKDictionaryGetObjectMethod)( DKDictionaryRef _self, DKObjectRef key );
typedef int         (*DKDictionaryApplyFunctionMethod)( DKDictionaryRef _self, DKDictionaryApplierFunction, void * context );
typedef void        (*DKDictionaryInsertObjectMethod)( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object, DKDictionaryInsertPolicy policy );
typedef void        (*DKDictionaryRemoveObjectMethod)( DKMutableDictionaryRef _self, DKObjectRef key );
typedef void        (*DKDictionaryRemoveAllObjectsMethod)( DKMutableDictionaryRef _self );

struct DKDictionary
{
    DKInterface _interface;

    DKDictionaryGetCountMethod          getCount;
    DKDictionaryGetObjectMethod         getObject;
    DKDictionaryApplyFunctionMethod     applyFunction;
    DKDictionaryInsertObjectMethod      insertObject;
    DKDictionaryRemoveObjectMethod      removeObject;
    DKDictionaryRemoveAllObjectsMethod  removeAllObjects;
};

typedef const struct DKDictionary DKDictionary;


DKClassRef  DKDictionaryClass( void );
void        DKSetDictionaryClass( DKClassRef _self );

DKClassRef  DKMutableDictionaryClass( void );
void        DKSetMutableDictionaryClass( DKClassRef _self );

DKIndex DKDictionaryGetCount( DKDictionaryRef _self );

void DKDictionarySetObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
void DKDictionaryAddObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
void DKDictionaryReplaceObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
void DKDictionaryAddEntriesFromDictionary( DKMutableDictionaryRef _self, DKDictionaryRef src );

int DKDictionaryContainsKey( DKDictionaryRef _self, DKObjectRef key );
int DKDictionaryContainsObject( DKDictionaryRef _self, DKObjectRef object );

DKListRef DKDictionaryCopyKeys( DKDictionaryRef _self );
DKListRef DKDictionaryCopyObjects( DKDictionaryRef _self );

DKObjectRef DKDictionaryGetObject( DKDictionaryRef _self, DKObjectRef key );

void DKDictionaryRemoveObject( DKMutableDictionaryRef _self, DKObjectRef key );
void DKDictionaryRemoveAllObjects( DKMutableDictionaryRef _self );

int DKDictionaryApplyFunction( DKDictionaryRef _self, DKDictionaryApplierFunction callback, void * context );

DKStringRef DKDictionaryCopyDescription( DKListRef _self );


#endif // _DK_DICTIONARY_H_





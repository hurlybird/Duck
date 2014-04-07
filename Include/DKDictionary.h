/*****************************************************************************************

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

*****************************************************************************************/

#ifndef _DK_DICTIONARY_H_
#define _DK_DICTIONARY_H_

#include "DKRuntime.h"
#include "DKList.h"


DKDeclareInterfaceSelector( Dictionary );


typedef const void * DKDictionaryRef;
typedef void * DKMutableDictionaryRef;


typedef DKObjectRef (*DKDictionaryCreateWithVAKeysAndObjectsMethod)( DKClassRef _class, va_list keysAndObjects );
typedef DKObjectRef (*DKDictionaryCreateWithDictionaryMethod)( DKClassRef _class, DKDictionaryRef srcDictionary );

typedef DKObjectRef (*DKDictionaryGetObjectMethod)( DKDictionaryRef _self, DKObjectRef key );

typedef void        (*DKDictionaryInsertObjectMethod)( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
typedef void        (*DKDictionaryRemoveObjectMethod)( DKMutableDictionaryRef _self, DKObjectRef key );
typedef void        (*DKDictionaryRemoveAllObjectsMethod)( DKMutableDictionaryRef _self );


struct DKDictionaryInterface
{
    const DKInterface _interface;

    DKDictionaryCreateWithVAKeysAndObjectsMethod    createWithVAKeysAndObjects;
    DKDictionaryCreateWithDictionaryMethod          createWithDictionary;

    DKGetCountMethod                    getCount;
    DKDictionaryGetObjectMethod         getObject;
    
    // Mutable dictionaries -- these raise errors when called on immutable dictionaries
    DKDictionaryInsertObjectMethod      insertObject;
    DKDictionaryRemoveObjectMethod      removeObject;
    DKDictionaryRemoveAllObjectsMethod  removeAllObjects;
};

typedef const struct DKDictionaryInterface * DKDictionaryInterfaceRef;


DKClassRef  DKDictionaryClass( void );
void        DKSetDefaultDictionaryClass( DKClassRef _class );

DKClassRef  DKMutableDictionaryClass( void );
void        DKSetDefaultMutableDictionaryClass( DKClassRef _class );

#define     DKDictionaryCreateEmpty()    DKCreate( DKDictionaryClass() )
#define     DKDictionaryCreateMutable()  DKCreate( DKMutableDictionaryClass() )

DKObjectRef DKDictionaryCreateWithKeysAndObjects( DKClassRef _class, DKObjectRef firstKey, ... );
DKObjectRef DKDictionaryCreateWithDictionary( DKClassRef _class, DKDictionaryRef srcDictionary );

DKIndex     DKDictionaryGetCount( DKDictionaryRef _self );
DKObjectRef DKDictionaryGetObject( DKDictionaryRef _self, DKObjectRef key );

int         DKDictionaryContainsKey( DKDictionaryRef _self, DKObjectRef key );
int         DKDictionaryContainsObject( DKDictionaryRef _self, DKObjectRef object );

DKListRef   DKDictionaryCopyKeys( DKDictionaryRef _self );
DKListRef   DKDictionaryCopyObjects( DKDictionaryRef _self );

void        DKDictionarySetObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
void        DKDictionaryAddObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
void        DKDictionaryReplaceObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
void        DKDictionaryAddEntriesFromDictionary( DKMutableDictionaryRef _self, DKDictionaryRef src );

void        DKDictionaryRemoveObject( DKMutableDictionaryRef _self, DKObjectRef key );
void        DKDictionaryRemoveAllObjects( DKMutableDictionaryRef _self );



#endif // _DK_DICTIONARY_H_





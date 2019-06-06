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


DK_API DKDeclareInterfaceSelector( Dictionary );


// typedef DKObjectRef DKDictionaryRef; -- Declared in DKPlatform.h
typedef DKObjectRef DKMutableDictionaryRef;


typedef DKObjectRef (*DKDictionaryInitWithVAKeysAndObjectsMethod)( DKDictionaryRef _self, va_list keysAndObjects );
typedef DKObjectRef (*DKDictionaryInitWithDictionaryMethod)( DKDictionaryRef _self, DKDictionaryRef srcDictionary );

typedef DKObjectRef (*DKDictionaryGetObjectMethod)( DKDictionaryRef _self, DKObjectRef key );

typedef void        (*DKDictionaryInsertObjectMethod)( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
typedef void        (*DKDictionaryRemoveObjectMethod)( DKMutableDictionaryRef _self, DKObjectRef key );
typedef void        (*DKDictionaryRemoveAllObjectsMethod)( DKMutableDictionaryRef _self );


struct DKDictionaryInterface
{
    const DKInterface _interface;

    DKDictionaryInitWithVAKeysAndObjectsMethod initWithVAKeysAndObjects;
    DKDictionaryInitWithDictionaryMethod       initWithDictionary;

    DKGetCountMethod                    getCount;
    DKDictionaryGetObjectMethod         getObject;
    
    // Mutable dictionaries -- these raise errors when called on immutable dictionaries
    DKDictionaryInsertObjectMethod      insertObject;
    DKDictionaryRemoveObjectMethod      removeObject;
    DKDictionaryRemoveAllObjectsMethod  removeAllObjects;
};

typedef const struct DKDictionaryInterface * DKDictionaryInterfaceRef;


DK_API DKClassRef  DKDictionaryClass( void );
DK_API void        DKSetDefaultDictionaryClass( DKClassRef _class );

DK_API DKClassRef  DKMutableDictionaryClass( void );
DK_API void        DKSetDefaultMutableDictionaryClass( DKClassRef _class );

#define            DKEmptyDictionary()         DKAutorelease( DKNew( DKDictionaryClass() ) )
#define            DKMutableDictionary()       DKAutorelease( DKNew( DKMutableDictionaryClass() ) )

#define            DKNewEmptyDictionary()      DKNew( DKDictionaryClass() )
#define            DKNewMutableDictionary()    DKNew( DKMutableDictionaryClass() )

#define            DKDictionaryWithKeysAndObjects( firstKey, ... )         DKAutorelease( DKDictionaryInitWithKeysAndObjects( DKAlloc( DKDictionaryClass() ), firstKey, __VA_ARGS__, NULL ) )
#define            DKDictionaryWithDictionary( srcDictionary )             DKAutorelease( DKDictionaryInitWithDictionary( DKAlloc( DKDictionaryClass() ), srcDictionary ) )

#define            DKNewDictionaryWithKeysAndObjects( firstKey, ... )      DKDictionaryInitWithKeysAndObjects( DKAlloc( DKDictionaryClass() ), firstKey, __VA_ARGS__, NULL )
#define            DKNewDictionaryWithDictionary( srcDictionary )          DKDictionaryInitWithDictionary( DKAlloc( DKDictionaryClass() ), srcDictionary )

#define            DKMutableDictionaryWithKeysAndObjects( firstKey, ... )  DKAutorelease( DKDictionaryInitWithKeysAndObjects( DKAlloc( DKMutableDictionaryClass() ), firstKey, __VA_ARGS__, NULL ) )
#define            DKMutableDictionaryWithDictionary( srcDictionary )      DKAutorelease( DKDictionaryInitWithDictionary( DKAlloc( DKMutableDictionaryClass() ), srcDictionary ) )

DK_API DKObjectRef DKDictionaryInitWithKeysAndObjects( DKDictionaryRef _self, ... );
DK_API DKObjectRef DKDictionaryInitWithDictionary( DKDictionaryRef _self, DKDictionaryRef srcDictionary );

DK_API DKIndex     DKDictionaryGetCount( DKDictionaryRef _self );
DK_API DKObjectRef DKDictionaryGetObject( DKDictionaryRef _self, DKObjectRef key );

DK_API int         DKDictionaryContainsKey( DKDictionaryRef _self, DKObjectRef key );
DK_API int         DKDictionaryContainsObject( DKDictionaryRef _self, DKObjectRef object );

DK_API DKListRef   DKDictionaryGetAllKeys( DKDictionaryRef _self );
DK_API DKListRef   DKDictionaryGetAllObjects( DKDictionaryRef _self );

DK_API void        DKDictionarySetObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
DK_API void        DKDictionaryAddObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
DK_API void        DKDictionaryReplaceObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
DK_API void        DKDictionaryInsertObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
DK_API void        DKDictionaryInsertEntriesFromDictionary( DKMutableDictionaryRef _self, DKDictionaryRef src, DKInsertPolicy policy );

DK_API void        DKDictionaryRemoveObject( DKMutableDictionaryRef _self, DKObjectRef key );
DK_API void        DKDictionaryRemoveAllObjects( DKMutableDictionaryRef _self );

DK_API bool        DKDictionaryEqual( DKDictionaryRef _self, DKDictionaryRef other );

DK_API bool        DKDictionaryIsSubsetOfDictionary( DKDictionaryRef _self, DKDictionaryRef other );


#endif // _DK_DICTIONARY_H_





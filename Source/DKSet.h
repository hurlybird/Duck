/*****************************************************************************************

  DKSet.h

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

#ifndef _DK_SET_H_
#define _DK_SET_H_

#include "DKRuntime.h"
#include "DKList.h"


DK_API DKDeclareInterfaceSelector( Set );


//typedef DKObjectRef DKSetRef; -- Declared in DKPlatform.h
typedef DKObjectRef DKMutableSetRef;


typedef int (*DKSetApplierFunction)( DKObjectRef object, void * context );

typedef DKObjectRef (*DKSetInitWithVAObjectsMethod)( DKSetRef _self, va_list objects );
typedef DKObjectRef (*DKSetInitWithCArrayMethod)( DKSetRef _self, DKObjectRef objects[], DKIndex count );
typedef DKObjectRef (*DKSetInitWithCollectionMethod)( DKSetRef _self, DKObjectRef srcCollection );

typedef DKObjectRef (*DKSetGetMemberMethod)( DKSetRef _self, DKObjectRef object );

typedef void        (*DKSetAddObjectMethod)( DKMutableSetRef _self, DKObjectRef object );
typedef void        (*DKSetRemoveObjectMethod)( DKMutableSetRef _self, DKObjectRef object );
typedef void        (*DKSetRemoveAllObjectsMethod)( DKMutableSetRef _self );


struct DKSetInterface
{
    const DKInterface _interface;

    DKSetInitWithVAObjectsMethod  initWithVAObjects;
    DKSetInitWithCArrayMethod     initWithCArray;
    DKSetInitWithCollectionMethod initWithCollection;

    DKGetCountMethod            getCount;
    DKSetGetMemberMethod        getMember;
    
    // Mutable Sets -- these raise errors when called on immutable sets
    DKSetAddObjectMethod        addObject;
    DKSetRemoveObjectMethod     removeObject;
    DKSetRemoveAllObjectsMethod removeAllObjects;
};

typedef const struct DKSetInterface * DKSetInterfaceRef;


DK_API DKClassRef  DKSetClass( void );
DK_API void        DKSetDefaultSetClass( DKClassRef _self );

DK_API DKClassRef  DKMutableSetClass( void );
DK_API void        DKSetDefaultMutableSetClass( DKClassRef _self );

#define            DKEmptySet()            DKAutorelease( DKNew( DKSetClass() ) )
#define            DKMutableSet()          DKAutorelease( DKNew( DKMutableSetClass() ) )

#define            DKSetWithObject( object )           DKAutorelease( DKSetInitWithObject( DKAlloc( DKSetClass() ), object ) )
#define            DKSetWithObjects( ... )             DKAutorelease( DKSetInitWithObjects( DKAlloc( DKSetClass() ), __VA_ARGS__, NULL ) )
#define            DKSetWithVAObjects( objects )       DKAutorelease( DKSetInitWithVAObjects( DKAlloc( DKSetClass() ), objects ) )
#define            DKSetWithCArray( objects, count )   DKAutorelease( DKSetInitWithCArray( DKAlloc( DKSetClass() ), objects, count ) )
#define            DKSetWithCollection( collection )   DKAutorelease( DKSetInitWithCollection( DKAlloc( DKSetClass() ), collection ) )

#define            DKNewMutableSet()       DKNew( DKMutableSetClass() )

DK_API DKObjectRef DKSetInitWithObject( DKSetRef _self, DKObjectRef object );
DK_API DKObjectRef DKSetInitWithObjects( DKSetRef _self, ... );
DK_API DKObjectRef DKSetInitWithVAObjects( DKSetRef _self, va_list objects );
DK_API DKObjectRef DKSetInitWithCArray( DKSetRef _self, DKObjectRef objects[], DKIndex count );
DK_API DKObjectRef DKSetInitWithCollection( DKSetRef _self, DKObjectRef srcCollection );

DK_API DKIndex     DKSetGetCount( DKSetRef _self );
DK_API DKObjectRef DKSetGetMember( DKSetRef _self, DKObjectRef object );
DK_API bool        DKSetContainsObject( DKSetRef _self, DKObjectRef object );

DK_API DKListRef   DKSetGetAllObjects( DKDictionaryRef _self );

DK_API void        DKSetAddObject( DKMutableSetRef _self, DKObjectRef object );
DK_API void        DKSetRemoveObject( DKMutableSetRef _self, DKObjectRef object );
DK_API void        DKSetRemoveAllObjects( DKMutableSetRef _self );

DK_API bool        DKSetEqualToSet( DKSetRef _self, DKSetRef otherSet );
DK_API int         DKSetIsSubsetOfSet( DKSetRef _self, DKSetRef otherSet );
DK_API int         DKSetIntersectsSet( DKSetRef _self, DKSetRef otherSet );

DK_API void        DKSetUnion( DKMutableSetRef _self, DKSetRef otherSet );
DK_API void        DKSetMinus( DKMutableSetRef _self, DKSetRef otherSet );
DK_API void        DKSetIntersect( DKMutableSetRef _self, DKSetRef otherSet );




#endif // _DK_SET_H_

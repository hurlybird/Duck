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

#ifndef _DK_SET_H_
#define _DK_SET_H_

#include "DKRuntime.h"
#include "DKList.h"


DKDeclareInterfaceSelector( Set );


typedef const void * DKSetRef;
typedef void * DKMutableSetRef;


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


DKClassRef  DKSetClass( void );
void        DKSetDefaultSetClass( DKClassRef _self );

DKClassRef  DKMutableSetClass( void );
void        DKSetDefaultMutableSetClass( DKClassRef _self );

#define     DKSetCreateEmpty()    DKCreate( DKSetClass() )
#define     DKSetCreateMutable()  DKCreate( DKMutableSetClass() )

#define     DKSetCreateWithObject( cls, object )            DKSetInitWithObject( DKAlloc( cls, 0 ), object )
#define     DKSetCreateWithObjects( cls, ... )              DKSetInitWithObjects( DKAlloc( cls, 0 ), __VA_ARGS__ )
#define     DKSetCreateWithVAObjects( cls, objects )        DKSetInitWithVAObjects( DKAlloc( cls, 0 ), objects )
#define     DKSetCreateWithCArray( cls, objects, count )    DKSetInitWithCArray( DKAlloc( cls, 0 ), objects, count )
#define     DKSetCreateWithCollection( cls, collection )    DKSetInitWithCollection( DKAlloc( cls, 0 ), collection )

DKObjectRef DKSetInitWithObject( DKSetRef _self, DKObjectRef object );
DKObjectRef DKSetInitWithObjects( DKSetRef _self, ... );
DKObjectRef DKSetInitWithVAObjects( DKSetRef _self, va_list objects );
DKObjectRef DKSetInitWithCArray( DKSetRef _self, DKObjectRef objects[], DKIndex count );
DKObjectRef DKSetInitWithCollection( DKSetRef _self, DKObjectRef srcCollection );

DKIndex     DKSetGetCount( DKSetRef _self );
DKObjectRef DKSetGetMember( DKSetRef _self, DKObjectRef object );
int         DKSetContainsObject( DKSetRef _self, DKObjectRef object );

void        DKSetAddObject( DKMutableSetRef _self, DKObjectRef object );

void        DKSetRemoveObject( DKMutableSetRef _self, DKObjectRef object );
void        DKSetRemoveAllObjects( DKMutableSetRef _self );

int         DKSetIsSubsetOfSet( DKSetRef _self, DKSetRef otherSet );
int         DKSetIntersectsSet( DKSetRef _self, DKSetRef otherSet );

void        DKSetUnion( DKMutableSetRef _self, DKSetRef otherSet );
void        DKSetMinus( DKMutableSetRef _self, DKSetRef otherSet );
void        DKSetIntersect( DKMutableSetRef _self, DKSetRef otherSet );




#endif // _DK_SET_H_

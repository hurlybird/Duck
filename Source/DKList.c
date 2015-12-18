/*****************************************************************************************

  DKList.c

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

#include "DKList.h"
#include "DKArray.h"
#include "DKLinkedList.h"
#include "DKString.h"
#include "DKStream.h"
#include "DKComparison.h"
#include "DKSet.h"


DKThreadSafeFastSelectorInit( List );


struct DKList
{
    const DKObject * _obj;
};


///
//  DKListClass()
//
static DKClassRef DefaultListClass = NULL;

DKClassRef DKListClass( void )
{
    if( DefaultListClass )
        return DefaultListClass;
    
    return DKArrayClass();
}

void DKSetDefaultListClass( DKClassRef cls )
{
    DefaultListClass = cls;
}


///
//  DKMutableListClass()
//
static DKClassRef DefaultMutableListClass = NULL;

DKClassRef DKMutableListClass( void )
{
    if( DefaultMutableListClass )
        return DefaultMutableListClass;
    
    return DKMutableArrayClass();
}

void DKSetDefaultMutableListClass( DKClassRef cls )
{
    DefaultMutableListClass = cls;
}


///
//  DKListInitWithObject()
//
DKObjectRef DKListInitWithObject( DKListRef _self, DKObjectRef object )
{
    return DKListInitWithCArray( _self, &object, 1 );
}


///
//  DKListInitWithObjects()
//
DKObjectRef DKListInitWithObjects( DKListRef _self, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, _self );
    
    _self = DKListInitWithVAObjects( _self, arg_ptr );
    
    va_end( arg_ptr );
    
    return _self;
}


///
//  DKListInitWithVAObjects()
//
DKObjectRef DKListInitWithVAObjects( DKListRef _self, va_list objects )
{
    if( _self )
    {
        DKListInterfaceRef listInterface = DKGetInterface( _self, DKSelector(List) );
        _self = listInterface->initWithVAObjects( _self, objects );
    }
    
    return _self;
}


///
//  DKListInitWithCArray()
//
DKObjectRef DKListInitWithCArray( DKListRef _self, DKObjectRef objects[], DKIndex count )
{
    if( _self )
    {
        DKListInterfaceRef listInterface = DKGetInterface( _self, DKSelector(List) );
        _self = listInterface->initWithCArray( _self, objects, count );
    }
    
    return _self;
}


///
//  DKListInitWithCollection()
//
DKObjectRef DKListInitWithCollection( DKListRef _self, DKObjectRef srcCollection )
{
    if( _self )
    {
        DKListInterfaceRef listInterface = DKGetInterface( _self, DKSelector(List) );
        _self = listInterface->initWithCollection( _self, srcCollection );
    }
    
    return _self;
}


///
//  DKListInitSetWithObjects()
//
DKObjectRef DKListInitSetWithObjects( DKListRef _self, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, _self );
    
    _self = DKListInitSetWithVAObjects( _self, arg_ptr );
    
    va_end( arg_ptr );
    
    return _self;
}


///
//  DKListInitSetWithVAObjects()
//
DKObjectRef DKListInitSetWithVAObjects( DKListRef _self, va_list objects )
{
    if( _self )
    {
        DKSetRef set = DKSetInitWithVAObjects( DKAlloc( DKSetClass() ), objects );

        DKListInterfaceRef listInterface = DKGetInterface( _self, DKSelector(List) );
        _self = listInterface->initWithCollection( _self, set );
        
        DKRelease( set );
    }
    
    return _self;
}


///
//  DKListInitSetWithCArray()
//
DKObjectRef DKListInitSetWithCArray( DKListRef _self, DKObjectRef objects[], DKIndex count )
{
    if( _self )
    {
        DKSetRef set = DKSetInitWithCArray( DKAlloc( DKSetClass() ), objects, count );

        DKListInterfaceRef listInterface = DKGetInterface( _self, DKSelector(List) );
        _self = listInterface->initWithCollection( _self, set );
        
        DKRelease( set );
    }
    
    return _self;
}


///
//  DKListInitSetWithCollection()
//
DKObjectRef DKListInitSetWithCollection( DKListRef _self, DKObjectRef srcCollection )
{
    if( _self )
    {
        DKSetRef set = DKSetInitWithCollection( DKAlloc( DKSetClass() ), srcCollection );

        DKListInterfaceRef listInterface = DKGetInterface( _self, DKSelector(List) );
        _self = listInterface->initWithCollection( _self, set );
        
        DKRelease( set );
    }
    
    return _self;
}


///
//  DKListGetCount()
//
DKIndex DKListGetCount( DKListRef _self )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        return list->getCount( _self );
    }
    
    return 0;
}


///
//  DKListGetCountOfObject()
//
DKIndex DKListGetCountOfObject( DKListRef _self, DKObjectRef object )
{
    DKIndex count = 0;

    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        
        DKIndex n = list->getCount( _self );
        
        for( DKIndex i = 0; i < n; i++ )
        {
            DKObjectRef object2 = list->getObjectAtIndex( _self, i );
            
            if( DKEqual( object, object2 ) )
                count++;
        }
    }
    
    return count;
}


///
//  DKListGetFirstIndexOfObject()
//
DKIndex DKListGetFirstIndexOfObject( DKListRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
    
        DKIndex n = list->getCount( _self );
        
        for( DKIndex i = 0; i < n; ++i )
        {
            DKObjectRef object2 = list->getObjectAtIndex( _self, i );
            
            if( DKEqual( object, object2 ) )
                return i;
        }
    }
    
    return DKNotFound;
}


///
//  DKListGetLastIndexOfObject()
//
DKIndex DKListGetLastIndexOfObject( DKListRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
    
        DKIndex n = list->getCount( _self );
        
        for( DKIndex i = n - 1; i >= 0; --i )
        {
            DKObjectRef object2 = list->getObjectAtIndex( _self, i );
            
            if( DKEqual( object, object2 ) )
                return i;
        }
    }
    
    return DKNotFound;
}


///
//  DKListGetObjectAtIndex()
//
DKObjectRef DKListGetObjectAtIndex( DKListRef _self, DKIndex index )
{
    DKObjectRef object = NULL;

    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        return list->getObjectAtIndex( _self, index );
    }
    
    return object;
}


///
//  DKListGetObjectsInRange()
//
DKIndex DKListGetObjectsInRange( DKListRef _self, DKRange range, DKObjectRef objects[] )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        return list->getObjectsInRange( _self, range, objects );
    }
    
    return 0;
}


///
//  DKListAppendObject()
//
void DKListAppendObject( DKMutableListRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        list->appendCArray( _self, &object, 1 );
    }
}


///
//  DKListAppendCollection()
//
void DKListAppendCollection( DKMutableListRef _self, DKObjectRef srcCollection )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        list->appendCollection( _self, srcCollection );
    }
}


///
//  DKListSetObjectAtIndex()
//
void DKListSetObjectAtIndex( DKMutableListRef _self, DKObjectRef object, DKIndex index )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        DKRange replace = DKRangeMake( index, 1 );
        list->replaceRangeWithCArray( _self, replace, &object, 1 );
    }
}


///
//  DKListInsertObjectAtIndex()
//
void DKListInsertObjectAtIndex( DKMutableListRef _self, DKObjectRef object, DKIndex index )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        DKRange insert = DKRangeMake( index, 0 );
        list->replaceRangeWithCArray( _self, insert, &object, 1 );
    }
}


///
//  DKListReplaceRangeWithCArray()
//
void DKListReplaceRangeWithCArray( DKMutableListRef _self, DKRange range, DKObjectRef objects[], DKIndex count )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        list->replaceRangeWithCArray( _self, range, objects, count );
    }
}


///
//  DKListReplaceRangeWithCollection()
//
void DKListReplaceRangeWithCollection( DKMutableListRef _self, DKRange range, DKObjectRef srcCollection )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        list->replaceRangeWithCollection( _self, range, srcCollection );
    }
}


///
//  DKListContainsObject()
//
bool DKListContainsObject( DKListRef _self, DKObjectRef object )
{
    return DKListGetFirstIndexOfObject( _self, object ) != DKNotFound;
}


///
//  DKListAddObjectToSet()
//
void DKListAddObjectToSet( DKMutableListRef _self, DKObjectRef object )
{
    if( DKListGetFirstIndexOfObject( _self, object ) == DKNotFound )
        DKListAppendObject( _self, object );
}


///
//  DKListGetMemberOfSet()
//
DKObjectRef DKListGetMemberOfSet( DKListRef _self, DKObjectRef object )
{
    DKIndex i = DKListGetFirstIndexOfObject( _self, object );
    
    if( i != DKNotFound )
        return DKListGetObjectAtIndex( _self, i );
    
    return NULL;
}


///
//  DKListRemoveObject()
//
void DKListRemoveObject( DKMutableListRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
    
        DKIndex count = list->getCount( _self );
        
        for( DKIndex i = 0; i < count; )
        {
            DKObjectRef object2 = list->getObjectAtIndex( _self, i );
            
            if( DKEqual( object, object2 ) )
            {
                list->replaceRangeWithCArray( _self, DKRangeMake( i, 1 ), NULL, 0 );
                --count;
            }
            
            else
            {
                ++i;
            }
        }
    }
}


///
//  DKListRemoveObjectAtIndex()
//
void DKListRemoveObjectAtIndex( DKMutableListRef _self, DKIndex index )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        DKRange remove = DKRangeMake( index, 1 );
        list->replaceRangeWithCArray( _self, remove, NULL, 0 );
    }
}


///
//  DKListRemoveAllObjects()
//
void DKListRemoveAllObjects( DKMutableListRef _self )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        DKRange remove = DKRangeMake( 0, list->getCount( _self ) );
        list->replaceRangeWithCArray( _self, remove, NULL, 0 );
    }
}


///
//  DKListEqual()
//
bool DKListEqual( DKListRef _self, DKListRef other )
{
    if( _self )
    {
        DKListInterfaceRef list1 = DKGetInterface( _self, DKSelector(List) );
        
        DKListInterfaceRef list2;
        
        if( DKQueryInterface( other, DKSelector(List), (DKInterfaceRef *)&list2 ) )
        {
            DKIndex count = list1->getCount( _self );
            
            if( list2->getCount( other ) == count )
            {
                for( DKIndex i = 0; i < count; ++i )
                {
                    DKObjectRef obj1 = list1->getObjectAtIndex( _self, i );
                    DKObjectRef obj2 = list2->getObjectAtIndex( other, i );
                    
                    if( !DKEqual( obj1, obj2 ) )
                        return false;
                }
                
                return true;
            }
        }
    }
    
    return false;
}


///
//  DKListCompare()
//
int DKListCompare( DKListRef _self, DKListRef other )
{
    if( _self )
    {
        DKListInterfaceRef list1 = DKGetInterface( _self, DKSelector(List) );
        
        DKListInterfaceRef list2;
        
        if( DKQueryInterface( other, DKSelector(List), (DKInterfaceRef *)&list2 ) )
        {
            DKIndex count1 = list1->getCount( _self );
            DKIndex count2 = list2->getCount( _self );
            
            DKIndex count = (count1 < count2) ? count1 : count2;
            
            for( DKIndex i = 0; i < count; ++i )
            {
                DKObjectRef obj1 = list1->getObjectAtIndex( _self, i );
                DKObjectRef obj2 = list2->getObjectAtIndex( other, i );
                
                int cmp = DKCompare( obj1, obj2 );
                
                if( cmp != 0 )
                    return cmp;
            }
            
            if( count1 < count2 )
                return -1;
            
            else if( count1 > count2 )
                return 1;
            
            return 0;
        }
    }
    
    return DKPointerCompare( _self, other );
}


///
//  DKListSort()
//
void DKListSort( DKMutableListRef _self, DKCompareFunction cmp )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        list->sort( _self, cmp );
    }
}


///
//  DKListShuffle()
//
void DKListShuffle( DKMutableListRef _self )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        list->shuffle( _self );
    }
}







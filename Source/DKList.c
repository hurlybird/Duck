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
#include "DKLinkedList.h"
#include "DKString.h"
#include "DKStream.h"


DKThreadSafeFastSelectorInit( List );


///
//  DKListClass()
//
static DKClassRef DefaultListClass = NULL;

DKClassRef DKListClass( void )
{
    if( DefaultListClass )
        return DefaultListClass;
    
    return DKLinkedListClass();
}

void DKSetDefaultListClass( DKClassRef _self )
{
    DefaultListClass = _self;
}


///
//  DKMutableListClass()
//
static DKClassRef DefaultMutableListClass = NULL;

DKClassRef DKMutableListClass( void )
{
    if( DefaultMutableListClass )
        return DefaultMutableListClass;
    
    return DKMutableLinkedListClass();
}

void DKSetDefaultMutableListClass( DKClassRef _self )
{
    DefaultMutableListClass = _self;
}


///
//  DKListCreateWithObject()
//
DKObjectRef DKListCreateWithObject( DKClassRef _class, DKObjectRef object )
{
    if( _class == NULL )
        _class = DKListClass();
    
    DKListInterfaceRef list = DKGetInterface( _class, DKSelector(List) );

    return list->createWithCArray( _class, NULL, 0 );
}


///
//  DKListCreateWithObjects()
//
DKObjectRef DKListCreateWithObjects( DKClassRef _class, DKObjectRef firstObject, ... )
{
    if( _class == NULL )
        _class = DKListClass();
    
    DKListInterfaceRef list = DKGetInterface( _class, DKSelector(List) );
    
    va_list arg_ptr;
    va_start( arg_ptr, firstObject );
    
    DKObjectRef obj = list->createWithVAObjects( _class, arg_ptr );
    
    va_end( arg_ptr );
    
    return obj;
}


///
//  DKListCreateWithCArray()
//
DKObjectRef DKListCreateWithCArray( DKClassRef _class, DKObjectRef objects[], DKIndex count )
{
    if( _class == NULL )
        _class = DKListClass();
    
    DKListInterfaceRef list = DKGetInterface( _class, DKSelector(List) );

    return list->createWithCArray( _class, objects, count );
}


///
//  DKListCreateWithCollection()
//
DKObjectRef DKListCreateWithCollection( DKClassRef _class, DKObjectRef srcCollection )
{
    if( _class == NULL )
        _class = DKListClass();
    
    DKListInterfaceRef list = DKGetInterface( _class, DKSelector(List) );

    return list->createWithCollection( _class, srcCollection );
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
void DKListSetObjectAtIndex( DKMutableListRef _self, DKIndex index, DKObjectRef object )
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
void DKListInsertObjectAtIndex( DKMutableListRef _self, DKIndex index, DKObjectRef object )
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







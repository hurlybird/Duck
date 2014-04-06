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
            DKObjectRef object2;
            
            list->getObjects( _self, DKRangeMake( i, 1 ), &object2 );
            
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
            DKObjectRef object2;
            
            list->getObjects( _self, DKRangeMake( i, 1 ), &object2 );
            
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
            DKObjectRef object2;
            
            list->getObjects( _self, DKRangeMake( i, 1 ), &object2 );
            
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
        list->getObjects( _self, DKRangeMake( index, 1 ), &object );
    }
    
    return object;
}


///
//  DKListGetObjects()
//
DKIndex DKListGetObjects( DKListRef _self, DKRange range, DKObjectRef objects[] )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        return list->getObjects( _self, range, objects );
    }
    
    return 0;
}


///
//  DKListApplyFunction()
//
int DKListApplyFunction( DKListRef _self, DKListApplierFunction callback, void * context )
{
    int result = 0;
    
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        
        DKIndex n = list->getCount( _self );
        
        for( DKIndex i = 0; i < n; ++i )
        {
            DKObjectRef object;
            
            list->getObjects( _self, DKRangeMake( i, 1 ), &object );

            if( (result = callback( object, context )) != 0 )
                break;
        }
    }
    
    return result;
}


///
//  DKListAppendObject()
//
void DKListAppendObject( DKMutableListRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        DKRange append = DKRangeMake( list->getCount( _self ), 0 );
        list->replaceObjects( _self, append, &object, 1 );
    }
}


///
//  DKListAppendList()
//
void DKListAppendList( DKMutableListRef _self, DKListRef srcList )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        DKRange append = DKRangeMake( list->getCount( _self ), 0 );
        list->replaceObjectsWithList( _self, append, srcList );
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
        list->replaceObjects( _self, replace, &object, 1 );
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
        list->replaceObjects( _self, insert, &object, 1 );
    }
}


///
//  DKListReplaceObjects()
//
void DKListReplaceObjects( DKMutableListRef _self, DKRange range, DKObjectRef objects[], DKIndex count )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        list->replaceObjects( _self, range, objects, count );
    }
}


///
//  DKListReplaceObjectsWithList()
//
void DKListReplaceObjectsWithList( DKMutableListRef _self, DKRange range, DKListRef srcList )
{
    if( _self )
    {
        DKListInterfaceRef list = DKGetInterface( _self, DKSelector(List) );
        list->replaceObjectsWithList( _self, range, srcList );
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
        list->replaceObjects( _self, remove, NULL, 0 );
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
        list->replaceObjects( _self, remove, NULL, 0 );
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


///
//  DKListCopyDescription()
//
struct PrintDescriptionContext
{
    DKMutableObjectRef stream;
    DKIndex n;
};

static int PrintDescriptionCallback( DKObjectRef object, void * context )
{
    struct PrintDescriptionContext * ctx = context;

    const char * prefix = (ctx->n == 0) ? "\n    " : ",\n    ";

    if( DKIsKindOfClass( object, DKStringClass() ) )
        DKSPrintf( ctx->stream, "%s\"%@\"", prefix, object );
    
    else
        DKSPrintf( ctx->stream, "%s%@", prefix, object );
    
    ctx->n++;
    
    return 0;
}

DKStringRef DKListCopyDescription( DKListRef _self )
{
    DKMutableStringRef desc = DKStringCreateMutable();
    
    DKIndex count = DKListGetCount( _self );
    
    DKSPrintf( desc, "%@, %d objects = [", DKGetClassName( _self ), count );
    
    struct PrintDescriptionContext context = { desc, 0 };
    DKListApplyFunction( _self, PrintDescriptionCallback, &context );
    
    DKSPrintf( desc, "\n]\n" );
    
    return desc;
}









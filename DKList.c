//
//  DKList.c
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#include "DKList.h"
#include "DKLinkedList.h"


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


///
//  DKSetListClass()
//
void DKSetListClass( DKClassRef _self )
{
    DefaultListClass = _self;
}


///
//  DKListGetCount()
//
DKIndex DKListGetCount( DKListRef _self )
{
    if( _self )
    {
        DKList * list = DKGetInterface( _self, DKSelector(List) );
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
    
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
    
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
    
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
        
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
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
        DKList * list = DKGetInterface( _self, DKSelector(List) );
        list->shuffle( _self );
    }
}











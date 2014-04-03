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
void DKSetListClass( DKClassRef ref )
{
    DefaultListClass = ref;
}


///
//  DKListGetCount()
//
DKIndex DKListGetCount( DKListRef ref )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        return list->getCount( ref );
    }
    
    return 0;
}


///
//  DKListGetCountOfObject()
//
DKIndex DKListGetCountOfObject( DKListRef ref, DKObjectRef object )
{
    DKIndex count = 0;

    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
    
        DKIndex n = list->getCount( ref );
        
        for( DKIndex i = 0; i < n; i++ )
        {
            DKObjectRef object2;
            
            list->getObjects( ref, DKRangeMake( i, 1 ), &object2 );
            
            if( DKEqual( object, object2 ) )
                count++;
        }
    }
    
    return count;
}


///
//  DKListGetFirstIndexOfObject()
//
DKIndex DKListGetFirstIndexOfObject( DKListRef ref, DKObjectRef object )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
    
        DKIndex n = list->getCount( ref );
        
        for( DKIndex i = 0; i < n; ++i )
        {
            DKObjectRef object2;
            
            list->getObjects( ref, DKRangeMake( i, 1 ), &object2 );
            
            if( DKEqual( object, object2 ) )
                return i;
        }
    }
    
    return DKNotFound;
}


///
//  DKListGetLastIndexOfObject()
//
DKIndex DKListGetLastIndexOfObject( DKListRef ref, DKObjectRef object )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
    
        DKIndex n = list->getCount( ref );
        
        for( DKIndex i = n - 1; i >= 0; --i )
        {
            DKObjectRef object2;
            
            list->getObjects( ref, DKRangeMake( i, 1 ), &object2 );
            
            if( DKEqual( object, object2 ) )
                return i;
        }
    }
    
    return DKNotFound;
}


///
//  DKListGetObjectAtIndex()
//
DKObjectRef DKListGetObjectAtIndex( DKListRef ref, DKIndex index )
{
    DKObjectRef object = NULL;

    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        list->getObjects( ref, DKRangeMake( index, 1 ), &object );
    }
    
    return object;
}


///
//  DKListGetObjects()
//
DKIndex DKListGetObjects( DKListRef ref, DKRange range, DKObjectRef objects[] )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        return list->getObjects( ref, range, objects );
    }
    
    return 0;
}


///
//  DKListApplyFunction()
//
int DKListApplyFunction( DKListRef ref, DKListApplierFunction callback, void * context )
{
    int result = 0;
    
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        
        DKIndex n = list->getCount( ref );
        
        for( DKIndex i = 0; i < n; ++i )
        {
            DKObjectRef object;
            
            list->getObjects( ref, DKRangeMake( i, 1 ), &object );

            if( (result = callback( object, context )) != 0 )
                break;
        }
    }
    
    return result;
}


///
//  DKListAppendObject()
//
void DKListAppendObject( DKMutableListRef ref, DKObjectRef object )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        DKRange append = DKRangeMake( list->getCount( ref ), 0 );
        list->replaceObjects( ref, append, &object, 1 );
    }
}


///
//  DKListAppendList()
//
void DKListAppendList( DKMutableListRef ref, DKListRef srcList )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        DKRange append = DKRangeMake( list->getCount( ref ), 0 );
        list->replaceObjectsWithList( ref, append, srcList );
    }
}


///
//  DKListSetObjectAtIndex()
//
void DKListSetObjectAtIndex( DKMutableListRef ref, DKIndex index, DKObjectRef object )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        DKRange replace = DKRangeMake( index, 1 );
        list->replaceObjects( ref, replace, &object, 1 );
    }
}


///
//  DKListInsertObjectAtIndex()
//
void DKListInsertObjectAtIndex( DKMutableListRef ref, DKIndex index, DKObjectRef object )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        DKRange insert = DKRangeMake( index, 0 );
        list->replaceObjects( ref, insert, &object, 1 );
    }
}


///
//  DKListReplaceObjects()
//
void DKListReplaceObjects( DKMutableListRef ref, DKRange range, DKObjectRef objects[], DKIndex count )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        list->replaceObjects( ref, range, objects, count );
    }
}


///
//  DKListReplaceObjectsWithList()
//
void DKListReplaceObjectsWithList( DKMutableListRef ref, DKRange range, DKListRef srcList )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        list->replaceObjectsWithList( ref, range, srcList );
    }
}


///
//  DKListRemoveObjectAtIndex()
//
void DKListRemoveObjectAtIndex( DKMutableListRef ref, DKIndex index )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        DKRange remove = DKRangeMake( index, 1 );
        list->replaceObjects( ref, remove, NULL, 0 );
    }
}


///
//  DKListRemoveAllObjects()
//
void DKListRemoveAllObjects( DKMutableListRef ref )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        DKRange remove = DKRangeMake( 0, list->getCount( ref ) );
        list->replaceObjects( ref, remove, NULL, 0 );
    }
}


///
//  DKListSort()
//
void DKListSort( DKMutableListRef ref, DKCompareFunction cmp )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        list->sort( ref, cmp );
    }
}


///
//  DKListShuffle()
//
void DKListShuffle( DKMutableListRef ref )
{
    if( ref )
    {
        DKList * list = DKGetInterface( ref, DKSelector(List) );
        list->shuffle( ref );
    }
}











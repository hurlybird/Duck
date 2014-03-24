//
//  DKList.c
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#include "DKList.h"
#include "DKLinkedList.h"


DKDefineFastLookupInterface( List );


///
//  DKListClass()
//
static DKTypeRef DefaultListClass = NULL;

DKTypeRef DKListClass( void )
{
    if( DefaultListClass )
        return DefaultListClass;
    
    return DKLinkedListClass();
}


///
//  DKSetListClass()
//
void DKSetListClass( DKTypeRef ref )
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
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
        return list->getCount( ref );
    }
    
    return 0;
}


///
//  DKListGetCountOfObject()
//
DKIndex DKListGetCountOfObject( DKListRef ref, DKTypeRef object )
{
    DKIndex count = 0;

    if( ref )
    {
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
    
        DKIndex n = list->getCount( ref );
        
        for( DKIndex i = 0; i < n; i++ )
        {
            DKTypeRef object2;
            
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
DKIndex DKListGetFirstIndexOfObject( DKListRef ref, DKTypeRef object )
{
    if( ref )
    {
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
    
        DKIndex n = list->getCount( ref );
        
        for( DKIndex i = 0; i < n; ++i )
        {
            DKTypeRef object2;
            
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
DKIndex DKListGetLastIndexOfObject( DKListRef ref, DKTypeRef object )
{
    if( ref )
    {
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
    
        DKIndex n = list->getCount( ref );
        
        for( DKIndex i = n - 1; i >= 0; --i )
        {
            DKTypeRef object2;
            
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
DKTypeRef DKListGetObjectAtIndex( DKListRef ref, DKIndex index )
{
    DKTypeRef object = NULL;

    if( ref )
    {
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
        list->getObjects( ref, DKRangeMake( index, 1 ), &object );
    }
    
    return object;
}


///
//  DKListGetObjects()
//
DKIndex DKListGetObjects( DKListRef ref, DKRange range, DKTypeRef objects[] )
{
    if( ref )
    {
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
        return list->getObjects( ref, range, objects );
    }
    
    return 0;
}


///
//  DKListApplyFunction()
//
void DKListApplyFunction( DKListRef ref, DKListApplierFunction callback, void * context )
{
    if( ref )
    {
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
        
        DKIndex n = list->getCount( ref );
        
        for( DKIndex i = 0; i < n; ++i )
        {
            DKTypeRef object;
            
            list->getObjects( ref, DKRangeMake( i, 1 ), &object );

            callback( object, context );
        }
    }
}


///
//  DKListAppendObject()
//
void DKListAppendObject( DKMutableListRef ref, DKTypeRef object )
{
    if( ref )
    {
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
        DKRange append = DKRangeMake( list->getCount( ref ), 0 );
        list->replaceObjects( ref, append, &object, 1 );
    }
}


///
//  DKListAppendList()
//
void DKListAppendList( DKMutableListRef ref, DKTypeRef srcList )
{
    if( ref )
    {
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
        DKRange append = DKRangeMake( list->getCount( ref ), 0 );
        list->replaceObjectsWithList( ref, append, srcList );
    }
}


///
//  DKListSetObjectAtIndex()
//
void DKListSetObjectAtIndex( DKMutableListRef ref, DKIndex index, DKTypeRef object )
{
    if( ref )
    {
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
        DKRange replace = DKRangeMake( index, 1 );
        list->replaceObjects( ref, replace, &object, 1 );
    }
}


///
//  DKListInsertObjectAtIndex()
//
void DKListInsertObjectAtIndex( DKMutableListRef ref, DKIndex index, DKTypeRef object )
{
    if( ref )
    {
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
        DKRange insert = DKRangeMake( index, 0 );
        list->replaceObjects( ref, insert, &object, 1 );
    }
}


///
//  DKListReplaceObjects()
//
void DKListReplaceObjects( DKMutableListRef ref, DKRange range, DKTypeRef objects[], DKIndex count )
{
    if( ref )
    {
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
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
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
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
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
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
        DKList * list = DKLookupInterface( ref, DKSelector(List) );
        DKRange remove = DKRangeMake( 0, list->getCount( ref ) );
        list->replaceObjects( ref, remove, NULL, 0 );
    }
}











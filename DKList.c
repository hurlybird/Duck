//
//  DKList.c
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#include "DKList.h"


DKDefineInterface( DKList );


///
//  DKListObjectCallbacks()
//
static const DKListCallbacks __DKListObjectCallbacks__ =
{
    DKRetain,
    DKRelease,
    DKEqual
};

const DKListCallbacks * DKListObjectCallbacks( void )
{
    return &__DKListObjectCallbacks__;
}


///
//  DKListSUIDCallbacks()
//
static const DKListCallbacks __DKListStringCallbacks__ =
{
    DKDoNothingRetain,
    DKDoNothingRelease,
    DKStrEqual
};

const DKListCallbacks * DKListStringCallbacks( void )
{
    return &__DKListStringCallbacks__;
}


///
//  DKListSUIDCallbacks()
//
static const DKListCallbacks __DKListIndexCallbacks__ =
{
    DKDoNothingRetain,
    DKDoNothingRelease,
    DKPtrEqual
};

const DKListCallbacks * DKListIndexCallbacks( void )
{
    return &__DKListIndexCallbacks__;
}


///
//  DKListGetCount()
//
DKIndex DKListGetCount( DKListRef ref )
{
    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        return list->getCount( ref );
    }
    
    return 0;
}


///
//  DKListGetCountOfValue()
//
DKIndex DKListGetCountOfValue( DKListRef ref, const void * value )
{
    DKIndex count = 0;

    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        const DKListCallbacks * callbacks = list->getCallbacks( ref );
    
        DKIndex n = list->getCount( ref );
        
        for( DKIndex i = 0; i < n; i++ )
        {
            const void * value2;
            
            list->getValues( ref, DKRangeMake( i, 1 ), &value2 );
            
            if( callbacks->equal( value, value2 ) )
                count++;
        }
    }
    
    return count;
}


///
//  DKListGetFirstIndexOfValue()
//
DKIndex DKListGetFirstIndexOfValue( DKListRef ref, const void * value )
{
    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        const DKListCallbacks * callbacks = list->getCallbacks( ref );
    
        DKIndex n = list->getCount( ref );
        
        for( DKIndex i = 0; i < n; ++i )
        {
            const void * value2;
            
            list->getValues( ref, DKRangeMake( i, 1 ), &value2 );
            
            if( callbacks->equal( value, value2 ) )
                return i;
        }
    }
    
    return DKNotFound;
}


///
//  DKListGetLastIndexOfValue()
//
DKIndex DKListGetLastIndexOfValue( DKListRef ref, const void * value )
{
    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        const DKListCallbacks * callbacks = list->getCallbacks( ref );
    
        DKIndex n = list->getCount( ref );
        
        for( DKIndex i = n - 1; i >= 0; --i )
        {
            const void * value2;
            
            list->getValues( ref, DKRangeMake( i, 1 ), &value2 );
            
            if( callbacks->equal( value, value2 ) )
                return i;
        }
    }
    
    return DKNotFound;
}


///
//  DKListGetValueAtIndex()
//
const void * DKListGetValueAtIndex( DKListRef ref, DKIndex index )
{
    const void * value = NULL;

    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        list->getValues( ref, DKRangeMake( index, 1 ), &value );
    }
    
    return value;
}


///
//  DKListGetValues()
//
DKIndex DKListGetValues( DKListRef ref, DKRange range, const void ** values )
{
    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        return list->getValues( ref, range, values );
    }
    
    return 0;
}


///
//  DKListApplyFunction()
//
void DKListApplyFunction( DKListRef ref, DKListApplierFunction callback, void * context )
{
    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        DKIndex n = list->getCount( ref );
        
        for( DKIndex i = 0; i < n; ++i )
        {
            const void * value;
            
            list->getValues( ref, DKRangeMake( i, 1 ), &value );

            callback( value, context );
        }
    }
}


///
//  DKListAppendValue()
//
void DKListAppendValue( DKMutableListRef ref, const void * value )
{
    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        DKRange append = DKRangeMake( list->getCount( ref ), 0 );
        list->replaceValues( ref, append, &value, 1 );
    }
}


///
//  DKListSetValueAtIndex()
//
void DKListSetValueAtIndex( DKMutableListRef ref, DKIndex index, const void * value )
{
    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        DKRange replace = DKRangeMake( index, 1 );
        list->replaceValues( ref, replace, &value, 1 );
    }
}


///
//  DKListInsertValueAtIndex()
//
void DKListInsertValueAtIndex( DKMutableListRef ref, DKIndex index, const void * value )
{
    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        DKRange insert = DKRangeMake( index, 0 );
        list->replaceValues( ref, insert, &value, 1 );
    }
}


///
//  DKListReplaceValues()
//
void DKListReplaceValues( DKMutableListRef ref, DKRange range, const void ** values, DKIndex count )
{
    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        list->replaceValues( ref, range, values, count );
    }
}


///
//  DKListReplaceValuesWithList()
//
void DKListReplaceValuesWithList( DKMutableListRef ref, DKRange range, DKListRef srcList )
{
    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        list->replaceValuesWithList( ref, range, srcList );
    }
}


///
//  DKListRemoveValueAtIndex()
//
void DKListRemoveValueAtIndex( DKMutableListRef ref, DKIndex index )
{
    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        DKRange remove = DKRangeMake( index, 1 );
        list->replaceValues( ref, remove, NULL, 0 );
    }
}


///
//  DKListRemoveAllValues()
//
void DKListRemoveAllValues( DKMutableListRef ref )
{
    DKListInterfaceRef list = DKGetInterface( ref, DKSelector( DKList ) );
    
    if( list )
    {
        DKRange remove = DKRangeMake( 0, list->getCount( ref ) );
        list->replaceValues( ref, remove, NULL, 0 );
    }
}











//
//  DKElementArray.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKElementArray.h"


///
//  DKElementArrayInit()
//
void DKElementArrayInit( DKElementArray * array, DKIndex elementSize )
{
    DKByteArrayInit( &array->byteArray );
    array->elementSize = elementSize;
}


///
//  DKElementArrayReserve()
//
void DKElementArrayReserve( DKElementArray * array, DKIndex length )
{
    DKByteArrayReserve( &array->byteArray, length * array->elementSize );
}


///
//  DKElementArrayClear()
void DKElementArrayClear( DKElementArray * array )
{
    DKByteArrayClear( &array->byteArray );
}


///
//  DKElementArrayReplaceElements()
//
void DKElementArrayReplaceElements( DKElementArray * array, DKRange range, const void * elements, DKIndex length )
{
    DKRange byteRange = DKRangeMake( range.location * array->elementSize, range.length * array->elementSize );
    DKIndex byteLength = length * array->elementSize;
    
    DKByteArrayReplaceBytes( &array->byteArray, byteRange, elements, byteLength );
}


///
//  DKElementArrayGetCount()
//
DKIndex DKElementArrayGetCount( const DKElementArray * array )
{
    return array->byteArray.length / array->elementSize;
}


///
//  DKElementArrayAppendElement()
//
void DKElementArrayAppendElement( DKElementArray * array, const void * element )
{
    DKRange byteRange = DKRangeMake( array->byteArray.length, 0 );
    DKByteArrayReplaceBytes( &array->byteArray, byteRange, element, array->elementSize );
}


///
//  DKElementArrayInsertElementAtIndex()
//
void DKElementArrayInsertElementAtIndex( DKElementArray * array, DKIndex index, const void * element )
{
    DKRange byteRange = DKRangeMake( index * array->elementSize, 0 );
    DKByteArrayReplaceBytes( &array->byteArray, byteRange, element, array->elementSize );
}


///
//  DKElementArraySetElementAtIndex()
//
void DKElementArraySetElementAtIndex( DKElementArray * array, DKIndex index, const void * element )
{
    DKRange byteRange = DKRangeMake( index * array->elementSize, array->elementSize );
    DKByteArrayReplaceBytes( &array->byteArray, byteRange, element, array->elementSize );
}


///
//  DKElementArrayRemoveElementAtIndex()
//
void DKElementArrayRemoveElementAtIndex( DKElementArray * array, DKIndex index )
{
    DKRange byteRange = DKRangeMake( index * array->elementSize, array->elementSize );
    DKByteArrayReplaceBytes( &array->byteArray, byteRange, NULL, 0 );
}





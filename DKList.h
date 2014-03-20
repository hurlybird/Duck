//
//  DKList.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-09.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_LIST_H_
#define _DK_LIST_H_

#include "DKObject.h"


DKDeclareSUID( DKListInterfaceID );


typedef const void * DKListRef;
typedef void * DKMutableListRef;


typedef void (*DKListApplierFunction)( const void * value, void * context );

typedef const void * (* DKListRetainCallback)( const void * value );
typedef void (* DKListReleaseCallback)( const void * value );
typedef int  (* DKListEqualCallback)( const void * value1, const void * value2 );


typedef struct
{
    DKListRetainCallback retain;
    DKListReleaseCallback release;
    DKListEqualCallback equal;
    
} DKListCallbacks;

const DKListCallbacks * DKListObjectCallbacks( void );
const DKListCallbacks * DKListStringCallbacks( void );
const DKListCallbacks * DKListIndexCallbacks( void );


typedef struct
{
    const DKInterface _interface;

    const DKListCallbacks * (* const getCallbacks)( DKListRef ref );

    DKIndex     (* const getCount)( DKListRef ref );
    DKIndex     (* const getValues)( DKListRef ref, DKRange range, const void ** values );
    void        (* const replaceValues)( DKMutableListRef ref, DKRange range, const void ** values, DKIndex count );
    void        (* const replaceValuesWithList)( DKMutableListRef ref, DKRange range, DKListRef srcList );

} DKListInterface;

typedef const DKListInterface * DKListInterfaceRef;


DKIndex     DKListGetCount( DKListRef ref );
DKIndex     DKListGetCountOfValue( DKListRef ref, const void * value );
DKIndex     DKListGetFirstIndexOfValue( DKListRef ref, const void * value );
DKIndex     DKListGetLastIndexOfValue( DKListRef ref, const void * value );

const void * DKListGetValueAtIndex( DKListRef ref, DKIndex index );
DKIndex     DKListGetValues( DKListRef ref, DKRange range, const void ** values );

void        DKListApplyFunction( DKListRef ref, DKListApplierFunction callback, void * context );

void        DKListAppendValue( DKMutableListRef ref, const void * value );
void        DKListSetValueAtIndex( DKMutableListRef ref, DKIndex index, const void * value );
void        DKListInsertValueAtIndex( DKMutableListRef ref, DKIndex index, const void * value );
void        DKListReplaceValues( DKMutableListRef ref, DKRange range, const void ** values, DKIndex count );
void        DKListReplaceValuesWithList( DKMutableListRef ref, DKRange range, DKListRef srcList );
void        DKListRemoveValueAtIndex( DKMutableListRef ref, DKIndex index );
void        DKListRemoveAllValues( DKMutableListRef ref );



#endif // _DK_LIST_H_

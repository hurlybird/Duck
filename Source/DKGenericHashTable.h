/*****************************************************************************************

  DKGenericHashTable.h

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

#ifndef _DK_GENERIC_HASH_TABLE_H_
#define _DK_GENERIC_HASH_TABLE_H_

#include "DKRuntime.h"


typedef enum
{
    DKRowStatusEmpty,
    DKRowStatusActive,
    DKRowStatusDeleted

} DKRowStatus;


typedef struct
{
    DKRowStatus (*rowStatus)( const void * row );
    DKHashCode  (*rowHash)( const void * row );
    bool        (*rowEqual)( const void * row1, const void * row2 );
    void        (*rowInit)( void * row );
    void        (*rowUpdate)( void * row, const void * src );
    void        (*rowDelete)( void * row );

} DKGenericHashTableCallbacks;


typedef struct
{
    uint8_t * rows;
    
    DKIndex activeCount;    // number of active rows in the table
    
    size_t rowSize;         // row size in bytes
    size_t rowCount;        // total number of rows in the table
    size_t maxActive;       // maximum number of active rows
    
    DKGenericHashTableCallbacks callbacks;
    
} DKGenericHashTable;


void DKGenericHashTableInit( DKGenericHashTable * hashTable, size_t rowSize, const DKGenericHashTableCallbacks * callbacks );
void DKGenericHashTableFinalize( DKGenericHashTable * hashTable );

#define DKGenericHashTableGetCount( table )     ((table)->activeCount)
#define DKGenericHashTableGetRow( table, i )    (const void *)((table)->rows + ((table)->rowSize * i))

const void * DKGenericHashTableFind( DKGenericHashTable * hashTable, const void * entry );
void DKGenericHashTableInsert( DKGenericHashTable * hashTable, const void * entry, DKInsertPolicy policy );
void DKGenericHashTableRemove( DKGenericHashTable * hashTable, const void * entry );
void DKGenericHashTableRemoveAll( DKGenericHashTable * hashTable );





#endif // _DK_GENERIC_HASH_TABLE_H_


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

#include "DKGenericHashTable.h"



struct HashTableSize
{
    size_t rowCount;    // prime
    size_t maxActive;   // rowCount / 2
};

static const struct HashTableSize HashTableSizes[] =
{
    { 11, 5 },
    { 23, 11 },
    { 47, 23 },
    { 97, 48 },
    { 197, 98 },
    { 397, 198 },
    { 797, 398 },
    { 1597, 798 },
    { 3203, 1601 },
    { 6421, 3210 },
    { 12853, 6426 },
    { 25717, 12858 },
    { 51437, 25718 },
    { 102877, 51438 },
    { 205759, 102879 },
    { 411527, 205763 },
    { 823117, 411558 },
    { 1646237, 823118 },
    { 3292489, 1646244 },
    { 6584983, 3292491 },
    { 13169977, 6584988 },
    { 26339969, 13169984 },
    { 52679969, 26339984 },
    { 105359939, 52679969 },
    { 210719881, 105359940 },
    { 421439783, 210719891 },
    { 842879579, 421439789 },
    { 1685759167, 842879583 },
    
    // This could be larger on 64-bit architectures...
    
    { 0, 0 }
};


// Size table generation for load < 0.5
#if 0
static bool IsPrime( int64_t x )
{
    for( int64_t i = 3; (i * i) < x; i += 2 )
    {
        if( (x % i) == 0 )
            return false;
    }
    
    return true;
}

static int64_t NextPrime( int64_t x )
{
    if( (x & 1) == 0 )
        x++;
    
    for( ; !IsPrime( x ); x += 2 )
        ;
    
    return x;
}

static void GenerateHashTableSizes( void )
{
    int64_t max = 0x7fffffff;

    for( int64_t i = 11; i <= max; )
    {
        printf( "    { %lld, %lld },\n", i, i / 2 );
        i = NextPrime( i * 2 );
    }
}
#endif


///
//  NextHashTableSize()
//
static struct HashTableSize NextHashTableSize( size_t rowCount )
{
    for( int i = 0; HashTableSizes[i].rowCount != 0; i++ )
    {
        if( HashTableSizes[i].rowCount > rowCount )
            return HashTableSizes[i];
    }
    
    DKFatalError( "DKGenericHashTable: Exceeded maximum table size (~843 million entries).\n" );

    struct HashTableSize zero = { 0, 0 };
    return zero;
}


///
//  ResizeAndRehash()
//
static void ResizeAndRehash( DKGenericHashTable * hashTable )
{
    if( hashTable->activeCount < hashTable->maxActive )
        return;
    
    uint8_t * oldRows = hashTable->rows;
    DKIndex oldRowCount = hashTable->rowCount;
    
    struct HashTableSize newSize = NextHashTableSize( hashTable->rowCount );
    
    hashTable->rowCount = newSize.rowCount;
    hashTable->maxActive = newSize.maxActive;

    hashTable->activeCount = 0;
    hashTable->rows = dk_malloc( hashTable->rowSize * hashTable->rowCount );
    
    for( DKIndex i = 0; i < hashTable->rowCount; ++i )
    {
        void * row = hashTable->rows + (hashTable->rowSize * i);
        hashTable->callbacks.rowInit( row );
    }
    
    if( oldRows )
    {
        for( DKIndex i = 0; i < oldRowCount; ++i )
        {
            void * row = oldRows + (hashTable->rowSize * i);
            
            if( hashTable->callbacks.rowStatus( row ) == DKRowStatusActive )
            {
                DKGenericHashTableInsert( hashTable, row, DKInsertAlways );
                hashTable->callbacks.rowDelete( row );
            }
        }
        
        dk_free( oldRows );
    }
}


///
//  DKGenericHashTableFind()
//
static void * Find( DKGenericHashTable * hashTable, const void * _row, DKRowStatus * outStatus )
{
    DKHashCode hash = hashTable->callbacks.rowHash( _row );

    size_t i = 0;
    size_t x = hash % hashTable->rowCount;
    
    void * firstDeletedRow = NULL;
    
    while( 1 )
    {
        void * row = hashTable->rows + (x * hashTable->rowSize);

        DKRowStatus status = hashTable->callbacks.rowStatus( row );
        
        // If the row is empty we've come to the end of the probe, so either return the
        // empty row or recycle the first deleted row we found
        if( status == DKRowStatusEmpty )
        {
            if( firstDeletedRow )
            {
                *outStatus = DKRowStatusDeleted;
                return firstDeletedRow;
            }
            
            *outStatus = DKRowStatusEmpty;
            return row;
        }
        
        // If this is the row we're looking for, return it
        else if( status == DKRowStatusActive )
        {
            if( hashTable->callbacks.rowEqual( row, _row ) )
            {
                *outStatus = DKRowStatusActive;
                return row;
            }
        }
        
        // Remember the first deleted row we find
        else if( firstDeletedRow == NULL )
        {
            firstDeletedRow = row;
        }

        // Quadratic probing
        i++;
        x += (2 * i) - 1;
        
        if( x >= hashTable->rowCount )
            x -= hashTable->rowCount;
    }
        
    DKAssert( 0 );
    return NULL;
}


///
//  DKGenericHashTableInit()
//
void DKGenericHashTableInit( DKGenericHashTable * hashTable, size_t rowSize, const DKGenericHashTableCallbacks * callbacks )
{
    hashTable->rows = NULL;
    
    hashTable->activeCount = 0;

    hashTable->rowSize = rowSize;
    hashTable->rowCount = 0;
    hashTable->maxActive = 0;
    
    hashTable->callbacks = *callbacks;
}


///
//  DKGenericHashTableFinalize()
//
void DKGenericHashTableFinalize( DKGenericHashTable * hashTable )
{
    for( DKIndex i = 0; i < hashTable->rowCount; ++i )
    {
        void * row = hashTable->rows + (hashTable->rowSize * i);

        DKRowStatus status = hashTable->callbacks.rowStatus( row );

        if( status == DKRowStatusActive )
            hashTable->callbacks.rowDelete( row );
    }
    
    dk_free( hashTable->rows );
}


///
//  DKGenericHashTableFind()
//
const void * DKGenericHashTableFind( DKGenericHashTable * hashTable, const void * entry )
{
    if( hashTable->activeCount > 0 )
    {
        DKRowStatus status;
        void * row = Find( hashTable, entry, &status );
        
        if( status == DKRowStatusActive )
            return row;
    }
    
    return NULL;
}


///
//  DKGenericHashTableInsert()
//
bool DKGenericHashTableInsert( DKGenericHashTable * hashTable, const void * entry, DKInsertPolicy policy )
{
    // Lazy table allocation
    if( hashTable->rows == NULL )
        ResizeAndRehash( hashTable );

    DKRowStatus status;
    void * row = Find( hashTable, entry, &status );

    if( status == DKRowStatusActive )
    {
        if( policy == DKInsertIfNotFound )
            return false;
        
        hashTable->callbacks.rowUpdate( row, entry );
    }
    
    else
    {
        if( policy == DKInsertIfFound )
            return false;
        
        hashTable->callbacks.rowUpdate( row, entry );

        hashTable->activeCount++;
        
        ResizeAndRehash( hashTable );
    }
    
    return true;
}


///
//  DKGenericHashTableRemove()
//
void DKGenericHashTableRemove( DKGenericHashTable * hashTable, const void * entry )
{
    if( hashTable->activeCount > 0 )
    {
        DKRowStatus status;
        void * row = Find( hashTable, entry, &status );

        if( status == DKRowStatusActive )
        {
            hashTable->callbacks.rowDelete( row );
            hashTable->activeCount--;
        }
    }
}


///
//  DKGenericHashTableRemoveAll()
//
void DKGenericHashTableRemoveAll( DKGenericHashTable * hashTable )
{
    for( DKIndex i = 0; i < hashTable->rowCount; ++i )
    {
        void * row = hashTable->rows + (hashTable->rowSize * i);

        DKRowStatus status = hashTable->callbacks.rowStatus( row );

        if( status == DKRowStatusActive )
            hashTable->callbacks.rowDelete( row );
        
        hashTable->callbacks.rowInit( row );
    }
    
    hashTable->activeCount = 0;
}








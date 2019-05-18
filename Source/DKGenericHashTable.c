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


void * DKRowStatusActive =  "DKRowStatusActive";
void * DKRowStatusEmpty =   "DKRowStatusEmpty";
void * DKRowStatusDeleted = "DKRowStatusDeleted";


struct HashTableSize
{
    size_t rowCount;    // prime
    size_t maxActive;   // rowCount / 2, load < 0.5 is required by quadratic probing
};


static const struct HashTableSize HashTableSizes[] =
{
    // 1.5x Growth
    #if 1
    { 11, 5 },
    { 17, 8 },
    { 25, 12 },
    { 61, 30 },
    { 127, 63 },
    { 191, 95 },
    { 509, 254 },
    { 1021, 510 },
    { 1531, 765 },
    { 2297, 1148 },
    { 3583, 1791 },
    { 5623, 2811 },
    { 8447, 4223 },
    { 12799, 6399 },
    { 19447, 9723 },
    { 29179, 14589 },
    { 44029, 22014 },
    { 66047, 33023 },
    { 99317, 49658 },
    { 148991, 74495 },
    { 223711, 111855 },
    { 335609, 167804 },
    { 503551, 251775 },
    { 755449, 377724 },
    { 1133303, 566651 },
    { 1700087, 850043 },
    { 2550269, 1275134 },
    { 3825649, 1912824 },
    { 5738743, 2869371 },
    { 8608247, 4304123 },
    { 12912379, 6456189 },
    { 19368703, 9684351 },
    { 29053181, 14526590 },
    { 43579873, 21789936 },
    { 65369851, 32684925 },
    { 98054897, 49027448 },
    { 147082471, 73541235 },
    { 220623863, 110311931 },
    { 330936049, 165468024 },
    { 496404217, 248202108 },
    { 744606449, 372303224 },
    { 1116909791, 558454895 },
    { 1675364863, 837682431 },
    
    // 2x Growth
    #else
    { 11, 5 },
    { 23, 11 },
    { 61, 30 },
    { 127, 63 },
    { 509, 254 },
    { 1021, 510 },
    { 2297, 1148 },
    { 4603, 2301 },
    { 9209, 4604 },
    { 18427, 9213 },
    { 36857, 18428 },
    { 73727, 36863 },
    { 147709, 73854 },
    { 295663, 147831 },
    { 591341, 295670 },
    { 1182703, 591351 },
    { 2365439, 1182719 },
    { 4731119, 2365559 },
    { 9462263, 4731131 },
    { 18924793, 9462396 },
    { 37849849, 18924924 },
    { 75699709, 37849854 },
    { 151399421, 75699710 },
    { 302799097, 151399548 },
    { 605598407, 302799203 },
    { 1211196923, 605598461 },
    #endif
    
    // This could be larger on 64-bit architectures...
    
    { 0, 0 }
};


///
//  Hash Table Size Generators
//
#if 0
static bool IsPrime( int64_t x )
{
    if( x <= 3 )
        return x != 0;

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

static void GenerateSizes( int64_t (*nextSize)( int64_t x ) )
{
    int64_t max = 0x7fffffff;

    for( int64_t i = 11; i <= max; )
    {
        printf( "    { %" PRId64 ", %" PRId64 " },\n", i, i / 2 );
        i = nextSize( i * 2 );

        // Round up the table size to multiples of N allocated bytes for better memory
        // allocation behaviour. This assumes each hash table row is 16 bytes, i.e. two
        // pointers, which is true for DKHashTable on 64-bit systems.
        size_t bytes = i * 16;
        size_t boundary = 0;
    
        if( bytes >= 4096 )
            boundary = (bytes + 4095) & ~4095;
    
        else if( bytes >= 512 )
            boundary = (bytes + 511) & ~511;
            
        while( 1 )
        {
            size_t j = nextSize( i + 1 );
            bytes = j * 16;
            
            if( bytes > boundary )
                break;
            
            i = j;
        }
    }
}

void DKGenerateHashTableSizes( void )
{
    GenerateSizes( NextPrime );
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
    if( (size_t)(hashTable->activeCount + hashTable->deletedCount) < hashTable->maxActive )
        return;
    
    uint8_t * oldRows = hashTable->rows;
    size_t oldRowCount = hashTable->rowCount;

    // Only resize the table if its more than half full
    if( (hashTable->rowCount == 0) || (hashTable->activeCount > hashTable->deletedCount) )
    {
        struct HashTableSize newSize = NextHashTableSize( hashTable->rowCount );
        
        hashTable->rowCount = newSize.rowCount;
        hashTable->maxActive = newSize.maxActive;
    }

    hashTable->activeCount = 0;
    hashTable->deletedCount = 0;
    hashTable->rows = dk_malloc( hashTable->rowSize * hashTable->rowCount );
    
    for( size_t i = 0; i < hashTable->rowCount; ++i )
    {
        void * row = hashTable->rows + (hashTable->rowSize * i);
        hashTable->callbacks.rowInit( row );
    }
    
    if( oldRows )
    {
        for( size_t i = 0; i < oldRowCount; ++i )
        {
            void * row = oldRows + (hashTable->rowSize * i);
            DKRowStatus status = hashTable->callbacks.rowStatus( row );
            
            if( DKRowIsActive( status ) )
            {
                DKGenericHashTableInsert( hashTable, row, DKInsertAlways );
                hashTable->callbacks.rowDelete( row );
            }
        }
        
        dk_free( oldRows );
    }
}


///
//  Find()
//
static void * Find( DKGenericHashTable * hashTable, const void * _row, DKRowStatus * outStatus )
{
    DKHashCode hash = hashTable->callbacks.rowHash( _row );

    size_t i = 0;
    size_t x = hash % hashTable->rowCount;
    
    void * firstDeletedRow = NULL;

    //printf( "Find: hash=%lu count=%ld active=%ld\n", hash, hashTable->rowCount, hashTable->activeCount );
    
    while( 1 )
    {
        void * row = hashTable->rows + (x * hashTable->rowSize);

        DKRowStatus status = hashTable->callbacks.rowStatus( row );

        //printf( "   i=%ld x=%ld status=%s\n", i, x, DK_HASHTABLE_ROW_STATUS_STR( status ) );
        
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
        else if( status != DKRowStatusDeleted )
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

        // The probe has failed if we hit this
        DKAssert( x < hashTable->rowCount );
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
    hashTable->deletedCount = 0;

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
    for( size_t i = 0; i < hashTable->rowCount; ++i )
    {
        void * row = hashTable->rows + (hashTable->rowSize * i);
        DKRowStatus status = hashTable->callbacks.rowStatus( row );

        if( DKRowIsActive( status ) )
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
        
        if( DKRowIsActive( status ) )
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

    if( DKRowIsActive( status ) )
    {
        if( policy == DKInsertIfNotFound )
            return false;
        
        hashTable->callbacks.rowUpdate( row, entry );
    }
    
    else
    {
        if( policy == DKInsertIfFound )
            return false;
        
        if( DKRowIsDeleted( status ) )
            hashTable->deletedCount--;
        
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

        if( DKRowIsActive( status ) )
        {
            hashTable->callbacks.rowDelete( row );
            hashTable->activeCount--;
            hashTable->deletedCount++;
        }
    }
}


///
//  DKGenericHashTableRemoveAll()
//
void DKGenericHashTableRemoveAll( DKGenericHashTable * hashTable )
{
    for( size_t i = 0; i < hashTable->rowCount; ++i )
    {
        void * row = hashTable->rows + (hashTable->rowSize * i);

        DKRowStatus status = hashTable->callbacks.rowStatus( row );

        if( DKRowIsActive( status ) )
            hashTable->callbacks.rowDelete( row );
        
        hashTable->callbacks.rowInit( row );
    }
    
    hashTable->activeCount = 0;
    hashTable->deletedCount = 0;
}


///
//  DKGenericHashTableForeachRow()
//
void DKGenericHashTableForeachRow( DKGenericHashTable * hashTable, DKGenericHashTableForeachRowCallback callback, void * context )
{
    for( size_t i = 0; i < hashTable->rowCount; ++i )
    {
        void * row = hashTable->rows + (hashTable->rowSize * i);

        DKRowStatus status = hashTable->callbacks.rowStatus( row );

        if( DKRowIsActive( status ) )
            callback( row, context );
    }
}








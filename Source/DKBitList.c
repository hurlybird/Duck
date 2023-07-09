/*****************************************************************************************

  DKBitlist.c

  Copyright (c) 2019 Derek W. Nylen

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

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKBitList.h"
#include "DKAllocation.h"


#define ROUND_POW2( n, p )  (((n) + ((p) - 1)) & ~((p) - 1))

#ifndef MIN
#define MIN( a, b )         ((a) < (b) ? (a) : (b))
#endif

#define BITS_PER_INT        (sizeof(uint32_t) * 8)
#define BIT_INDEX( n )      ((n) / BITS_PER_INT)
#define BIT_OFFSET( n )     ((n) % BITS_PER_INT)


struct DKBitList
{
    DKObject _obj;
    
    size_t numBits;
    size_t numInts;
    uint32_t bits[1]; // Variable size
};


static struct DKBitList DKPlaceholderBitList =
{
    DKInitStaticObjectHeader( NULL ),
};


static void *   DKBitListAllocPlaceholder( DKClassRef _class, size_t extraBytes );
static void     DKBitListDealloc( DKBitListRef _self );


///
//  DKBitListClass()
//
DKThreadSafeClassInit( DKBitListClass )
{
    // NOTE: The bits field of DKBitlist is dynamically sized, and not included in the
    // base instance structure size.
    DKClassRef cls = DKNewClass( DKSTR( "DKBitList" ), DKObjectClass(), sizeof(struct DKBitList), 0, NULL, NULL );
    
    // Allocation
    struct DKAllocationInterface * allocation = DKNewInterface( DKSelector(Allocation), sizeof(struct DKAllocationInterface) );
    allocation->alloc = (DKAllocMethod)DKBitListAllocPlaceholder;
    allocation->dealloc = (DKDeallocMethod)DKBitListDealloc;

    DKInstallClassInterface( cls, allocation );
    DKRelease( allocation );

    return cls;
}


///
//  DKBitListAllocPlaceholder()
//
static void * DKBitListAllocPlaceholder( DKClassRef _class, size_t extraBytes )
{
    if( _class == DKBitListClass_SharedObject )
    {
        DKPlaceholderBitList._obj.isa = DKBitListClass_SharedObject;
        return &DKPlaceholderBitList;
    }
    
    DKAssert( 0 );
    return NULL;
}


///
//  DKBitListDealloc()
//
static void DKBitListDealloc( DKBitListRef _self )
{
    if( _self == &DKPlaceholderBitList )
        return;
    
    DKDeallocObject( _self );
}


///
//  DKBitListInit()
//
DKObjectRef DKBitListInit( DKBitListRef _self, DKIndex length, bool initialValue )
{
    if( _self == &DKPlaceholderBitList  )
    {
        size_t bits, ints, extraBytes;

        if( length <= 0 )
        {
            bits = 0;
            ints = 1;
        }
        
        else
        {
            bits = length;
            ints = ROUND_POW2( bits, BITS_PER_INT );
        }

        extraBytes = (ints - 1) * sizeof(uint32_t); // The first int is included in DKBitList

        _self = DKAllocObject( DKBitListClass(), extraBytes );
        _self->numBits = bits;
        _self->numInts = ints;

        if( initialValue )
            DKBitListSetAllBits( _self );

        else
            DKBitListClearAllBits( _self );
    }
    
    else if( _self != NULL )
    {
        DKFatalError( "DKBitListInit: Trying to initialize a non-bitlist object." );
    }

    return _self;
}


///
//  DKBitListGetLength()
//
DKIndex DKBitListGetLength( DKBitListRef _self )
{
    return _self->numBits;
}


///
//  DKBitListSetAllBits()
//
void DKBitListSetAllBits( DKBitListRef _self )
{
    memset( _self->bits, -1, _self->numInts * sizeof(uint32_t) );
}


///
//  DKBitListClearAllBits()
//
void DKBitListClearAllBits( DKBitListRef _self )
{
    memset( _self->bits, 0, _self->numInts * sizeof(uint32_t) );
}


///
//  DKBitListFlipAllBits()
//
void DKBitListFlipAllBits( DKBitListRef _self )
{
    size_t i, n;
    
    n = _self->numInts;
    
    for( i = 0; i < n; i++ )
        _self->bits[i] = ~_self->bits[i];
}


///
//  DKBitListSetBit()
//
void DKBitListSetBit( DKBitListRef _self, DKIndex i )
{
    size_t index = BIT_INDEX( i );
    size_t offset = BIT_OFFSET( i );
    
    DKAssert( (size_t)i < _self->numBits );
    _self->bits[index] |= (1 << offset);
}


///
//  DKBitListClearBit()
//
void DKBitListClearBit( DKBitListRef _self, DKIndex i )
{
    size_t index = BIT_INDEX( i );
    size_t offset = BIT_OFFSET( i );
    
    DKAssert( (size_t)i < _self->numBits );
    _self->bits[index] &= ~(1 << offset);
}


///
//  DKBitListFlipBit()
//
void DKBitListFlipBit( DKBitListRef _self, DKIndex i )
{
    size_t index = BIT_INDEX( i );
    size_t offset = BIT_OFFSET( i );
    
    DKAssert( (size_t)i < _self->numBits );
    _self->bits[index] ^= (1 << offset);
}


///
//  DKBitListTestBit()
//
bool DKBitListTestBit( DKBitListRef _self, DKIndex i )
{
    size_t index = BIT_INDEX( i );
    size_t offset = BIT_OFFSET( i );
    
    DKAssert( (size_t)i < _self->numBits );
    uint32_t test = _self->bits[index] & (1 << offset);
    
    return test >> offset;
}


///
//  DKBitListTestAndSetBit()
//
bool DKBitListTestAndSetBit( DKBitListRef _self, DKIndex i )
{
    size_t index = BIT_INDEX( i );
    size_t offset = BIT_OFFSET( i );
    
    DKAssert( (size_t)i < _self->numBits );
    uint32_t test = _self->bits[index] & (1 << offset);
    _self->bits[index] |= (1 << offset);
    
    return test >> offset;
}


///
//  TestAndClearBit()
//
bool DKBitListTestAndClearBit( DKBitListRef _self, DKIndex i )
{
    size_t index = BIT_INDEX( i );
    size_t offset = BIT_OFFSET( i );
    
    DKAssert( (size_t)i < _self->numBits );
    uint32_t test = _self->bits[index] & (1 << offset);
    _self->bits[index] &= ~(1 << offset);
    
    return test >> offset;
}


///
//  TestAndFlipBit()
//
bool DKBitListTestAndFlipBit( DKBitListRef _self, DKIndex i )
{
    size_t index = BIT_INDEX( i );
    size_t offset = BIT_OFFSET( i );
    
    DKAssert( (size_t)i < _self->numBits );
    uint32_t test = _self->bits[index] & (1 << offset);
    _self->bits[index] ^= (1 << offset);
    
    return test >> offset;
}


///
//  DKBitListGetFirstSetBit()
//
DKIndex DKBitListGetFirstSetBit( DKBitListRef _self )
{
    size_t i, j, n;
    uint32_t bits;
    
    if( _self->numInts > 0 )
    {
        n = _self->numInts - 1;
        
        for( i = 0; i < n; i++ )
        {
            bits = _self->bits[i];
            
            if( bits != 0 )
            {
                for( j = 0; j < BITS_PER_INT; j++ )
                {
                    if( bits & (1 << j) )
                        return (i * BITS_PER_INT) + j;
                }
            }
        }

        // Unroll the last iteration to avoid a branch
        bits = _self->bits[i];
        
        if( bits != 0 )
        {
            n = _self->numBits & (BITS_PER_INT - 1);

            for( j = 0; j < n; j++ )
            {
                if( bits & (1 << j) )
                    return (i * BITS_PER_INT) + j;
            }
        }
    }
    
    return DKNotFound;
}


///
//  DKBitListGetFirstClearBit()
//
DKIndex DKBitListGetFirstClearBit( DKBitListRef _self )
{
    size_t i, j, n;
    uint32_t bits;

    if( _self->numInts > 0 )
    {
        n = _self->numInts - 1;
        
        for( i = 0; i < n; i++ )
        {
            bits = _self->bits[i];
            
            if( bits != (uint32_t)(-1) )
            {
                for( j = 0; j < BITS_PER_INT; j++ )
                {
                    if( !(bits & (1 << j)) )
                        return (i * BITS_PER_INT) + j;
                }
            }
        }

        // Unroll the last iteration to avoid a branch
        bits = _self->bits[i];
        
        if( bits != (uint32_t)(-1) )
        {
            n = _self->numBits & (BITS_PER_INT - 1);
            
            for( j = 0; j < n; j++ )
            {
                if( !(bits & (1 << j)) )
                    return (i * BITS_PER_INT) + j;
            }
        }
    }
    
    return DKNotFound;
}


///
//  DKBitListCountSetBits()
//
DKIndex DKBitListCountSetBits( DKBitListRef _self )
{
    size_t i, j, n, count;
    uint32_t bits;

    count = 0;
    
    if( _self->numInts > 0 )
    {
        n = _self->numInts - 1;
        
        for( i = 0; i < n; i++ )
        {
            bits = _self->bits[i];
            
            if( bits != 0 )
            {
                for( j = 0; j < BITS_PER_INT; j++ )
                    count += (bits >> j) & 1;
            }
        }
        
        // Unroll the last iteration to avoid a branch
        bits = _self->bits[i];
        
        if( bits != 0 )
        {
            n = _self->numBits & (BITS_PER_INT - 1);
            
            for( j = 0; j < n; j++ )
                count += (bits >> j) & 1;
        }
    }
    
    return count;
}


///
//  DKBitListCountClearBits()
//
DKIndex DKBitListCountClearBits( DKBitListRef _self )
{
    size_t i, j, n, count;
    uint32_t bits;

    count = 0;
    
    if( _self->numInts > 0 )
    {
        n = _self->numInts - 1;
        
        for( i = 0; i < n; i++ )
        {
            bits = _self->bits[i];
            
            if( bits != (uint32_t)(-1) )
            {
                for( j = 0; j < BITS_PER_INT; j++ )
                    count += ~(bits >> j) & 1;
            }
        }
        
        // Unroll the last iteration to avoid a branch
        bits = _self->bits[i];
        
        if( bits != (uint32_t)(-1) )
        {
            n = _self->numBits & (BITS_PER_INT - 1);
            
            for( j = 0; j < n; j++ )
                count += ~(bits >> j) & 1;
        }
    }
    
    return count;
}


///
//  DKBitListMergeBitsAND()
//
void DKBitListMergeBitsAND( DKBitListRef a, DKBitListRef b, DKBitListRef r )
{
    size_t i, n;
    
    n = MIN( a->numInts, b->numInts );
    n = MIN( n, b->numInts );
    
    for( i = 0; i < n; i++ )
        r->bits[i] = a->bits[i] & b->bits[i];
}


///
//  DKBitListMergeBitsNAND()
//
void DKBitListMergeBitsNAND( DKBitListRef a, DKBitListRef b, DKBitListRef r )
{
    size_t i, n;
    
    n = MIN( a->numInts, b->numInts );
    n = MIN( n, b->numInts );
    
    for( i = 0; i < n; i++ )
        r->bits[i] = ~(a->bits[i] & b->bits[i]);
}


///
//  DKBitListMergeBitsOR()
//
void DKBitListMergeBitsOR( DKBitListRef a, DKBitListRef b, DKBitListRef r )
{
    size_t i, n;
    
    n = MIN( a->numInts, b->numInts );
    n = MIN( n, b->numInts );
    
    for( i = 0; i < n; i++ )
        r->bits[i] = a->bits[i] | b->bits[i];
}


///
//  DKBitListMergeBitsNOR()
//
void DKBitListMergeBitsNOR( DKBitListRef a, DKBitListRef b, DKBitListRef r )
{
    size_t i, n;
    
    n = MIN( a->numInts, b->numInts );
    n = MIN( n, b->numInts );
    
    for( i = 0; i < n; i++ )
        r->bits[i] = ~(a->bits[i] | b->bits[i]);
}


///
//  DKBitListMergeBitsXOR()
//
void DKBitListMergeBitsXOR( DKBitListRef a, DKBitListRef b, DKBitListRef r )
{
    size_t i, n;
    
    n = MIN( a->numInts, b->numInts );
    n = MIN( n, b->numInts );
    
    for( i = 0; i < n; i++ )
        r->bits[i] = a->bits[i] ^ b->bits[i];
}


///
//  DKBitListMergeBitsNXOR()
//
void DKBitListMergeBitsNXOR( DKBitListRef a, DKBitListRef b, DKBitListRef r )
{
    size_t i, n;
    
    n = MIN( a->numInts, b->numInts );
    n = MIN( n, b->numInts );
    
    for( i = 0; i < n; i++ )
        r->bits[i] = ~(a->bits[i] ^ b->bits[i]);
}








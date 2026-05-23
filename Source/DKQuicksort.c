// =======================================================================================
//
// DKQuicksort.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKQuicksort.h"


// Generic Sorting =======================================================================
struct SortContext_t;

typedef int (*CompareFunction)( const void * a, const void * b, void * context );
typedef void (*SwapFunction)( void * a, void * b, struct SortContext_t * ctx );
typedef void (*InsertionSortFunction)( void * _ptr, struct SortContext_t * ctx, size_t count );

typedef struct SortContext_t
{
    size_t size;

    CompareFunction cmp;
    void * context;

    SwapFunction swap;
    void * tmp;

    InsertionSortFunction isort;
    
} SortContext;


///
//  ElementAtIndex()
//
static inline void * ElementAtIndex( void * ptr, size_t size, DKIndex index )
{
    return (uint8_t *)ptr + (index * size);
}


///
//  Swap*()
//
static void Swap8( void * _a, void * _b, struct SortContext_t * ctx )
{
    uint8_t * a = _a;
    uint8_t * b = _b;
    
    uint8_t swap = *a;
    *a = *b;
    *b = swap;
}

static void Swap16( void * _a, void * _b, struct SortContext_t * ctx )
{
    uint16_t * a = _a;
    uint16_t * b = _b;
    
    uint16_t swap = *a;
    *a = *b;
    *b = swap;
}

static void Swap32( void * _a, void * _b, struct SortContext_t * ctx )
{
    uint32_t * a = _a;
    uint32_t * b = _b;
    
    uint32_t swap = *a;
    *a = *b;
    *b = swap;
}

static void Swap64( void * _a, void * _b, struct SortContext_t * ctx )
{
    uint64_t * a = _a;
    uint64_t * b = _b;
    
    uint64_t swap = *a;
    *a = *b;
    *b = swap;
}

static void SwapN( void * a, void * b, struct SortContext_t * ctx )
{
    memcpy( ctx->tmp, a, ctx->size );
    memcpy( a, b, ctx->size );
    memcpy( b, ctx->tmp, ctx->size );
}


///
//  InsertionSort*()
//
static void InsertionSort8( void * _ptr, SortContext * ctx, size_t count )
{
    uint8_t * ptr = _ptr;

    for( size_t i = 1; i < count; ++i )
    {
        uint8_t x = ptr[i];
        DKIndex j = i - 1;
        
        while( (j >= 0) && (ctx->cmp( &ptr[j], &x, ctx->context ) > 0) )
        {
            ptr[j+1] = ptr[j];
            --j;
        }

        ptr[j+1] = x;
    }
}

static void InsertionSort16( void * _ptr, SortContext * ctx, size_t count )
{
    uint16_t * ptr = _ptr;

    for( size_t i = 1; i < count; ++i )
    {
        uint16_t x = ptr[i];
        DKIndex j = i - 1;
        
        while( (j >= 0) && (ctx->cmp( &ptr[j], &x, ctx->context ) > 0) )
        {
            ptr[j+1] = ptr[j];
            --j;
        }

        ptr[j+1] = x;
    }
}

static void InsertionSort32( void * _ptr, SortContext * ctx, size_t count )
{
    uint32_t * ptr = _ptr;

    for( size_t i = 1; i < count; ++i )
    {
        uint32_t x = ptr[i];
        DKIndex j = i - 1;
        
        while( (j >= 0) && (ctx->cmp( &ptr[j], &x, ctx->context ) > 0) )
        {
            ptr[j+1] = ptr[j];
            --j;
        }

        ptr[j+1] = x;
    }
}

static void InsertionSort64( void * _ptr, SortContext * ctx, size_t count )
{
    uint64_t * ptr = _ptr;

    for( size_t i = 1; i < count; ++i )
    {
        uint64_t x = ptr[i];
        DKIndex j = i - 1;
        
        while( (j >= 0) && (ctx->cmp( &ptr[j], &x, ctx->context ) > 0) )
        {
            ptr[j+1] = ptr[j];
            --j;
        }

        ptr[j+1] = x;
    }
}

static void InsertionSortN( void * ptr, SortContext * ctx, size_t count )
{
    #define ELEM( index )       ElementAtIndex( ptr, ctx->size, index )
    #define COPY( dst, src )    memcpy( dst, src, ctx->size )

    for( size_t i = 1; i < count; ++i )
    {
        COPY( ctx->tmp, ELEM( i ) );
        DKIndex j = i - 1;
        
        while( (j >= 0) && (ctx->cmp( ELEM( j ), ctx->tmp, ctx->context ) > 0) )
        {
            COPY( ELEM( j + 1 ), ELEM( j ) );
            --j;
        }

        COPY( ELEM( j + 1 ), ctx->tmp );
    }
    
    #undef ELEM
    #undef COPY
}


///
//  DKQuicksort()
//
#define ELEM( index )       ElementAtIndex( ptr, ctx->size, index )

static void * QuickSortPivot( void * ptr, SortContext * ctx, DKIndex lo, DKIndex hi )
{
    DKIndex mid = lo + (hi - lo) / 2;

    void * elem_lo = ELEM( lo );
    void * elem_mid = ELEM( mid );
    void * elem_hi = ELEM( hi );

    if( ctx->cmp( elem_mid, elem_lo, ctx->context ) > 0 )
        ctx->swap( elem_lo, elem_mid, ctx );
    
    if( ctx->cmp( elem_hi, elem_lo, ctx->context ) > 0 )
        ctx->swap( elem_lo, elem_hi, ctx );

    if( ctx->cmp( elem_mid, elem_hi, ctx->context ) > 0 )
        ctx->swap( elem_mid, elem_hi, ctx );

    return elem_hi;
}

static DKIndex QuickSortPartition( void * ptr, SortContext * ctx, DKIndex lo, DKIndex hi )
{
    void * pivot = QuickSortPivot( ptr, ctx, lo, hi );

    DKIndex i = lo - 1;
    DKIndex j = hi + 1;

    while( true )
    {
        while( ctx->cmp( ELEM( ++i ), pivot, ctx->context ) < 0 )
            ;
        
        while( ctx->cmp( ELEM( --j ), pivot, ctx->context ) > 0 )
            ;
        
        if( i >= j )
            return j;

        ctx->swap( ELEM( i ), ELEM( j ), ctx );
    }
}

static void QuickSort( void * ptr, SortContext * ctx, DKIndex lo, DKIndex hi )
{
    if( (hi - lo) < 10 )
    {
        ctx->isort( ELEM( lo ), ctx, hi - lo + 1 );
        return;
    }
 
    if( lo < hi )
    {
        DKIndex p = QuickSortPartition( ptr, ctx, lo, hi );
        
        QuickSort( ptr, ctx, lo, p );
        QuickSort( ptr, ctx, p + 1, hi );
    }
}

void DKQuicksort( void * ptr, size_t count, size_t size, int (*cmp)(const void *, const void *, void *), void * context )
{
    if( count > 1 )
    {
        SortContext ctx;
        ctx.size = size;
        ctx.cmp = cmp;
        ctx.context = context;

        uint8_t stack_tmp[DK_MAX_SORT_ELEM_SIZE];

        if( size <= DK_MAX_SORT_ELEM_SIZE )
            ctx.tmp = stack_tmp;
            
        else
            ctx.tmp = dk_malloc( size );

        switch( size )
        {
        case 1:
            ctx.swap = Swap8;
            ctx.isort = InsertionSort8;
            break;

        case 2:
            ctx.swap = Swap16;
            ctx.isort = InsertionSort16;
            break;

        case 4:
            ctx.swap = Swap32;
            ctx.isort = InsertionSort32;
            break;

        case 8:
            ctx.swap = Swap64;
            ctx.isort = InsertionSort64;
            break;

        default:
            ctx.swap = SwapN;
            ctx.isort = InsertionSortN;
            break;
        }

        QuickSort( ptr, &ctx, 0, count - 1 );
        
        if( ctx.tmp != stack_tmp )
            dk_free( ctx.tmp );
    }
}

#undef ELEM




// Object Sorting ========================================================================

///
//  InsertionSortObj()
//
static void InsertionSortObj( DKObjectRef objects[], size_t count, DKCompareFunction cmp )
{
    for( size_t i = 1; i < count; ++i )
    {
        DKObjectRef x = objects[i];
        DKIndex j = i - 1;
        
        while( (j >= 0) && (cmp( objects[j], x ) > 0) )
        {
            objects[j+1] = objects[j];
            --j;
        }
        
        objects[j+1] = x;
    }
}


///
//  DKQuicksortObjects()
//
static DKObjectRef QuickSortPivotObj( DKObjectRef objects[], DKIndex lo, DKIndex hi, DKCompareFunction cmp )
{
    DKIndex mid = lo + (hi - lo) / 2;
    DKObjectRef swap;

    if( cmp( objects[mid], objects[lo] ) > 0 )
    {
        swap = objects[lo];
        objects[lo] = objects[mid];
        objects[mid] = swap;
    }
    
    if( cmp( objects[hi], objects[lo] ) > 0 )
    {
        swap = objects[lo];
        objects[lo] = objects[hi];
        objects[hi] = swap;
    }

    if( cmp( objects[mid], objects[hi] ) > 0 )
    {
        swap = objects[mid];
        objects[mid] = objects[hi];
        objects[hi] = swap;
    }

    return objects[hi];
}

static size_t QuickSortPartitionObj( DKObjectRef objects[], DKIndex lo, DKIndex hi, DKCompareFunction cmp )
{
    DKObjectRef pivot = QuickSortPivotObj( objects, lo, hi, cmp );
    DKIndex i = lo - 1;
    DKIndex j = hi + 1;
    DKObjectRef swap;

    while( true )
    {
        while( cmp( objects[++i], pivot ) < 0 )
            ;
        
        while( cmp( objects[--j], pivot ) > 0 )
            ;
        
        if( i >= j )
            return j;
        
        swap = objects[i];
        objects[i] = objects[j];
        objects[j] = swap;
    }
}

static void QuickSortObj( DKObjectRef objects[], DKIndex lo, DKIndex hi, DKCompareFunction cmp )
{
    if( (hi - lo) < 10 )
    {
        InsertionSortObj( &objects[lo], hi - lo + 1, cmp );
        return;
    }
 
    if( lo < hi )
    {
        DKIndex p = QuickSortPartitionObj( objects, lo, hi, cmp );
        
        QuickSortObj( objects, lo, p, cmp );
        QuickSortObj( objects, p + 1, hi, cmp );
    }
}

void DKQuicksortObjects( DKObjectRef objects[], size_t count, DKCompareFunction cmp )
{
    if( count > 1 )
    {
        QuickSortObj( objects, 0, count - 1, cmp );
    }
}

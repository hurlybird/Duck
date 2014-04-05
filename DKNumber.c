//
//  DKNumber.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-31.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKNumber.h"
#include "DKRuntime.h"
#include "DKCopying.h"
#include "DKString.h"
#include "DKStream.h"


#define MAX_COUNT   65535


typedef union
{
    int32_t     _int32[1];
    int64_t     _int64[1];
    uint32_t    _uint32[1];
    uint64_t    _uint64[1];
    float       _float[1];
    double      _double[1];

} DKNumberValue;

struct DKNumber
{
    DKObjectHeader _obj;
    
    uint16_t type;
    uint16_t count;
    DKNumberValue value;
};


// Number Size and Type Casting ==========================================================
static const size_t FieldSize[DKNumberMaxTypes] =
{
    0,
    sizeof(int32_t),
    sizeof(int64_t),
    sizeof(uint32_t),
    sizeof(uint64_t),
    sizeof(float),
    sizeof(double)
};

typedef void (*CastFunction)( const DKNumberValue * val, DKNumberValue * out );

#define DefineCastFunction( func, src, dst, dst_type )                                  \
    static void func( const DKNumberValue * v, DKNumberValue * r )                      \
    {                                                                                   \
        r->dst[0] = (dst_type)v->src[0];                                                \
    }

DefineCastFunction( CastInt32ToInt32,   _int32,  _int32,  int32_t );
DefineCastFunction( CastInt32ToInt64,   _int32,  _int64,  int64_t );
DefineCastFunction( CastInt32ToUInt32,  _int32,  _uint32, uint32_t );
DefineCastFunction( CastInt32ToUInt64,  _int32,  _uint64, uint64_t );
DefineCastFunction( CastInt32ToFloat,   _int32,  _float,  float );
DefineCastFunction( CastInt32ToDouble,  _int32,  _double, double );

DefineCastFunction( CastInt64ToInt32,   _int64,  _int32,  int32_t );
DefineCastFunction( CastInt64ToInt64,   _int64,  _int64,  int64_t );
DefineCastFunction( CastInt64ToUInt32,  _int64,  _uint32, uint32_t );
DefineCastFunction( CastInt64ToUInt64,  _int64,  _uint64, uint64_t );
DefineCastFunction( CastInt64ToFloat,   _int64,  _float,  float );
DefineCastFunction( CastInt64ToDouble,  _int64,  _double, double );

DefineCastFunction( CastUInt32ToInt32,  _uint32, _int32,  int32_t );
DefineCastFunction( CastUInt32ToInt64,  _uint32, _int64,  int64_t );
DefineCastFunction( CastUInt32ToUInt32, _uint32, _uint32, uint32_t );
DefineCastFunction( CastUInt32ToUInt64, _uint32, _uint64, uint64_t );
DefineCastFunction( CastUInt32ToFloat,  _uint32, _float,  float );
DefineCastFunction( CastUInt32ToDouble, _uint32, _double, double );

DefineCastFunction( CastUInt64ToInt32,  _uint64, _int32,  int32_t );
DefineCastFunction( CastUInt64ToInt64,  _uint64, _int64,  int64_t );
DefineCastFunction( CastUInt64ToUInt32, _uint64, _uint32, uint32_t );
DefineCastFunction( CastUInt64ToUInt64, _uint64, _uint64, uint64_t );
DefineCastFunction( CastUInt64ToFloat,  _uint64, _float,  float );
DefineCastFunction( CastUInt64ToDouble, _uint64, _double, double );

DefineCastFunction( CastFloatToInt32,   _float,  _int32,  int32_t );
DefineCastFunction( CastFloatToInt64,   _float,  _int64,  int64_t );
DefineCastFunction( CastFloatToUInt32,  _float,  _uint32, uint32_t );
DefineCastFunction( CastFloatToUInt64,  _float,  _uint64, uint64_t );
DefineCastFunction( CastFloatToFloat,   _float,  _float,  float );
DefineCastFunction( CastFloatToDouble,  _float,  _double, double );

DefineCastFunction( CastDoubleToInt32,  _double, _int32,  int32_t );
DefineCastFunction( CastDoubleToInt64,  _double, _int64,  int64_t );
DefineCastFunction( CastDoubleToUInt32, _double, _uint32, uint32_t );
DefineCastFunction( CastDoubleToUInt64, _double, _uint64, uint64_t );
DefineCastFunction( CastDoubleToFloat,  _double, _float,  float );
DefineCastFunction( CastDoubleToDouble, _double, _double, double );


static CastFunction CastFunctions[DKNumberMaxTypes][DKNumberMaxTypes] =
{
    /*                      Int32               Int64               UInt32              Uint64              Float               Double  */
                 {  NULL,               NULL,               NULL,               NULL,               NULL,               NULL                },
    /* Int32 */  {  NULL,   CastInt32ToInt32,   CastInt32ToInt64,   CastInt32ToUInt32,  CastInt32ToUInt64,  CastInt32ToFloat,   CastInt32ToDouble   },
    /* Int64 */  {  NULL,   CastInt64ToInt32,   CastInt64ToInt64,   CastInt64ToUInt32,  CastInt64ToUInt64,  CastInt64ToFloat,   CastInt64ToDouble   },
    /* UInt32 */ {  NULL,   CastUInt32ToInt32,  CastUInt32ToInt64,  CastUInt32ToUInt32, CastUInt32ToUInt64, CastUInt32ToFloat,  CastUInt32ToDouble  },
    /* UInt64 */ {  NULL,   CastUInt64ToInt32,  CastUInt64ToInt64,  CastUInt64ToUInt32, CastUInt64ToUInt64, CastUInt64ToFloat,  CastUInt64ToDouble  },
    /* Float */  {  NULL,   CastFloatToInt32,   CastFloatToInt64,   CastFloatToUInt32,  CastFloatToUInt64,  CastFloatToFloat,   CastFloatToDouble   },
    /* Double */ {  NULL,   CastDoubleToInt32,  CastDoubleToInt64,  CastDoubleToUInt32, CastDoubleToUInt64, CastDoubleToFloat,  CastDoubleToDouble  }
};



///
//  DKNumberClass()
//
DKThreadSafeClassInit( DKNumberClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKNumber" ), DKObjectClass(), sizeof(struct DKNumber), 0 );
    
    // Copying
    struct DKCopying * copying = DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
    copying->copy = DKRetain;
    copying->mutableCopy = (void *)DKRetain;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );
    
    // Comparison
    struct DKComparison * comparison = DKAllocInterface( DKSelector(Comparison), sizeof(DKComparison) );
    comparison->equal = (void *)DKNumberEqual;
    comparison->compare = (void *)DKNumberCompare;
    comparison->hash = (void *)DKNumberHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );
    
    // Description
    struct DKDescription * description = DKAllocInterface( DKSelector(Description), sizeof(DKDescription) );
    description->copyDescription = (void *)DKNumberCopyDescription;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    return cls;
}


///
//  DKNumberCreate()
//
DKNumberRef DKNumberCreate( const void * value, DKNumberType type, size_t count )
{
    DKAssert( (type > 0) && (type < DKNumberMaxTypes) );
    DKAssert( (count > 0) && (count < MAX_COUNT) );
    DKAssert( value != NULL );

    size_t size = FieldSize[type];
    size_t bytes = size * count;
    
    struct DKNumber * number = (struct DKNumber *)DKAllocObject( DKNumberClass(), bytes );
    
    if( number )
    {
        number->type = type;
        number->count = count;
        memcpy( &number->value, value, bytes );
    }
    
    return number;
}


///
//  DKNumberCreate*()
//
DKNumberRef DKNumberCreateInt32( int32_t x )
{
    return DKNumberCreate( &x, DKNumberInt32, 1 );
}

DKNumberRef DKNumberCreateInt64( int64_t x )
{
    return DKNumberCreate( &x, DKNumberInt64, 1 );
}

DKNumberRef DKNumberCreateUInt32( uint32_t x )
{
    return DKNumberCreate( &x, DKNumberUInt32, 1 );
}

DKNumberRef DKNumberCreateUInt64( uint64_t x )
{
    return DKNumberCreate( &x, DKNumberUInt64, 1 );
}

DKNumberRef DKNumberCreateFloat( float x )
{
    return DKNumberCreate( &x, DKNumberFloat, 1 );
}

DKNumberRef DKNumberCreateDouble( double x )
{
    return DKNumberCreate( &x, DKNumberDouble, 1 );
}


///
//  DKNumberGetType()
//
DKNumberType DKNumberGetType( DKNumberRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKNumberClass() );
        
        const struct DKNumber * number = _self;
        return number->type;
    }
    
    return DKNumberVoid;
}


///
//  DKNumberGetCount()
//
size_t DKNumberGetCount( DKNumberRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKNumberClass() );
        
        const struct DKNumber * number = _self;
        return number->count;
    }
    
    return 0;
}


///
//  DKNumberGetValue()
//
size_t DKNumberGetValue( DKNumberRef _self, void * value )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKNumberClass() );
        
        const struct DKNumber * number = _self;

        size_t size = FieldSize[number->type];
        memcpy( value, &number->value, size * number->count );
        
        return number->count;
    }

    return 0;
}


///
//  DKNumberCastValue()
//
size_t DKNumberCastValue( DKNumberRef _self, void * value, DKNumberType type )
{
    DKAssert( (type > 0) && (type < DKNumberMaxTypes) );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKNumberClass() );
        
        const struct DKNumber * number = _self;

        const uint8_t * src = (const uint8_t *)&number->value;
        uint8_t * dst = value;
        
        CastFunction func = CastFunctions[number->type][type];
        size_t size = FieldSize[number->type];
        
        for( unsigned int i = 0; i < number->count; ++i )
            func( (const DKNumberValue *)&src[i * size], (DKNumberValue *)&dst[i * size] );
        
        return number->count;
    }

    return 0;
}


///
//  DKNumberGetValuePtr()
//
const void * DKNumberGetValuePtr( DKNumberRef _self )
{
    static int64_t zero = 0;

    if( _self )
    {
        DKAssertKindOfClass( _self, DKNumberClass() );
        
        const struct DKNumber * number = _self;
        return &number->value;
    }
    
    return &zero;
}


///
//  DKNumberEqual()
//
int DKNumberEqual( DKNumberRef a, DKNumberRef b )
{
    if( DKIsKindOfClass( b, DKNumberClass() ) )
        return DKNumberCompare( a, b ) == 0;
    
    return 0;
}


///
//  DKNumberCompare()
//
int DKNumberCompare( DKNumberRef a, DKNumberRef b )
{
    if( a )
    {
        DKAssertKindOfClass( a, DKNumberClass() );
        DKAssertKindOfClass( b, DKNumberClass() );

        if( (a->count == 1) && (b->count == 1) )
        {
            if( a->type == b->type )
            {
                #define CMP( x, y )     (((x) < (y)) ? 1 : (((x) > (y)) ? -1 : 0))
            
                switch( a->type )
                {
                case DKNumberInt32:  return CMP( a->value._int32, b->value._int32 );
                case DKNumberInt64:  return CMP( a->value._int64, b->value._int64 );
                case DKNumberUInt32: return CMP( a->value._uint32, b->value._uint32 );
                case DKNumberUInt64: return CMP( a->value._uint64, b->value._uint64 );
                case DKNumberFloat:  return CMP( a->value._float, b->value._float );
                case DKNumberDouble: return CMP( a->value._double, b->value._double );
                }
                
                #undef CMP
            }
            
            else
            {
                DKNumberValue da, db;
                CastFunctions[a->type][DKNumberDouble]( &a->value, &da );
                CastFunctions[b->type][DKNumberDouble]( &b->value, &db );
                
                if( da._double[0] < db._double[0] )
                    return 1;

                if( da._double[0] > db._double[0] )
                    return -1;
                
                return 0;
            }
        }
        
        else
        {
            size_t sa = FieldSize[a->type] * a->count;
            size_t sb = FieldSize[b->type] * b->count;
        
            if( sa < sb )
                return 1;
            
            if( sa > sb )
                return -1;
            
            return memcmp( &a->value, &b->value, sa );
        }
    }
    
    return DKPointerCompare( a, b );
}


///
//  DKNumberHash()
//
DKHashCode DKNumberHash( DKNumberRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKNumberClass() );
    
        const struct DKNumber * number = _self;
        size_t size = FieldSize[number->type] * number->count;

        if( (number->count == 1) && (size < sizeof(DKHashCode)) )
        {
            switch( number->type )
            {
            case DKNumberInt32:  return number->value._int32;
            case DKNumberInt64:  return number->value._int64;
            case DKNumberUInt32: return number->value._uint32;
            case DKNumberUInt64: return number->value._uint64;
            case DKNumberFloat:  return *((uint32_t *)&number->value._float);
            
            #if __LP64__
            case DKNumberDouble: return *((uint64_t *)&number->value._double);
            #endif
            }
        }
        
        else
        {
            return dk_memhash( &number->value, size );
        }
    }
    
    return 0;
}


///
//  DKNumberCopyDescription()
//
DKStringRef DKNumberCopyDescription( DKNumberRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKNumberClass() );
        const struct DKNumber * number = _self;
        
        DKMutableStringRef desc = DKStringCreateMutable();
        
        #define PRINT( fmt, field )                                 \
            DKSPrintf( desc, fmt, number->value.field[0] );         \
            for( unsigned int i = 1; i < number->count; ++i )       \
                DKSPrintf( desc, " " fmt, number->value.field[i] )
        
        switch( number->type )
        {
        case DKNumberInt32:  PRINT( "%d", _int32 ); break;
        case DKNumberInt64:  PRINT( "%lld", _int64 ); break;
        case DKNumberUInt32: PRINT( "%u", _uint32 ); break;
        case DKNumberUInt64: PRINT( "%llu", _uint64 ); break;
        case DKNumberFloat:  PRINT( "%f", _float ); break;
        case DKNumberDouble: PRINT( "%lf", _double ); break;
        }
        
        #undef PRINT
        
        return desc;
    }
    
    return NULL;
}







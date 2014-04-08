/*****************************************************************************************

  DKNumber.c

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

#include "DKNumber.h"
#include "DKRuntime.h"
#include "DKCopying.h"
#include "DKString.h"
#include "DKStream.h"


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
    DKObject _obj;
    DKNumberValue value;
};

struct DKNumberComponentInfo
{
    const char * name;
    size_t size;
};

static const struct DKNumberComponentInfo ComponentInfo[DKNumberMaxComponentTypes] =
{
    { "void",       0,                  },
    { "int32_t",    sizeof(int32_t)     },
    { "int64_t",    sizeof(int64_t)     },
    { "uint32_t",   sizeof(uint32_t)    },
    { "uint64_t",   sizeof(uint64_t)    },
    { "float",      sizeof(float)       },
    { "double",     sizeof(double)      }
};

size_t DKNumberGetComponentSize( int32_t type )
{
    DKNumberComponentType baseType = DKNumberGetComponentType( type );

    if( (baseType > 0) && (baseType < DKNumberMaxComponentTypes) )
        return ComponentInfo[baseType].size;
    
    return ComponentInfo[0].size;
}

const char * DKNumberGetComponentName( DKNumberType type )
{
    DKNumberComponentType baseType = DKNumberGetComponentType( type );

    if( (baseType > 0) && (baseType < DKNumberMaxComponentTypes) )
        return ComponentInfo[baseType].name;
    
    return ComponentInfo[0].name;
}

int DKNumberTypeIsValid( int32_t type )
{
    DKNumberComponentType baseType = DKNumberGetComponentType( type );
    size_t count = DKNumberGetComponentCount( type );
    
    return (baseType > 0) && (baseType < DKNumberMaxComponentTypes) && (count > 0) && (count <= DKNumberMaxComponentCount);
}




// Type Casting ==========================================================================
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


static CastFunction _CastFunctions[DKNumberMaxComponentTypes][DKNumberMaxComponentTypes] =
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

static CastFunction GetCastFunction( DKNumberType fromType, DKNumberType toType )
{
    DKAssert( DKNumberTypeIsValid( fromType ) );
    DKAssert( DKNumberTypeIsValid( toType ) );

    DKNumberComponentType fromBaseType = DKNumberGetComponentType( fromType );
    DKNumberComponentType toBaseType = DKNumberGetComponentType( toType );
    
    return _CastFunctions[fromBaseType][toBaseType];
}


///
//  DKNumberClass()
//
DKThreadSafeClassInit( DKNumberClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKNumber" ), DKObjectClass(), sizeof(struct DKNumber), 0 );
    
    // Copying
    struct DKCopyingInterface * copying = DKAllocInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = DKRetain;
    copying->mutableCopy = (void *)DKRetain;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );
    
    // Comparison
    struct DKComparisonInterface * comparison = DKAllocInterface( DKSelector(Comparison), sizeof(struct DKComparisonInterface) );
    comparison->equal = (void *)DKNumberEqual;
    comparison->compare = (void *)DKNumberCompare;
    comparison->hash = (void *)DKNumberHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );
    
    // Description
    struct DKDescriptionInterface * description = DKAllocInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->copyDescription = (void *)DKNumberCopyDescription;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    return cls;
}


///
//  DKNumberCreate()
//
DKNumberRef DKNumberCreate( const void * value, DKNumberType type )
{
    DKAssert( DKNumberTypeIsValid( type ) );
    DKAssert( value != NULL );

    size_t size = DKNumberGetComponentSize( type );
    size_t count = DKNumberGetComponentCount( type );
    size_t bytes = size * count;
    
    struct DKNumber * number = (struct DKNumber *)DKAllocObject( DKNumberClass(), bytes );
    number = DKInitializeObject( number );
    
    if( number )
    {
        DKSetObjectTag( number, type );
        memcpy( &number->value, value, bytes );
    }
    
    return number;
}


///
//  DKNumberCreate*()
//
DKNumberRef DKNumberCreateInt32( int32_t x )
{
    return DKNumberCreate( &x, DKNumberInt32 );
}

DKNumberRef DKNumberCreateInt64( int64_t x )
{
    return DKNumberCreate( &x, DKNumberInt64 );
}

DKNumberRef DKNumberCreateUInt32( uint32_t x )
{
    return DKNumberCreate( &x, DKNumberUInt32 );
}

DKNumberRef DKNumberCreateUInt64( uint64_t x )
{
    return DKNumberCreate( &x, DKNumberUInt64 );
}

DKNumberRef DKNumberCreateFloat( float x )
{
    return DKNumberCreate( &x, DKNumberFloat );
}

DKNumberRef DKNumberCreateDouble( double x )
{
    return DKNumberCreate( &x, DKNumberDouble );
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
        return DKGetObjectTag( number );
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

        DKNumberType type = DKGetObjectTag( number );
        size_t size = DKNumberGetComponentSize( type );
        size_t count = DKNumberGetComponentCount( type );
        
        memcpy( value, &number->value, size * count );
        
        return count;
    }

    return 0;
}


///
//  DKNumberCastValue()
//
size_t DKNumberCastValue( DKNumberRef _self, void * value, DKNumberType type )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKNumberClass() );
        
        return DKNumberConvert( &_self->value, DKGetObjectTag( _self ), value, type );
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

        DKNumberType a_type = DKGetObjectTag( a );
        DKNumberType b_type = DKGetObjectTag( b );

        size_t a_count = DKNumberGetComponentType( a_type );
        size_t b_count = DKNumberGetComponentType( b_type );

        if( (a_count == 1) && (b_count == 1) )
        {
            if( a_type == b_type )
            {
                DKNumberComponentType baseType = DKNumberGetComponentType( a_type );
                
                #define CMP( x, y )     (((x) < (y)) ? 1 : (((x) > (y)) ? -1 : 0))
            
                switch( baseType )
                {
                case DKNumberComponentInt32:  return CMP( a->value._int32[0],  b->value._int32[0] );
                case DKNumberComponentInt64:  return CMP( a->value._int64[0],  b->value._int64[0] );
                case DKNumberComponentUInt32: return CMP( a->value._uint32[0], b->value._uint32[0] );
                case DKNumberComponentUInt64: return CMP( a->value._uint64[0], b->value._uint64[0] );
                case DKNumberComponentFloat:  return CMP( a->value._float[0],  b->value._float[0] );
                case DKNumberComponentDouble: return CMP( a->value._double[0], b->value._double[0] );
                
                default:
                    DKAssert( 0 );
                    break;
                }
                
                #undef CMP
            }
            
            else
            {
                DKNumberValue da, db;
                GetCastFunction( a_type, DKNumberDouble )( &a->value, &da );
                GetCastFunction( b_type, DKNumberDouble )( &b->value, &db );
                
                if( da._double[0] < db._double[0] )
                    return 1;

                if( da._double[0] > db._double[0] )
                    return -1;
                
                return 0;
            }
        }
        
        else
        {
            size_t sa = DKNumberGetComponentSize( a_type ) * a_count;
            size_t sb = DKNumberGetComponentSize( b_type ) * b_count;
        
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
    
        DKNumberType type = DKGetObjectTag( _self );
        DKNumberComponentType baseType = DKNumberGetComponentType( type );
        size_t size = DKNumberGetComponentSize( type );
        size_t count = DKNumberGetComponentCount( type );

        if( (count == 1) && (size <= sizeof(DKHashCode)) )
        {
            switch( baseType )
            {
            case DKNumberComponentInt32:  return _self->value._int32[0];
            case DKNumberComponentUInt32: return _self->value._uint32[0];
            case DKNumberComponentFloat:  return *((uint32_t *)&_self->value._float[0]);
            
            #if __LP64__
            case DKNumberComponentInt64:  return _self->value._int64[0];
            case DKNumberComponentUInt64: return _self->value._uint64[0];
            case DKNumberComponentDouble: return *((uint64_t *)&_self->value._double[0]);
            #endif
            
            default:
                DKAssert( 0 );
                break;
            }
        }
        
        else
        {
            return dk_memhash( &_self->value, size * count );
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

        DKNumberType type = DKGetObjectTag( _self );
        DKNumberComponentType baseType = DKNumberGetComponentType( type );
        size_t count = DKNumberGetComponentCount( type );
        
        #define PRINT( fmt, field )                                 \
            DKSPrintf( desc, fmt, number->value.field[0] );         \
            for( unsigned int i = 1; i < count; ++i )               \
                DKSPrintf( desc, " " fmt, number->value.field[i] )
        
        switch( baseType )
        {
        case DKNumberComponentInt32:  PRINT( "%d", _int32 ); break;
        case DKNumberComponentInt64:  PRINT( "%lld", _int64 ); break;
        case DKNumberComponentUInt32: PRINT( "%u", _uint32 ); break;
        case DKNumberComponentUInt64: PRINT( "%llu", _uint64 ); break;
        case DKNumberComponentFloat:  PRINT( "%f", _float ); break;
        case DKNumberComponentDouble: PRINT( "%lf", _double ); break;
        
        default:
            DKAssert( 0 );
            break;
        }
        
        #undef PRINT
        
        return desc;
    }
    
    return NULL;
}


///
//  DKNumberConvert()
//
size_t DKNumberConvert( const void * src, DKNumberType srcType, void * dst, DKNumberType dstType )
{
    if( !DKNumberTypeIsValid( dstType ) )
        return 0;
    
    if( !DKNumberTypeIsValid( srcType ) )
        return 0;
    
    size_t dstCount = DKNumberGetComponentCount( dstType );
    size_t srcCount = DKNumberGetComponentCount( srcType );

    if( dstCount != srcCount )
        return 0;

    size_t dstSize = DKNumberGetComponentSize( dstType );
    size_t srcSize = DKNumberGetComponentSize( srcType );
    
    CastFunction func = GetCastFunction( srcType, dstType );
    
    for( size_t i = 0; i < dstCount; ++i )
    {
        DKNumberValue * srcValue = (DKNumberValue *)((const uint8_t *)src + (i * srcSize));
        DKNumberValue * dstValue = (DKNumberValue *)((uint8_t *)dst + (i * dstSize));
        
        func( srcValue, dstValue );
    }

    return dstCount;
}







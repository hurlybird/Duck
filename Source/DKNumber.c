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
#include "DKString.h"
#include "DKStream.h"


typedef union
{
    int8_t      _int8_t[1];
    int16_t     _int16_t[1];
    int32_t     _int32_t[1];
    int64_t     _int64_t[1];

    uint8_t     _uint8_t[1];
    uint16_t    _uint16_t[1];
    uint32_t    _uint32_t[1];
    uint64_t    _uint64_t[1];

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

    { "int8_t",     sizeof(int8_t)      },
    { "int16_t",    sizeof(int16_t)     },
    { "int32_t",    sizeof(int32_t)     },
    { "int64_t",    sizeof(int64_t)     },

    { "uint8_t",    sizeof(uint8_t)     },
    { "uint16_t",   sizeof(uint16_t)    },
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




// Type Casting/Comparing ================================================================

// Define a cast function ----------------------------------------------------------------
typedef void (*CastFunction)( const DKNumberValue * restrict x, DKNumberValue * restrict y, size_t count );

#define CastFuncName( xtype, ytype )    Cast_ ## xtype ## _to_ ## ytype

#define DefineCastFunction( xtype, ytype )                                              \
    static void CastFuncName( xtype, ytype )( const DKNumberValue * restrict x, DKNumberValue * restrict y, size_t count ) \
    {                                                                                   \
        for( size_t i = 0; i < count; ++i )                                             \
            y->_ ## ytype[i] = (ytype)x->_ ## xtype[i];                                 \
    }




// Define a cmp function -----------------------------------------------------------------
typedef int (*CmpFunction)( const DKNumberValue * x, const DKNumberValue * y, size_t count );

#define CmpFuncName( xtype, ytype )     Cmp_ ## xtype ## _to_ ## ytype

#define DefineCmpFunction( xtype, ytype )                                               \
    static int CmpFuncName( xtype, ytype )( const DKNumberValue * x, const DKNumberValue * y, size_t count ) \
    {                                                                                   \
        for( size_t i = 0; i < count; ++i )                                             \
        {                                                                               \
            if( x->_ ## xtype[i] < y->_ ## ytype[i] )                                   \
                return 1;                                                               \
                                                                                        \
            if( x->_ ## xtype[i] > y->_ ## ytype[i] )                                   \
                return -1;                                                              \
        }                                                                               \
                                                                                        \
        return 0;                                                                       \
    }




// Define functions for every type combination -------------------------------------------
#define DefineFunctionSet( xtype, ytype )                                               \
    DefineCastFunction( xtype, ytype )                                                  \
    DefineCmpFunction( xtype, ytype )

#define DefineFunctionSetForType( xtype )                                               \
    DefineFunctionSet( xtype, int8_t )                                                  \
    DefineFunctionSet( xtype, int16_t )                                                 \
    DefineFunctionSet( xtype, int32_t )                                                 \
    DefineFunctionSet( xtype, int64_t )                                                 \
    DefineFunctionSet( xtype, uint8_t )                                                 \
    DefineFunctionSet( xtype, uint16_t )                                                \
    DefineFunctionSet( xtype, uint32_t )                                                \
    DefineFunctionSet( xtype, uint64_t )                                                \
    DefineFunctionSet( xtype, float )                                                   \
    DefineFunctionSet( xtype, double )                                                  \


DefineFunctionSetForType( int8_t );
DefineFunctionSetForType( int16_t );
DefineFunctionSetForType( int32_t );
DefineFunctionSetForType( int64_t );

DefineFunctionSetForType( uint8_t );
DefineFunctionSetForType( uint16_t );
DefineFunctionSetForType( uint32_t );
DefineFunctionSetForType( uint64_t );

DefineFunctionSetForType( float );
DefineFunctionSetForType( double );




// Cast Function Table -------------------------------------------------------------------
#define CastFunctionTableRow( xtype )                                                   \
    {                                                                                   \
        NULL,                                                                           \
        CastFuncName( xtype, int8_t ),                                                  \
        CastFuncName( xtype, int16_t ),                                                 \
        CastFuncName( xtype, int32_t ),                                                 \
        CastFuncName( xtype, int64_t ),                                                 \
        CastFuncName( xtype, uint8_t ),                                                 \
        CastFuncName( xtype, uint16_t ),                                                \
        CastFuncName( xtype, uint32_t ),                                                \
        CastFuncName( xtype, uint64_t ),                                                \
        CastFuncName( xtype, float ),                                                   \
        CastFuncName( xtype, double ),                                                  \
    }

static CastFunction CastFunctionTable[DKNumberMaxComponentTypes][DKNumberMaxComponentTypes] =
{
    { NULL },

    CastFunctionTableRow( int8_t ),
    CastFunctionTableRow( int16_t ),
    CastFunctionTableRow( int32_t ),
    CastFunctionTableRow( int64_t ),

    CastFunctionTableRow( uint8_t ),
    CastFunctionTableRow( uint16_t ),
    CastFunctionTableRow( uint32_t ),
    CastFunctionTableRow( uint64_t ),

    CastFunctionTableRow( float ),
    CastFunctionTableRow( double )
};

static CastFunction GetCastFunction( DKNumberType xtype, DKNumberType ytype )
{
    DKAssert( DKNumberTypeIsValid( xtype ) );
    DKAssert( DKNumberTypeIsValid( ytype ) );

    DKNumberComponentType xbase = DKNumberGetComponentType( xtype );
    DKNumberComponentType ybase = DKNumberGetComponentType( ytype );
    
    return CastFunctionTable[xbase][ybase];
}




// Cmp Function Table --------------------------------------------------------------------
#define CmpFunctionTableRow( xtype )                                                    \
    {                                                                                   \
        NULL,                                                                           \
        CmpFuncName( xtype, int8_t ),                                                   \
        CmpFuncName( xtype, int16_t ),                                                  \
        CmpFuncName( xtype, int32_t ),                                                  \
        CmpFuncName( xtype, int64_t ),                                                  \
        CmpFuncName( xtype, uint8_t ),                                                  \
        CmpFuncName( xtype, uint16_t ),                                                 \
        CmpFuncName( xtype, uint32_t ),                                                 \
        CmpFuncName( xtype, uint64_t ),                                                 \
        CmpFuncName( xtype, float ),                                                    \
        CmpFuncName( xtype, double ),                                                   \
    }

static CmpFunction CmpFunctionTable[DKNumberMaxComponentTypes][DKNumberMaxComponentTypes] =
{
    { NULL },

    CmpFunctionTableRow( int8_t ),
    CmpFunctionTableRow( int16_t ),
    CmpFunctionTableRow( int32_t ),
    CmpFunctionTableRow( int64_t ),

    CmpFunctionTableRow( uint8_t ),
    CmpFunctionTableRow( uint16_t ),
    CmpFunctionTableRow( uint32_t ),
    CmpFunctionTableRow( uint64_t ),

    CmpFunctionTableRow( float ),
    CmpFunctionTableRow( double )
};

static CmpFunction GetCmpFunction( DKNumberType xtype, DKNumberType ytype )
{
    DKAssert( DKNumberTypeIsValid( xtype ) );
    DKAssert( DKNumberTypeIsValid( ytype ) );

    DKNumberComponentType xbase = DKNumberGetComponentType( xtype );
    DKNumberComponentType ybase = DKNumberGetComponentType( ytype );
    
    return CmpFunctionTable[xbase][ybase];
}




// DKNumber ==============================================================================

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
//  DKNumberCreateUUID()
//
DKNumberRef DKNumberCreateUUID( const DKUUID * uuid )
{
    DKUUID _uuid;
    
    if( uuid == NULL )
    {
        _uuid = dk_uuid_generate();
        uuid = &_uuid;
    }
    
    return DKNumberCreate( uuid, DKNumberUUID );
}


///
//  DKNumberCreateDate()
//
DKNumberRef DKNumberCreateDate( const DKDateTime * date )
{
    DKDateTime _date;
    
    if( date == NULL )
    {
        _date = dk_datetime();
        date = &_date;
    }

    return DKNumberCreate( &date, DKNumberDate );
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
bool DKNumberEqual( DKNumberRef a, DKNumberRef b )
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

        if( a_count == b_count )
        {
            CmpFunction cmpFunction = GetCmpFunction( a_type, b_type );
            return cmpFunction( &a->value, &b->value, b_count );
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
            case DKNumberComponentInt8:   return _self->value._int8_t[0];
            case DKNumberComponentInt16:  return _self->value._int16_t[0];
            case DKNumberComponentInt32:  return _self->value._int32_t[0];

            case DKNumberComponentUInt8:  return _self->value._uint8_t[0];
            case DKNumberComponentUInt16: return _self->value._uint16_t[0];
            case DKNumberComponentUInt32: return _self->value._uint32_t[0];

            case DKNumberComponentFloat:  return *((uint32_t *)&_self->value._float[0]);
            
            #if __LP64__
            case DKNumberComponentInt64:  return _self->value._int64_t[0];
            case DKNumberComponentUInt64: return _self->value._uint64_t[0];
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
        
        #define PRINT( fmt, field )                                         \
            DKSPrintf( desc, fmt, number->value._ ## field[0] );            \
            for( unsigned int i = 1; i < count; ++i )                       \
                DKSPrintf( desc, " " fmt, number->value._ ## field[i] )
        
        switch( baseType )
        {
        case DKNumberComponentInt8:   PRINT( "%d", int8_t ); break;
        case DKNumberComponentInt16:  PRINT( "%d", int16_t ); break;
        case DKNumberComponentInt32:  PRINT( "%d", int32_t ); break;
        case DKNumberComponentInt64:  PRINT( "%lld", int64_t ); break;
        
        case DKNumberComponentUInt8:  PRINT( "%u", uint8_t ); break;
        case DKNumberComponentUInt16: PRINT( "%u", uint16_t ); break;
        case DKNumberComponentUInt32: PRINT( "%u", uint32_t ); break;
        case DKNumberComponentUInt64: PRINT( "%llu", uint64_t ); break;
        
        case DKNumberComponentFloat:  PRINT( "%f", float ); break;
        case DKNumberComponentDouble: PRINT( "%lf", double ); break;
        
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

    CastFunction castFunction = GetCastFunction( srcType, dstType );
    castFunction( src, dst, dstCount );

    return dstCount;
}







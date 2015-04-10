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
#include "DKAllocation.h"
#include "DKComparison.h"
#include "DKDescription.h"
#include "DKEgg.h"



// Type Casting/Comparing ================================================================

#define DKFirstNumberType   DKEncodingTypeInt8
#define DKMaxNumberTypes    (DKEncodingTypeDouble - DKEncodingTypeInt8 + 1)

typedef union
{
    int8_t      _int8_t[1];     // Variable size
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

static CastFunction CastFunctionTable[DKMaxNumberTypes][DKMaxNumberTypes] =
{
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

static CastFunction GetCastFunction( DKEncoding xtype, DKEncoding ytype )
{
    DKAssert( DKEncodingIsNumber( xtype ) );
    DKAssert( DKEncodingIsNumber( ytype ) );

    uint32_t xbase = DKEncodingGetType( xtype ) - DKFirstNumberType;
    uint32_t ybase = DKEncodingGetType( ytype ) - DKFirstNumberType;
    
    return CastFunctionTable[xbase][ybase];
}




// Cmp Function Table --------------------------------------------------------------------
#define CmpFunctionTableRow( xtype )                                                    \
    {                                                                                   \
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

static CmpFunction CmpFunctionTable[DKMaxNumberTypes][DKMaxNumberTypes] =
{
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

static CmpFunction GetCmpFunction( DKEncoding xtype, DKEncoding ytype )
{
    DKAssert( DKEncodingIsNumber( xtype ) );
    DKAssert( DKEncodingIsNumber( ytype ) );

    uint32_t xbase = DKEncodingGetType( xtype ) - DKFirstNumberType;
    uint32_t ybase = DKEncodingGetType( ytype ) - DKFirstNumberType;
    
    return CmpFunctionTable[xbase][ybase];
}




// DKNumber ==============================================================================

struct DKNumber
{
    DKObject _obj;
    DKNumberValue value;
};


static struct DKNumber DKPlaceholderNumber =
{
    DKInitObjectHeader( NULL ),
};


static void *       DKNumberAllocPlaceholder( DKClassRef _class, size_t extraBytes );
static void         DKNumberDealloc( DKNumberRef _self );

static DKObjectRef  DKNumberInitWithEgg( DKNumberRef _self, DKEggUnarchiverRef egg );
static void         DKNumberAddToEgg( DKNumberRef _self, DKEggArchiverRef egg );


///
//  DKNumberClass()
//
DKThreadSafeClassInit( DKNumberClass )
{
    // NOTE: The value field of DKNumber is dynamically sized, and not included in the
    // base instance structure size.
    DKAssert( sizeof(struct DKNumber) == (sizeof(DKObject) + sizeof(DKNumberValue)) );
    DKClassRef cls = DKAllocClass( DKSTR( "DKNumber" ), DKObjectClass(), sizeof(DKObject), DKImmutableInstances, NULL, NULL );
    
    // Allocation
    struct DKAllocationInterface * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(struct DKAllocationInterface) );
    allocation->alloc = (DKAllocMethod)DKNumberAllocPlaceholder;
    allocation->dealloc = (DKDeallocMethod)DKNumberDealloc;

    DKInstallClassInterface( cls, allocation );
    DKRelease( allocation );
    
    // Comparison
    struct DKComparisonInterface * comparison = DKAllocInterface( DKSelector(Comparison), sizeof(struct DKComparisonInterface) );
    comparison->equal = (DKEqualityMethod)DKNumberEqual;
    comparison->compare = (DKCompareMethod)DKNumberCompare;
    comparison->hash = (DKHashMethod)DKNumberHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );
    
    // Description
    struct DKDescriptionInterface * description = DKAllocInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->getDescription = (DKGetDescriptionMethod)DKNumberGetDescription;
    description->getSizeInBytes = DKDefaultGetSizeInBytes;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Egg
    struct DKEggInterface * egg = DKAllocInterface( DKSelector(Egg), sizeof(struct DKEggInterface) );
    egg->initWithEgg = (DKInitWithEggMethod)DKNumberInitWithEgg;
    egg->addToEgg = (DKAddToEggMethod)DKNumberAddToEgg;
    
    DKInstallInterface( cls, egg );
    DKRelease( egg );

    return cls;
}


///
//  DKNumberAllocPlaceholder()
//
static void * DKNumberAllocPlaceholder( DKClassRef _class, size_t extraBytes )
{
    if( _class == DKNumberClass_SharedObject )
    {
        DKPlaceholderNumber._obj.isa = DKNumberClass_SharedObject;
        return &DKPlaceholderNumber;
    }
    
    DKAssert( 0 );
    return NULL;
}


///
//  DKNumberDealloc()
//
static void DKNumberDealloc( DKNumberRef _self )
{
    if( _self == &DKPlaceholderNumber )
        return;
    
    DKDeallocObject( _self );
}


///
//  DKNumberInit()
//
DKNumberRef DKNumberInit( DKNumberRef _self, const void * value, DKEncoding encoding )
{
    if( _self == &DKPlaceholderNumber  )
    {
        DKAssert( value != NULL );
        DKAssert( DKEncodingIsNumber( encoding ) );

        size_t size = DKEncodingGetSize( encoding );

        _self = DKAllocObject( DKNumberClass(), size );
        
        DKSetObjectTag( _self, encoding );
        memcpy( &_self->value, value, size );
    }
    
    else if( _self != NULL )
    {
        DKFatalError( "DKNumberInit: Trying to initialize a non-number object.\n" );
    }

    return _self;
}


///
//  DKNumberInitWithEgg()
//
static DKObjectRef DKNumberInitWithEgg( DKNumberRef _self, DKEggUnarchiverRef egg )
{
    if( _self == &DKPlaceholderNumber  )
    {
        DKEncoding encoding = DKEggGetEncoding( egg, DKSTR( "value" ) );
        DKAssert( DKEncodingIsNumber( encoding ) );

        size_t size = DKEncodingGetSize( encoding );

        _self = DKAllocObject( DKNumberClass(), size );
        
        DKSetObjectTag( _self, encoding );
        
        DKEggGetNumberData( egg, DKSTR( "value" ), &_self->value );
    }
    
    else if( _self != NULL )
    {
        DKFatalError( "DKNumberInitWithEgg: Trying to initialize a non-number object.\n" );
    }

    return _self;
}


///
//  DKNumberAddToEgg()
//
static void DKNumberAddToEgg( DKNumberRef _self, DKEggArchiverRef egg )
{
    DKEncoding encoding = DKGetObjectTag( _self );
    DKEggAddNumberData( egg, DKSTR( "value" ), encoding, &_self->value );
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
//  DKNumberGetEncoding()
//
DKEncoding DKNumberGetEncoding( DKNumberRef _self )
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
        
        DKEncoding encoding = DKGetObjectTag( _self );
        
        memcpy( value, &_self->value, DKEncodingGetSize( encoding ) );
        
        return DKEncodingGetCount( encoding );
    }

    return 0;
}


///
//  DKNumberCastValue()
//
size_t DKNumberCastValue( DKNumberRef _self, void * value, DKEncoding encoding )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKNumberClass() );
        
        return DKNumberConvert( &_self->value, DKGetObjectTag( _self ), value, encoding );
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

        DKEncoding a_encoding = DKGetObjectTag( a );
        DKEncoding b_encoding = DKGetObjectTag( b );

        size_t a_count = DKEncodingGetCount( a_encoding );
        size_t b_count = DKEncodingGetCount( b_encoding );

        if( a_count == b_count )
        {
            CmpFunction cmpFunction = GetCmpFunction( a_encoding, b_encoding );
            return cmpFunction( &a->value, &b->value, b_count );
        }
        
        else
        {
            size_t sa = DKEncodingGetSize( a_encoding );
            size_t sb = DKEncodingGetSize( b_encoding );
        
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
    
        DKEncoding encoding = DKGetObjectTag( _self );
        DKEncodingType type = DKEncodingGetType( encoding );
        size_t size = DKEncodingGetTypeSize( encoding );
        size_t count = DKEncodingGetCount( encoding );

        if( (count == 1) && (size <= sizeof(DKHashCode)) )
        {
            switch( type )
            {
            case DKEncodingTypeInt8:   return _self->value._int8_t[0];
            case DKEncodingTypeInt16:  return _self->value._int16_t[0];
            case DKEncodingTypeInt32:  return _self->value._int32_t[0];

            case DKEncodingTypeUInt8:  return _self->value._uint8_t[0];
            case DKEncodingTypeUInt16: return _self->value._uint16_t[0];
            case DKEncodingTypeUInt32: return _self->value._uint32_t[0];

            case DKEncodingTypeFloat:  return *((uint32_t *)&_self->value._float[0]);
            
            #if __LP64__
            case DKEncodingTypeInt64:  return _self->value._int64_t[0];
            case DKEncodingTypeUInt64: return _self->value._uint64_t[0];
            case DKEncodingTypeDouble: return *((uint64_t *)&_self->value._double[0]);
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
//  DKNumberGetDescription()
//
DKStringRef DKNumberGetDescription( DKNumberRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKNumberClass() );
        const struct DKNumber * number = _self;
        
        DKMutableStringRef desc = DKAutorelease( DKStringCreateMutable() );

        DKEncoding encoding = DKGetObjectTag( _self );
        DKEncodingType type = DKEncodingGetType( encoding );
        size_t count = DKEncodingGetCount( encoding );
        
        #define PRINT( fmt, field )                                         \
            DKSPrintf( desc, fmt, number->value._ ## field[0] );            \
            for( unsigned int i = 1; i < count; ++i )                       \
                DKSPrintf( desc, " " fmt, number->value._ ## field[i] )
        
        switch( type )
        {
        case DKEncodingTypeInt8:   PRINT( "%d", int8_t ); break;
        case DKEncodingTypeInt16:  PRINT( "%d", int16_t ); break;
        case DKEncodingTypeInt32:  PRINT( "%d", int32_t ); break;
        case DKEncodingTypeInt64:  PRINT( "%lld", int64_t ); break;
        
        case DKEncodingTypeUInt8:  PRINT( "%u", uint8_t ); break;
        case DKEncodingTypeUInt16: PRINT( "%u", uint16_t ); break;
        case DKEncodingTypeUInt32: PRINT( "%u", uint32_t ); break;
        case DKEncodingTypeUInt64: PRINT( "%llu", uint64_t ); break;
        
        case DKEncodingTypeFloat:  PRINT( "%f", float ); break;
        case DKEncodingTypeDouble: PRINT( "%lf", double ); break;
        
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
size_t DKNumberConvert( const void * src, DKEncoding srcEncoding, void * dst, DKEncoding dstEncoding )
{
    if( !DKEncodingIsNumber( srcEncoding ) )
        return 0;
    
    if( !DKEncodingIsNumber( dstEncoding ) )
        return 0;
    
    size_t dstCount = DKEncodingGetCount( dstEncoding );
    size_t srcCount = DKEncodingGetCount( srcEncoding );

    if( dstCount != srcCount )
        return 0;

    CastFunction castFunction = GetCastFunction( srcEncoding, dstEncoding );
    castFunction( src, dst, dstCount );

    return dstCount;
}







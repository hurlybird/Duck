/*****************************************************************************************

  DKPredicate.c

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

#include "DKPredicate.h"
#include "DKString.h"
#include "DKStream.h"
#include "DKNumber.h"
#include "DKCollection.h"
#include "DKEgg.h"
#include "DKComparison.h"
#include "DKCopying.h"
#include "DKDescription.h"



struct DKPredicate
{
    DKObject _obj;
    DKPredicateOp op;
    DKObjectRef a;
    DKObjectRef b;
};

typedef bool (*PredicateOpFunction)( DKObjectRef a, DKObjectRef b, DKObjectRef subst );

struct PredicateOpInfo
{
    DKPredicateOp op;
    DKStringRef name;
    PredicateOpFunction func;
};

static struct PredicateOpInfo PredicateOpInfoTable[DKMaxPredicateOp];
static bool PredicateOpTableInitialized = false;


static DKObjectRef  DKPredicateInitialize( DKObjectRef _self );
static void         DKPredicateFinalize( DKObjectRef _self );

static DKObjectRef  DKPredicateInitWithEgg( DKPredicateRef _self, DKEggUnarchiverRef egg );
static void         DKPredicateAddToEgg( DKPredicateRef _self, DKEggArchiverRef egg );

static DKStringRef  DKPredicateGetDescription( DKObjectRef _self );

static const struct PredicateOpInfo * GetPredicateOpInfo( DKPredicateOp op );
static void         InitPredicateOpInfoTable( void );

static bool         DKEvaluateInternal( DKObjectRef obj, DKObjectRef subst );



// DKPredicate ===========================================================================

///
//  DKPredicateClass()
//
DKThreadSafeClassInit( DKPredicateClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKPredicate" ), DKObjectClass(), sizeof(struct DKPredicate), 0, DKPredicateInitialize, DKPredicateFinalize );
    
    // Copying
    struct DKCopyingInterface * copying = DKNewInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKRetain;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // Description
    struct DKDescriptionInterface * description = DKNewInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->getDescription = (DKGetDescriptionMethod)DKPredicateGetDescription;
    description->getSizeInBytes = DKDefaultGetSizeInBytes;
    
    DKInstallInterface( cls, description );
    DKRelease( description );
    
    // Egg
    struct DKEggInterface * egg = DKNewInterface( DKSelector(Egg), sizeof(struct DKEggInterface) );
    egg->initWithEgg = (DKInitWithEggMethod)DKPredicateInitWithEgg;
    egg->addToEgg = (DKAddToEggMethod)DKPredicateAddToEgg;
    
    DKInstallInterface( cls, egg );
    DKRelease( egg );

    return cls;
}


///
//  DKPredicateInitialize()
//
static DKObjectRef DKPredicateInitialize( DKObjectRef _self )
{
    return _self;
}


///
//  DKPredicateFinalize()
//
static void DKPredicateFinalize( DKObjectRef _untyped_self )
{
    DKPredicateRef _self = _untyped_self;

    DKRelease( _self->a );
    DKRelease( _self->b );
}


///
//  DKPredicateInitWithEgg()
//
static DKObjectRef DKPredicateInitWithEgg( DKPredicateRef _self, DKEggUnarchiverRef egg )
{
    DKStringRef op = DKEggGetObject( egg, DKSTR( "op" ) );
    _self->op = DKPredicateOpFromString( op );

    _self->a = DKRetain( DKEggGetObject( egg, DKSTR( "a" ) ) );

    _self->b = DKRetain( DKEggGetObject( egg, DKSTR( "b" ) ) );

    return _self;
}


///
//  DKPredicateAddToEgg()
//
static void DKPredicateAddToEgg( DKPredicateRef _self, DKEggArchiverRef egg )
{
    DKStringRef op = DKStringFromPredicateOp( _self->op );
    DKEggAddObject( egg, DKSTR( "op" ), op );

    DKEggAddObject( egg, DKSTR( "a" ), _self->a );

    DKEggAddObject( egg, DKSTR( "b" ), _self->b );
}


///
//  DKPredicateGetDescription()
//
static DKStringRef DKPredicateGetDescription( DKObjectRef _untyped_self )
{
    DKPredicateRef _self = _untyped_self;

    if( (_self->op == DKPredicateFALSE) || (_self->op == DKPredicateTRUE) )
        return DKStringFromPredicateOp( _self->op );

    DKMutableStringRef desc = DKMutableString();

    DKObjectRef a = _self->a ? _self->a : DKSTR( "*" );
    DKObjectRef b = _self->b ? _self->b : DKSTR( "*" );

    if( (_self->op == DKPredicateNOT) ||
        ((_self->op == DKPredicateAND) && (_self->b == NULL)) ||
        ((_self->op == DKPredicateOR) && (_self->b == NULL)) )
    {
    
        DKSPrintf( desc, "%@ %@", DKStringFromPredicateOp( _self->op ), a );
        return desc;
    }
    
    else
    {
        DKSPrintf( desc, "%@ %@ %@", a, DKStringFromPredicateOp( _self->op ), b );
        return desc;
    }
}


///
//  DKPredicateCreate()
//
DKObjectRef DKPredicateInit( DKObjectRef _untyped_self, DKPredicateOp op, DKObjectRef a, DKObjectRef b )
{
    DKPredicateRef _self = _untyped_self;

    if( _self )
    {
        _self->op = op;
        _self->a = DKCopy( a );
        _self->b = DKCopy( b );
    }
    
    return _self;
}


///
//  DKPredicateEvaluate()
//
bool DKPredicateEvaluate( DKPredicateRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKPredicateClass() );

        const struct PredicateOpInfo * info = GetPredicateOpInfo( _self->op );
        
        return info->func( _self->a, _self->b, NULL );
    }

    return false;
}


///
//  DKPredicateEvaluateWithObject()
//
bool DKPredicateEvaluateWithObject( DKPredicateRef _self, DKObjectRef subst )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKPredicateClass() );

        const struct PredicateOpInfo * info = GetPredicateOpInfo( _self->op );

        DKObjectRef a = _self->a ? _self->a : subst;
        DKObjectRef b = _self->b ? _self->b : subst;

        return info->func( a, b, subst );
    }
    
    return false;
}




// Predicate Functions ===================================================================

///
//  EvaluateTRUE()
//
static bool EvaluateTRUE( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return true;
}


///
//  EvaluateFALSE()
//
static bool EvaluateFALSE( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return false;
}


///
//  EvaluateNOT()
//
static bool EvaluateNOT( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return !DKEvaluateInternal( a, subst );
}


///
//  EvaluateAND()
//
static int EvaluateANDCallback( DKObjectRef obj, void * context )
{
    if( DKEvaluateInternal( obj, context ) )
        return 0;

    return 1;
}

static bool EvaluateAND( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    DKCollectionInterfaceRef collectionInterface;
    
    if( (b == NULL) && DKQueryInterface( a, DKSelector(Collection), (DKInterfaceRef *)&collectionInterface ) )
    {
        int result = collectionInterface->foreachObject( a, EvaluateANDCallback, subst );
        return result == 0;
    }

    return DKEvaluateInternal( a, subst ) && DKEvaluateInternal( b, subst );
}


///
//  EvaluateOR()
//
static int EvaluateORCallback( DKObjectRef obj, void * context )
{
    if( DKEvaluateInternal( obj, context ) )
        return 1;
    
    return 0;
}

static bool EvaluateOR( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    DKCollectionInterfaceRef collectionInterface;
    
    if( (b == NULL) && DKQueryInterface( a, DKSelector(Collection), (DKInterfaceRef *)&collectionInterface ) )
    {
        int result = collectionInterface->foreachObject( a, EvaluateORCallback, subst );
        return result != 0;
    }

    return DKEvaluateInternal( a, subst ) || DKEvaluateInternal( b, subst );
}


///
//  EvaluateIS()
//
static bool EvaluateIS( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return a == b;
}


///
//  EvaluateISNT()
//
static bool EvaluateISNT( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return a != b;
}


///
//  EvaluateEQ()
//
static bool EvaluateEQ( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    if( DKIsKindOfClass( a, DKPredicateClass() ) ||
        DKIsKindOfClass( b, DKPredicateClass() ) )
    {
        bool eval_a = DKEvaluateInternal( a, subst );
        bool eval_b = DKEvaluateInternal( b, subst );
        
        return eval_a == eval_b;
    }

    return DKEqual( a, b );
}


///
//  EvaluateNEQ()
//
static bool EvaluateNEQ( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return !EvaluateEQ( a, b, subst );
}


///
//  EvaluateLT()
//
static bool EvaluateLT( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    int cmp = DKCompare( a, b );
    return cmp > 0;
}


///
//  EvaluateLTE()
//
static bool EvaluateLTE( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    int cmp = DKCompare( a, b );
    return cmp >= 0;
}


///
//  EvaluateGT()
//
static bool EvaluateGT( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    int cmp = DKCompare( a, b );
    return cmp < 0;
}


///
//  EvaluateGTE()
//
static bool EvaluateGTE( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    int cmp = DKCompare( a, b );
    return cmp <= 0;
}


///
//  EvaluateIN()
//
static bool EvaluateIN( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    if( DKIsKindOfClass( b, DKStringClass() ) )
        return DKStringHasSubstring( b, a );

    return DKContainsObject( b, a );
}


///
//  EvaluateKEYIN()
//
static bool EvaluateKEYIN( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return DKContainsKey( b, a );
}


///
//  EvaluateISA()
//
static bool EvaluateISA( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    if( DKIsKindOfClass( b, DKClassClass() ) )
        return DKIsKindOfClass( a, b );
    
    else if( DKIsKindOfClass( b, DKSelectorClass() ) )
        return DKQueryInterface( a, b, NULL );
    
    return false;
}


///
//  EvaluateHASPREFIX()
//
static bool EvaluateHASPREFIX( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return DKStringHasPrefix( a, b );
}


///
//  EvaluateHASSUFFIX()
//
static bool EvaluateHASSUFFIX( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return DKStringHasSuffix( a, b );
}




// Predicate Info Table ==================================================================

///
//  GetPredicateOpInfo()
//
static const struct PredicateOpInfo * GetPredicateOpInfo( DKPredicateOp op )
{
    DKAssert( (op >= 0) && (op < DKMaxPredicateOp) );

    InitPredicateOpInfoTable();
    
    return &PredicateOpInfoTable[op];
}


///
//  InitPredicateOpInfoTable
//
static void InitPredicateOpInfoTable( void )
{
    if( !PredicateOpTableInitialized )
    {
        #define InitTableRow( value )                           \
        {                                                       \
            int op = DKPredicate ## value;                      \
            PredicateOpInfoTable[op].op = op;                   \
            PredicateOpInfoTable[op].name = DKSTR( #value );    \
            PredicateOpInfoTable[op].func = Evaluate ## value;  \
        }
        
        // Constants
        InitTableRow( FALSE );
        InitTableRow( TRUE );

        // Logic
        InitTableRow( NOT );
        InitTableRow( AND );
        InitTableRow( OR );
        InitTableRow( IS );
        InitTableRow( ISNT );
        
        // DKEqual( A, B )
        InitTableRow( EQ );
        InitTableRow( NEQ );
        
        // DKCompare( A, B )
        InitTableRow( LT );
        InitTableRow( LTE );
        InitTableRow( GT );
        InitTableRow( GTE );

        // Is a member of collection, or substring of string
        InitTableRow( IN );
        
        // Is a key in a keyed collection
        InitTableRow( KEYIN );

        // Is kind of class, or implements an interface
        InitTableRow( ISA );
        
        // Contains a String prefix/suffix
        InitTableRow( HASPREFIX );
        InitTableRow( HASSUFFIX );
        
        #undef InitTableRow
    
        PredicateOpTableInitialized = true;
    };
}


///
//  DKStringFromPredicateOp()
//
DKStringRef DKStringFromPredicateOp( DKPredicateOp op )
{
    DKAssert( (op >= 0) && (op < DKMaxPredicateOp) );

    const struct PredicateOpInfo * info = GetPredicateOpInfo( op );
    return info->name;
}


///
//  DKPredicateOpFromString()
//
DKPredicateOp DKPredicateOpFromString( DKStringRef str )
{
    InitPredicateOpInfoTable();

    for( int i = 0; i < DKMaxPredicateOp; i++ )
    {
        if( DKStringEqualToString( PredicateOpInfoTable[i].name, str ) )
            return PredicateOpInfoTable[i].op;
    }
    
    return DKPredicateFALSE;
}


///
//  DKEvaluateInternal()
//
static bool DKEvaluateInternal( DKObjectRef obj, DKObjectRef subst )
{
    if( DKIsKindOfClass( obj, DKPredicateClass() ) )
    {
        return DKPredicateEvaluateWithObject( obj, subst );
    }
    
    if( DKIsKindOfClass( obj, DKNumberClass() ) )
    {
        DKEncoding encoding = DKNumberGetEncoding( obj );
        
        if( DKEncodingGetCount( encoding ) == 1 )
        {
            if( DKEncodingIsInteger( encoding ) )
            {
                int64_t val;
                DKNumberCastValue( obj, &val, DKNumberInt64 );
                
                return val != 0;
            }
            
            if( DKEncodingIsReal( encoding ) )
            {
                double val;
                DKNumberCastValue( obj, &val, DKNumberDouble );
                
                return val != 0.0;
            }
        }
    }
    
    if( DKIsKindOfClass( obj, DKStringClass() ) )
    {
        const char * cstr = DKStringGetCStringPtr( obj );
        
        if( strcmp( cstr, "1" ) == 0 )
            return true;
           
        if( strcasecmp( cstr, "true" ) == 0 )
            return true;

        if( strcasecmp( cstr, "yes" ) == 0 )
            return true;
    }

    return false;
}


///
//  DKEvaluate()
//
bool DKEvaluate( DKObjectRef obj )
{
    return DKEvaluateInternal( obj, NULL );
}







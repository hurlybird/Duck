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
#include "DKDictionary.h"
#include "DKStream.h"
#include "DKNumber.h"
#include "DKCollection.h"
#include "DKEgg.h"
#include "DKComparison.h"
#include "DKConversion.h"
#include "DKCopying.h"
#include "DKDescription.h"



struct DKPredicate
{
    DKObject _obj;
    
    DKPredicateOp op;
    
    DKObjectRef obj_a;
    DKStringRef key_a;
    
    DKObjectRef obj_b;
    DKStringRef key_b;
};

typedef bool (*PredicateOpFunction)( DKObjectRef a, DKObjectRef b, DKObjectRef subst );

typedef struct
{
    DKPredicateOp op;
    DKStringRef name;
    PredicateOpFunction func;
    
} PredicateOpInfo;

static DKObjectRef  DKPredicateInitialize( DKObjectRef _self );
static void         DKPredicateFinalize( DKObjectRef _self );

static void         DKPredicateParse( DKPredicateRef _self, const char * fmt, va_list args );

static DKObjectRef  DKPredicateInitWithEgg( DKPredicateRef _self, DKEggUnarchiverRef egg );
static void         DKPredicateAddToEgg( DKPredicateRef _self, DKEggArchiverRef egg );

static DKStringRef  DKPredicateGetDescription( DKObjectRef _self );

static const PredicateOpInfo * GetPredicateOpInfo( DKPredicateOp op );

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

    DKRelease( _self->obj_a );
    DKRelease( _self->key_a );
    
    DKRelease( _self->obj_b );
    DKRelease( _self->key_b );
}


///
//  DKPredicateInitWithEgg()
//
static DKObjectRef DKPredicateInitWithEgg( DKPredicateRef _self, DKEggUnarchiverRef egg )
{
    DKStringRef op = DKEggGetObject( egg, DKSTR( "op" ) );
    _self->op = DKPredicateOpFromString( op );

    _self->obj_a = DKRetain( DKEggGetObject( egg, DKSTR( "obj_a" ) ) );
    _self->key_a = DKRetain( DKEggGetObject( egg, DKSTR( "key_a" ) ) );

    _self->obj_b = DKRetain( DKEggGetObject( egg, DKSTR( "obj_b" ) ) );
    _self->key_b = DKRetain( DKEggGetObject( egg, DKSTR( "key_b" ) ) );

    return _self;
}


///
//  DKPredicateAddToEgg()
//
static void DKPredicateAddToEgg( DKPredicateRef _self, DKEggArchiverRef egg )
{
    DKStringRef op = DKStringFromPredicateOp( _self->op );
    DKEggAddObject( egg, DKSTR( "op" ), op );

    DKEggAddObject( egg, DKSTR( "obj_a" ), _self->obj_a );
    DKEggAddObject( egg, DKSTR( "key_a" ), _self->key_a );

    DKEggAddObject( egg, DKSTR( "obj_b" ), _self->obj_b );
    DKEggAddObject( egg, DKSTR( "key_b" ), _self->key_b );
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

    DKObjectRef obj_a = _self->obj_a ? _self->obj_a : DKSTR( "*" );
    DKObjectRef obj_b = _self->obj_b ? _self->obj_b : DKSTR( "*" );

    if( (_self->op == DKPredicateNOT) ||
        (_self->op == DKPredicateALL) ||
        (_self->op == DKPredicateANY) ||
        (_self->op == DKPredicateNONE) )
    {
    
        if( _self->key_a )
            DKSPrintf( desc, "%@ %@.%@", DKStringFromPredicateOp( _self->op ), obj_a, _self->key_a );
        
        else
            DKSPrintf( desc, "%@ %@", DKStringFromPredicateOp( _self->op ), obj_a );
        
        return desc;
    }
    
    else
    {
        if( _self->key_a )
        {
            if( _self->key_b )
                DKSPrintf( desc, "%@.%@ %@ %@.%@", obj_a, _self->key_a, DKStringFromPredicateOp( _self->op ), obj_b, _self->key_b );
            
            else
                DKSPrintf( desc, "%@.%@ %@ %@", obj_a, _self->key_a, DKStringFromPredicateOp( _self->op ), obj_b );
        }
        
        else
        {
            if( _self->key_b )
                DKSPrintf( desc, "%@ %@ %@.%@", obj_a, DKStringFromPredicateOp( _self->op ), obj_b, _self->key_b );
            
            else
                DKSPrintf( desc, "%@ %@ %@", obj_a, DKStringFromPredicateOp( _self->op ), obj_b );
        }
        
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
        _self->obj_a = DKCopy( a );
        _self->obj_b = DKCopy( b );
    }
    
    return _self;
}


///
//  DKPredicateInitWithFormat()
//
DKObjectRef DKPredicateInitWithFormat( DKObjectRef _untyped_self, DKStringRef fmt, ... )
{
    DKPredicateRef _self = _untyped_self;

    if( _self )
    {
        va_list arg_ptr;
        va_start( arg_ptr, fmt );
        
        DKPredicateParse( _self, DKStringGetCStringPtr( fmt ), arg_ptr );
        
        va_end( arg_ptr );
    }
    
    return _self;
}


///
//  DKPredicateParse()
//
static void DKPredicateParse( DKPredicateRef _self, const char * fmt, va_list args )
{
    DKAssert( 0 );
    
    // Do stuff here
}


///
//  DKPredicateEvaluate()
//
bool DKPredicateEvaluate( DKPredicateRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKPredicateClass() );

        const PredicateOpInfo * info = GetPredicateOpInfo( _self->op );
        
        DKObjectRef a = _self->obj_a;
        DKObjectRef b = _self->obj_b;

        if( _self->key_a )
            a = DKGetPropertyForKeyPath( a, _self->key_a );
        
        if( _self->key_b )
            b = DKGetPropertyForKeyPath( b, _self->key_a );
        
        return info->func( a, b, NULL );
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

        const PredicateOpInfo * info = GetPredicateOpInfo( _self->op );

        DKObjectRef a = _self->obj_a ? _self->obj_a : subst;
        DKObjectRef b = _self->obj_b ? _self->obj_b : subst;

        if( _self->key_a )
            a = DKGetPropertyForKeyPath( a, _self->key_a );
        
        if( _self->key_b )
            b = DKGetPropertyForKeyPath( b, _self->key_a );

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
static bool EvaluateAND( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return DKEvaluateInternal( a, subst ) && DKEvaluateInternal( b, subst );
}


///
//  EvaluateOR()
//
static bool EvaluateOR( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
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
//  EvaluateALL()
//
static int EvaluateALLCallback( DKObjectRef obj, void * context )
{
    if( DKEvaluateInternal( obj, context ) )
        return 0;

    return 1;
}

static bool EvaluateALL( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    DKCollectionInterfaceRef collectionInterface;
    
    if( DKQueryInterface( a, DKSelector(Collection), (DKInterfaceRef *)&collectionInterface ) )
    {
        int result = collectionInterface->foreachObject( a, EvaluateALLCallback, subst );
        return result == 0;
    }

    return false;
}


///
//  EvaluateANY()
//
static int EvaluateANYCallback( DKObjectRef obj, void * context )
{
    if( DKEvaluateInternal( obj, context ) )
        return 1;
    
    return 0;
}

static bool EvaluateANY( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    DKCollectionInterfaceRef collectionInterface;
    
    if( DKQueryInterface( a, DKSelector(Collection), (DKInterfaceRef *)&collectionInterface ) )
    {
        int result = collectionInterface->foreachObject( a, EvaluateANYCallback, subst );
        return result != 0;
    }

    return false;
}


///
//  EvaluateNONE()
//
static bool EvaluateNONE( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return !EvaluateANY( a, b, subst );
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
//  EvaluateCONTAINS()
//
static bool EvaluateCONTAINS( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    if( DKIsKindOfClass( b, DKStringClass() ) )
        return DKStringHasSubstring( a, b );

    return DKContainsObject( a, b );
}


///
//  EvaluateKEYIN()
//
static bool EvaluateKEYIN( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return DKContainsKey( b, a );
}


///
//  EvaluateCONTAINSKEY()
//
static bool EvaluateCONTAINSKEY( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return DKContainsKey( a, b );
}


///
//  EvaluateISA()
//
static bool EvaluateISA( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    if( a == NULL )
        return true;

    if( DKIsKindOfClass( b, DKClassClass() ) )
        return DKIsKindOfClass( a, b );
    
    else if( DKIsKindOfClass( b, DKSelectorClass() ) )
        return DKQueryInterface( a, b, NULL );
    
    return false;
}


///
//  EvaluateLIKE()
//
static bool EvaluateLIKE( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return DKStringEqualToString( a, b );
}


///
//  EvaluateBEGINSWITH()
//
static bool EvaluateBEGINSWITH( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return DKStringHasPrefix( a, b );
}


///
//  EvaluateENDSWITH()
//
static bool EvaluateENDSWITH( DKObjectRef a, DKObjectRef b, DKObjectRef subst )
{
    return DKStringHasSuffix( a, b );
}




// Predicate Info Table ==================================================================

DKThreadSafeStaticObjectInit( PredicateOpInfoTable, PredicateOpInfo * )
{
    PredicateOpInfo * table = malloc( sizeof(PredicateOpInfo) * DKMaxPredicateOp );
    
    #define InitTableRow( value )               \
    {                                           \
        int op = DKPredicate ## value;          \
        table[op].op = op;                      \
        table[op].name = DKSTR( #value );       \
        table[op].func = Evaluate ## value;     \
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

    // Aggregate logic
    InitTableRow( ALL );
    InitTableRow( ANY );
    InitTableRow( NONE );

    // Is a member of collection, or substring of string
    InitTableRow( IN );
    InitTableRow( CONTAINS );
    
    // Is a key in a keyed collection
    InitTableRow( KEYIN );
    InitTableRow( CONTAINSKEY );

    // Is kind of class, or implements an interface
    InitTableRow( ISA );
    
    // String comparison
    InitTableRow( LIKE );
    InitTableRow( BEGINSWITH );
    InitTableRow( ENDSWITH );
    
    #undef InitTableRow
    
    return table;
}


DKThreadSafeStaticObjectInit( PredicateOpLookupTable, DKDictionaryRef )
{
    DKMutableDictionaryRef dict = DKNewMutableDictionary();
    
    PredicateOpInfo * table = PredicateOpInfoTable();

    for( int i = 0; i < DKMaxPredicateOp; i++ )
    {
        DKNumberRef op = DKNewNumberWithInt32( table[i].op );
        DKDictionaryAddObject( dict, table[i].name, op );
        DKRelease( op );
    }
    
    // Aliases for common operators
    DKNumberRef eq = DKNewNumberWithInt32( DKPredicateEQ );
    DKDictionaryAddObject( dict, DKSTR( "=" ), eq );
    DKDictionaryAddObject( dict, DKSTR( "==" ), eq );
    DKRelease( eq );

    DKNumberRef neq = DKNewNumberWithInt32( DKPredicateEQ );
    DKDictionaryAddObject( dict, DKSTR( "!=" ), neq );
    DKDictionaryAddObject( dict, DKSTR( "<>" ), neq );
    DKRelease( neq );

    DKNumberRef gt = DKNewNumberWithInt32( DKPredicateEQ );
    DKDictionaryAddObject( dict, DKSTR( ">" ), gt );
    DKRelease( gt );

    DKNumberRef lt = DKNewNumberWithInt32( DKPredicateEQ );
    DKDictionaryAddObject( dict, DKSTR( "<" ), lt );
    DKRelease( lt );

    DKNumberRef gte = DKNewNumberWithInt32( DKPredicateEQ );
    DKDictionaryAddObject( dict, DKSTR( ">=" ), gte );
    DKRelease( gte );

    DKNumberRef lte = DKNewNumberWithInt32( DKPredicateEQ );
    DKDictionaryAddObject( dict, DKSTR( "<=" ), lte );
    DKRelease( lte );

    DKNumberRef and = DKNewNumberWithInt32( DKPredicateEQ );
    DKDictionaryAddObject( dict, DKSTR( "&&" ), and );
    DKRelease( and );

    DKNumberRef or = DKNewNumberWithInt32( DKPredicateEQ );
    DKDictionaryAddObject( dict, DKSTR( "||" ), or );
    DKRelease( or );

    DKNumberRef not = DKNewNumberWithInt32( DKPredicateEQ );
    DKDictionaryAddObject( dict, DKSTR( "!" ), not );
    DKRelease( not );
    
    return dict;
}


///
//  GetPredicateOpInfo()
//
static const PredicateOpInfo * GetPredicateOpInfo( DKPredicateOp op )
{
    DKAssert( (op >= 0) && (op < DKMaxPredicateOp) );

    PredicateOpInfo * table = PredicateOpInfoTable();
    
    return &table[op];
}


///
//  DKStringFromPredicateOp()
//
DKStringRef DKStringFromPredicateOp( DKPredicateOp op )
{
    DKAssert( (op >= 0) && (op < DKMaxPredicateOp) );

    const PredicateOpInfo * info = GetPredicateOpInfo( op );
    return info->name;
}


///
//  DKPredicateOpFromString()
//
DKPredicateOp DKPredicateOpFromString( DKStringRef str )
{
    DKNumberRef op = DKDictionaryGetObject( PredicateOpLookupTable(), str );
    return (DKPredicateOp)DKNumberGetInt32( op );
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
    
    return DKGetBool( obj );
}


///
//  DKEvaluate()
//
bool DKEvaluate( DKObjectRef obj )
{
    return DKEvaluateInternal( obj, NULL );
}







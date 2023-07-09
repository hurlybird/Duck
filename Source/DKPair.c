/*****************************************************************************************

  DKPair.c

  Copyright (c) 2017 Derek W. Nylen

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
#include "DKPair.h"
#include "DKString.h"
#include "DKEgg.h"
#include "DKComparison.h"


static void DKPairFinalize( DKObjectRef _untyped_self );

static DKObjectRef DKPairInitWithEgg( DKObjectRef _self, DKEggUnarchiverRef egg );
static void DKPairAddToEgg( DKObjectRef _self, DKEggArchiverRef egg );

static bool DKPairEqual( DKPairRef _self, DKObjectRef other );
static int DKPairCompare( DKPairRef _self, DKObjectRef other );
static DKHashCode DKPairHash( DKPairRef ptr );


DKThreadSafeClassInit( DKPairClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKPair" ), DKObjectClass(), sizeof(struct DKPair), 0, NULL, DKPairFinalize );

    // Comparison
    struct DKComparisonInterface * comparison = DKNewInterface( DKSelector(Comparison), sizeof(struct DKComparisonInterface) );
    comparison->equal = (DKEqualityMethod)DKPairEqual;
    comparison->compare = (DKCompareMethod)DKPairCompare;
    comparison->hash = (DKHashMethod)DKPairHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );

    // Egg
    struct DKEggInterface * egg = DKNewInterface( DKSelector(Egg), sizeof(struct DKEggInterface) );
    egg->initWithEgg = (DKInitWithEggMethod)DKPairInitWithEgg;
    egg->addToEgg = (DKAddToEggMethod)DKPairAddToEgg;
    
    DKInstallInterface( cls, egg );
    DKRelease( egg );

    return cls;
}


///
//  DKPairInit()
//
DKObjectRef DKPairInit( DKObjectRef _untyped_self, DKObjectRef first, DKObjectRef second )
{
    DKPairRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
        _self->first = DKRetain( first );
        _self->second = DKRetain( second );
    }
    
    return _self;
}


///
//  DKPairFinalize()
//
static void DKPairFinalize( DKObjectRef _untyped_self )
{
    DKPairRef _self = _untyped_self;
    
    DKRelease( _self->first );
    DKRelease( _self->second );
}


///
//  DKPairInitWithEgg()
//
static DKObjectRef DKPairInitWithEgg( DKObjectRef _untyped_self, DKEggUnarchiverRef egg )
{
    DKPairRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
        _self->first = DKRetain( DKEggGetObject( egg, DKSTR( "first" ) ) );
        _self->second = DKRetain( DKEggGetObject( egg, DKSTR( "second" ) ) );
    }

    return _self;
}


///
//  DKPairAddToEgg()
//
static void DKPairAddToEgg( DKObjectRef _untyped_self, DKEggArchiverRef egg )
{
    DKPairRef _self = _untyped_self;

    DKEggAddObject( egg, DKSTR( "first" ), _self->first );
    DKEggAddObject( egg, DKSTR( "second" ), _self->second );
}


///
//  DKPairGetFirstObject()
//
DKObjectRef DKPairGetFirstObject( DKPairRef _self )
{
    return _self ? _self->first : NULL;
}


///
//  DKPairGetSecondObject()
//
DKObjectRef DKPairGetSecondObject( DKPairRef _self )
{
    return _self ? _self->second : NULL;
}


///
//  DKPairEqual()
//
static bool DKPairEqual( DKPairRef _self, DKObjectRef other )
{
    if( DKIsKindOfClass( other, DKPairClass() ) )
    {
        DKAssertKindOfClass( _self, DKPairClass() );
        
        return DKEqual( _self->first, DKPairGetFirstObject( other ) ) &&
            DKEqual( _self->second, DKPairGetSecondObject( other ) );
    }
    
    return false;
}


///
//  DKPairCompare()
//
static int DKPairCompare( DKPairRef _self, DKObjectRef other )
{
    if( DKIsKindOfClass( other, DKPairClass() ) )
    {
        DKAssertKindOfClass( _self, DKPairClass() );
        
        int cmp = DKCompare( _self->first, DKPairGetFirstObject( other ) );
        
        if( cmp == 0 )
            cmp = DKCompare( _self->second, DKPairGetSecondObject( other ) );
        
        return cmp;
    }

    return DKPointerCompare( _self, other );
}


///
//  DKPairHash()
//
static DKHashCode DKPairHash( DKPairRef _self )
{
    DKAssertKindOfClass( _self, DKPairClass() );
    
    DKHashCode hash1 = DKHash( _self->first );
    DKHashCode hash2 = DKHash( _self->second );

    return hash1 ^ hash2;
}







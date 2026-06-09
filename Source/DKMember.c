// =======================================================================================
//
// DKMember.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKMember.h"
#include "DKString.h"
#include "DKEgg.h"
#include "DKComparison.h"


static void DKMemberFinalize( DKObjectRef _untyped_self );

static DKObjectRef DKMemberInitWithEgg( DKObjectRef _self, DKEggUnarchiverRef egg );
static void DKMemberAddToEgg( DKObjectRef _self, DKEggArchiverRef egg );

static bool DKMemberEqual( DKMemberRef _self, DKObjectRef other );
static int DKMemberCompare( DKMemberRef _self, DKObjectRef other );
static DKHashCode DKMemberHash( DKMemberRef ptr );


DKThreadSafeClassInit( DKMemberClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKMember" ), DKObjectClass(), sizeof(struct DKMember), 0, NULL, DKMemberFinalize );

    // Comparison
    struct DKComparisonInterface * comparison = DKNewInterface( DKSelector(Comparison) );
    comparison->equal = (DKEqualityMethod)DKMemberEqual;
    comparison->compare = (DKCompareMethod)DKMemberCompare;
    comparison->hash = (DKHashMethod)DKMemberHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );

    // Egg
    struct DKEggInterface * egg = DKNewInterface( DKSelector(Egg) );
    egg->initWithEgg = (DKInitWithEggMethod)DKMemberInitWithEgg;
    egg->addToEgg = (DKAddToEggMethod)DKMemberAddToEgg;
    
    DKInstallInterface( cls, egg );
    DKRelease( egg );

    return cls;
}


///
//  DKMemberInit()
//
DKObjectRef DKMemberInit( DKObjectRef _untyped_self, DKObjectRef object, size_t offset, DKEncoding encoding )
{
    DKMemberRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
        _self->object = DKRetain( object );
        _self->offset = offset;
        DKSetObjectTag( _self, encoding );
    }
    
    return _self;
}


///
//  DKMemberFinalize()
//
static void DKMemberFinalize( DKObjectRef _untyped_self )
{
    DKMemberRef _self = _untyped_self;
    
    DKRelease( _self->object );
}


///
//  DKMemberInitWithEgg()
//
static DKObjectRef DKMemberInitWithEgg( DKObjectRef _untyped_self, DKEggUnarchiverRef egg )
{
    DKMemberRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
        _self->object = DKRetain( DKEggGetObject( egg, DKSTR( "object" ) ) );
        
        DKEncoding offsetEncoding = DKEggGetEncoding( egg, DKSTR( "member" ) );
        DKRequire( offsetEncoding == DKEncode( DKEncodingTypeUInt64, 2 ) );
        
        uint64_t tmp[2];
        DKEggGetNumberData( egg, DKSTR( "member" ), tmp );

        _self->offset = (size_t)tmp[0];
        DKSetObjectTag( _self, (int32_t)tmp[1] );
    }

    return _self;
}


///
//  DKMemberAddToEgg()
//
static void DKMemberAddToEgg( DKObjectRef _untyped_self, DKEggArchiverRef egg )
{
    DKMemberRef _self = _untyped_self;

    DKEggAddObject( egg, DKSTR( "object" ), _self->object );
    
    uint64_t tmp[2];
    tmp[0] = _self->offset;
    tmp[1] = DKGetObjectTag( _self );
    
    DKEggAddNumberData( egg, DKSTR( "member" ), DKEncode( DKEncodingTypeUInt64, 2 ), tmp );
}


///
//  DKMemberGetObject()
//
DKObjectRef DKMemberGetObject( DKMemberRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMemberClass() );
        
        return _self->object;
    }

    return NULL;
}


///
//  DKMemberGetOffset()
//
size_t DKMemberGetOffset( DKMemberRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMemberClass() );
        
        return _self->offset;
    }

    return 0;
}


///
//  DKMemberGetEncoding()
//
DKEncoding DKMemberGetEncoding( DKMemberRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMemberClass() );
        
        return DKGetObjectTag( _self );
    }

    return DKEncodingTypeNone;
}


///
//  DKMemberGetValuePtr()
//
DK_API const void * DKMemberGetValuePtr( DKMemberRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMemberClass() );
        
        return (uint8_t *)_self->object + _self->offset;
    }
    
    return NULL;
}


///
//  DKMemberQueryValuePtr()
//
const void* DKMemberQueryValuePtr( DKMemberRef _self, DKEncoding * encoding )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMemberClass() );
        
        *encoding = DKGetObjectTag( _self );
        
        return (uint8_t *)_self->object + _self->offset;
    }
    
    return NULL;
}


///
//  DKMemberEqual()
//
static bool DKMemberEqual( DKMemberRef _self, DKObjectRef other )
{
    if( DKIsKindOfClass( other, DKMemberClass() ) )
    {
        DKAssertKindOfClass( _self, DKMemberClass() );

        DKMemberRef _other = other;
        
        return DKEqual( _self->object, _other->object ) &&
            (_self->offset == _other->offset) &&
            (DKGetObjectTag( _self ) == DKGetObjectTag( _other ));
    }
    
    return false;
}


///
//  DKMemberCompare()
//
static int DKMemberCompare( DKMemberRef _self, DKObjectRef other )
{
    if( DKIsKindOfClass( other, DKMemberClass() ) )
    {
        DKAssertKindOfClass( _self, DKMemberClass() );

        DKMemberRef _other = other;

        int cmp = DKCompare( _self->object, _other->object );
        
        if( cmp == 0 )
            cmp = (int)((int64_t)_self->offset - (int64_t)_other->offset);
        
        if( cmp == 0 )
            cmp = DKGetObjectTag( _self ) - DKGetObjectTag( _other );
        
        return cmp;
    }

    return DKPointerCompare( _self, other );
}


///
//  DKMemberHash()
//
static DKHashCode DKMemberHash( DKMemberRef _self )
{
    DKAssertKindOfClass( _self, DKMemberClass() );
    
    DKHashCode hash1 = DKHash( _self->object );
    DKHashCode hash2 = _self->offset;
    DKHashCode hash3 = DKGetObjectTag( _self );

    return hash1 ^ hash2 ^ hash3;
}







// =======================================================================================
//
// DKCharacterSet.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKGenericArray.h"
#include "DKCharacterSet.h"


struct DKCharacterSetEntry
{
    int operand;
    DKCharacterRange range;
};

struct DKCharacterSet
{
    DKObject _obj;
    
    DKGenericArray entries;
};


static DKObjectRef DKCharacterSetInit( DKObjectRef _untyped_self );
static void DKCharacterSetFinalize( DKObjectRef _untyped_self );


DKThreadSafeClassInit( DKCharacterSetClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKCharacterSet" ), DKObjectClass(), sizeof(struct DKCharacterSet), 0, DKCharacterSetInit, DKCharacterSetFinalize );

    return cls;
}


///
//  DKPairInit()
//
static DKObjectRef DKCharacterSetInit( DKObjectRef _untyped_self )
{
    DKCharacterSetRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
        DKGenericArrayInit( &_self->entries, sizeof(struct DKCharacterSetEntry) );
    }
    
    return _self;
}


///
//  DKPairFinalize()
//
static void DKCharacterSetFinalize( DKObjectRef _untyped_self )
{
    DKCharacterSetRef _self = _untyped_self;
    
    DKGenericArrayFinalize( &_self->entries );
}


///
//  DKCharacterSetAddRange()
//
void DKCharacterSetIncludeCharactersInRange( DKCharacterSetRef _self, DKCharacterRange range )
{
    if( _self )
    {
        struct DKCharacterSetEntry entry;
        entry.operand = 1;
        entry.range = range;
        
        DKGenericArrayAppendElements( &_self->entries, &entry, 1 );
    }
}


///
//  DKCharacterSetRemoveRange()
//
void DKCharacterSetExcludeCharactersInRange( DKCharacterSetRef _self, DKCharacterRange range )
{
    if( _self )
    {
        struct DKCharacterSetEntry entry;
        entry.operand = -1;
        entry.range = range;
        
        DKGenericArrayAppendElements( &_self->entries, &entry, 1 );
    }
}


///
//  DKCharacterSetExcludeUnicodeNonCharacters()
//
void DKCharacterSetExcludeUnicodeNonCharacters( DKCharacterSetRef _self )
{
    DKCharacterSetExcludeCharactersInRange( _self, DKCharacterRangeUnicodeSurrogates );
    DKCharacterSetExcludeCharactersInRange( _self, DKCharacterRangeUnicodePrivateUseArea );
    DKCharacterSetExcludeCharactersInRange( _self, DKCharacterRangeUnicodePrivateUseSuplA );
    DKCharacterSetExcludeCharactersInRange( _self, DKCharacterRangeUnicodePrivateUseSuplB );
}


///
//  DKCharacterSetContainsCharacter()
//
bool DKCharacterSetContainsCharacter( DKCharacterSetRef _self, DKChar32 ch )
{
    if( _self && (ch >= 0) )
    {
        int inside = 0;
        
        DKIndex count = DKGenericArrayGetLength( &_self->entries );
        
        for( DKIndex i = 0; i < count; i++ )
        {
            const struct DKCharacterSetEntry * entry = DKGenericArrayGetPointerToElementAtIndex( &_self->entries, i );
            
            if( (ch >= entry->range.first) && (ch <= entry->range.last) )
                inside += entry->operand;
        }
        
        return inside > 0;
    }

    return false;
}




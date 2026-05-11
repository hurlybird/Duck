/*****************************************************************************************

  DKCharacterSet.c

  Copyright (c) 2026 Derek W. Nylen

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




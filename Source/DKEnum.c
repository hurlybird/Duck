/*****************************************************************************************

  DKEnum.h

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

#include "DKEnum.h"
#include "DKGenericHashTable.h"
#include "DKComparison.h"
#include "DKCopying.h"
#include "DKString.h"


struct DKEnumEntry
{
    DKObjectRef str;
    int value;
};

struct DKEnum
{
    DKObject _obj;
    
    DKGenericHashTable table;
};


static DKObjectRef DKEnumInitialize( DKObjectRef _untyped_self );
static void DKEnumFinalize( DKObjectRef _self );


///
//  DKEnumClass()
//
DKThreadSafeClassInit( DKEnumClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKEnum" ), DKObjectClass(), sizeof(struct DKEnum), DKImmutableInstances, DKEnumInitialize, DKEnumFinalize );
    
    return cls;
}




// DKGenericHashTable Callbacks ==========================================================

static DKRowStatus RowStatus( const void * _row )
{
    const struct DKEnumEntry * row = _row;
    return (DKRowStatus)(row->str);
}

static DKHashCode RowHash( const void * _row )
{
    const struct DKEnumEntry * row = _row;
    return DKHash( row->str );
}

static bool RowEqual( const void * _row1, const void * _row2 )
{
    const struct DKEnumEntry * row1 = _row1;
    const struct DKEnumEntry * row2 = _row2;

    return DKEqual( row1->str, row2->str );
}

static void RowInit( void * _row )
{
    struct DKEnumEntry * row = _row;
    
    row->str = DKRowStatusEmpty;
    row->value = 0;
}

static void RowUpdate( void * _row, const void * _src )
{
    struct DKEnumEntry * row = _row;
    const struct DKEnumEntry * src = _src;
    
    if( (row->str == NULL) || DKRowIsSentinel( row->str ) )
    {
        row->str = DKRetain( src->str );
    }

    row->value = src->value;
}

static void RowDelete( void * _row )
{
    struct DKEnumEntry * row = _row;
    
    DKRelease( row->str );
    
    row->str = DKRowStatusDeleted;
    row->value = 0;
}




// Interface =============================================================================

///
//  DKEnumInitialize()
//
static DKObjectRef DKEnumInitialize( DKObjectRef _untyped_self )
{
    DKEnumRef _self = DKSuperInit( _untyped_self, DKObjectClass() );

    if( _self )
    {
        DKGenericHashTableCallbacks callbacks =
        {
            RowStatus,
            RowHash,
            RowEqual,
            RowInit,
            RowUpdate,
            RowDelete
        };

        DKGenericHashTableInit( &_self->table, sizeof(struct DKEnumEntry), &callbacks );
    }
    
    return _self;
}


///
//  DKEnumFinalize()
//
static void DKEnumFinalize( DKObjectRef _untyped_self )
{
    DKEnumRef _self = _untyped_self;
    
    DKGenericHashTableFinalize( &_self->table );
}


///
//  DKEnumInitWithCStringsAndValues()
//
DKObjectRef DKEnumInitWithCStringsAndValues( DKObjectRef _untyped_self, ... )
{
    DKEnumRef _self = DKInit( _untyped_self );
        
    if( _self )
    {
        va_list arg_ptr;
        va_start( arg_ptr, _untyped_self );

        const char * str;

        while( (str = va_arg( arg_ptr, const char * ) ) != NULL )
        {
            struct DKEnumEntry row;
            row.str = __DKStringGetConstantString( str, true );
            row.value = va_arg( arg_ptr, int );
    
            DKGenericHashTableInsert( &_self->table, &row, DKInsertAlways );
            
            DKRelease( row.str );
        }

        va_end( arg_ptr );
    }
    
    return _self;
}


///
//  DKEnumFromString()
//
int32_t DKEnumFromString( DKEnumRef _self, DKStringRef str )
{
    struct DKEnumEntry findRow;
    findRow.str = str;
    findRow.value = 0;
    
    const struct DKEnumEntry * row = DKGenericHashTableFind( &_self->table, &findRow );

    if( row )
        return row->value;

    return 0;
}


///
//  DKStringFromEnum()
//
DKStringRef DKStringFromEnum( DKEnumRef _self, int value )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKEnumClass() );

        for( DKIndex i = 0; i < DKGenericHashTableGetRowCount( &_self->table ); ++i )
        {
            const struct DKEnumEntry * row = DKGenericHashTableGetRow( &_self->table, i );
            DKRowStatus status = RowStatus( row );
            
            if( DKRowIsActive( status ) )
            {
                // Note: Enums are supposed to be shared objects, so we shouldn't need
                // to retain + autorlease the string before returning it
            
                if( row->value == value )
                    return row->str;
            }
        }
    }
    
    return NULL;
}



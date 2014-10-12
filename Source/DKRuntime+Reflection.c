/*****************************************************************************************

  DKRuntime+Reflection.c

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

#define DK_RUNTIME_PRIVATE 1

#include "DKRuntime.h"
#include "DKGenericHashTable.h"
#include "DKString.h"


struct NameDatabaseEntry
{
    const DKObject  _obj;
    DKStringRef     name;
    DKHashCode      hash;
};

#define NAME_DATABASE_DELETED_ENTRY ((void *)-1)


static DKGenericHashTable ClassNameDatabase;
static DKSpinLock ClassNameDatabaseSpinLock = DKSpinLockInit;

static DKGenericHashTable SelectorNameDatabase;
static DKSpinLock SelectorNameDatabaseSpinLock = DKSpinLockInit;



// GenericHashTable Callbacks ============================================================
static DKRowStatus NameDatabaseRowStatus( const void * _row )
{
    struct NameDatabaseEntry * const * row = _row;

    if( *row == NULL )
        return DKRowStatusEmpty;
    
    if( *row == NAME_DATABASE_DELETED_ENTRY )
        return DKRowStatusDeleted;
    
    return DKRowStatusActive;
}

static DKHashCode NameDatabaseRowHash( const void * _row )
{
    struct NameDatabaseEntry * const * row = _row;
    return (*row)->hash;
}

static bool NameDatabaseRowEqual( const void * _row1, const void * _row2 )
{
    struct NameDatabaseEntry * const * row1 = _row1;
    struct NameDatabaseEntry * const * row2 = _row2;

    if( (*row1)->hash != (*row2)->hash )
        return false;

    return DKStringEqualToString( (*row1)->name, (*row2)->name );
}

static void NameDatabaseRowInit( void * _row )
{
    struct NameDatabaseEntry ** row = _row;
    *row = NULL;
}

static void NameDatabaseRowUpdate( void * _row, const void * _src )
{
    struct NameDatabaseEntry ** row = _row;
    struct NameDatabaseEntry * const * src = _src;
    *row = *src;
}

static void NameDatabaseRowDelete( void * _row )
{
    struct NameDatabaseEntry ** row = _row;
    *row = NAME_DATABASE_DELETED_ENTRY;
}




// Name Database =========================================================================

///
//  DKNameDatabaseInit()
//
void DKNameDatabaseInit( void )
{
    DKGenericHashTableCallbacks nameDatabaseCallbacks =
    {
        NameDatabaseRowStatus,
        NameDatabaseRowHash,
        NameDatabaseRowEqual,
        NameDatabaseRowInit,
        NameDatabaseRowUpdate,
        NameDatabaseRowDelete
    };

    DKGenericHashTableInit( &ClassNameDatabase, sizeof(DKObjectRef), &nameDatabaseCallbacks );
    DKGenericHashTableInit( &SelectorNameDatabase, sizeof(DKObjectRef), &nameDatabaseCallbacks );
}


///
//  DKNameDatabaseInsertClass()
//
void DKNameDatabaseInsertClass( DKClassRef _class )
{
    if( _class->name == NULL )
        return;
    
    DKSpinLockLock( &ClassNameDatabaseSpinLock );

    const DKClassRef * existing = DKGenericHashTableFind( &ClassNameDatabase, &_class );
    
    if( existing == NULL )
        DKGenericHashTableInsert( &ClassNameDatabase, &_class, DKInsertIfNotFound );
    
    DKSpinLockUnlock( &ClassNameDatabaseSpinLock );
    
    if( (existing != NULL) && (*existing != _class) )
    {
        DKError( "DKRuntime: A class named '%s' already exists.",
            DKStringGetCStringPtr( _class->name ) );
    }
}


///
//  DKNameDatabaseRemoveClass()
//
void DKNameDatabaseRemoveClass( DKClassRef _class )
{
    DKSpinLockLock( &ClassNameDatabaseSpinLock );
    DKGenericHashTableRemove( &ClassNameDatabase, &_class );
    DKSpinLockUnlock( &ClassNameDatabaseSpinLock );
}


///
//  DKNameDatabaseInsertSelector()
//
void DKNameDatabaseInsertSelector( DKSEL sel )
{
    if( sel->name == NULL )
        return;

    DKSpinLockLock( &SelectorNameDatabaseSpinLock );

    const DKSEL * existing = DKGenericHashTableFind( &SelectorNameDatabase, &sel );
    
    if( existing == NULL )
        DKGenericHashTableInsert( &SelectorNameDatabase, &sel, DKInsertIfNotFound );
    
    DKSpinLockUnlock( &SelectorNameDatabaseSpinLock );
    
    if( (existing != NULL) && (*existing != sel) )
    {
        DKError( "DKRuntime: A selector named '%s' already exists.",
            DKStringGetCStringPtr( sel->name ) );
    }
}


///
//  DKNameDatabaseRemoveSelector()
//
void DKNameDatabaseRemoveSelector( DKSEL sel )
{
    DKSpinLockLock( &SelectorNameDatabaseSpinLock );
    DKGenericHashTableRemove( &SelectorNameDatabase, &sel );
    DKSpinLockUnlock( &SelectorNameDatabaseSpinLock );
}




// Public Interfaces =====================================================================


///
//  DKGetSelf()
//
DKObjectRef DKGetSelf( DKObjectRef _self )
{
    return _self;
}


///
//  DKIsMutable()
//
bool DKIsMutable( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        return (obj->isa->options & DKImmutableInstances) == 0;
    }
    
    return false;
}


///
//  DKGetClass()
//
DKClassRef DKGetClass( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        return obj->isa;
    }
    
    return NULL;
}


///
//  DKGetClassName()
//
DKStringRef DKGetClassName( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        if( (cls == DKClassClass()) || (cls == DKRootClass()) )
            cls = _self;
        
        return cls->name;
    }
    
    return DKSTR( "null" );
}


///
//  DKGetSuperclass()
//
DKClassRef DKGetSuperclass( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        return obj->isa->superclass;
    }
    
    return NULL;
}


///
//  DKIsMemberOfClass()
//
bool DKIsMemberOfClass( DKObjectRef _self, DKClassRef _class )
{
    if( _self )
    {
        const DKObject * obj = _self;
        return obj->isa == _class;
    }
    
    return false;
}


///
//  DKIsKindOfClass()
//
bool DKIsKindOfClass( DKObjectRef _self, DKClassRef _class )
{
    if( _self )
    {
        const DKObject * obj = _self;
        
        for( DKClassRef cls = obj->isa; cls != NULL; cls = cls->superclass )
        {
            if( cls == _class )
                return true;
        }
    }
    
    return false;
}


///
//  DKIsSubclass()
//
bool DKIsSubclass( DKClassRef _class, DKClassRef otherClass )
{
    if( _class )
    {
        for( DKClassRef cls = _class; cls != NULL; cls = cls->superclass )
        {
            if( cls == otherClass )
                return true;
        }
    }
    
    return false;
}


///
//  DKStringFromClass()
//
DKStringRef DKStringFromClass( DKClassRef _class )
{
    if( _class )
        return _class->name;
    
    return DKSTR( "null" );
}


///
//  DKClassFromString()
//
DKClassRef DKClassFromString( DKStringRef name )
{
    if( name )
    {
        struct NameDatabaseEntry _key;
        _key.name = name;
        _key.hash = DKStringHash( name );
        
        struct NameDatabaseEntry * key = &_key;

        DKSpinLockLock( &ClassNameDatabaseSpinLock );
        const DKClassRef * cls = DKGenericHashTableFind( &ClassNameDatabase, &key );
        DKSpinLockUnlock( &ClassNameDatabaseSpinLock );
        
        if( cls )
            return *cls;
    }
    
    return NULL;
}


///
//  DKStringFromSelector()
//
DKStringRef DKStringFromSelector( DKSEL sel )
{
    if( sel )
        return sel->name;
    
    return DKSTR( "null" );
}


///
//  DKSelectorFromString()
//
DKSEL DKSelectorFromString( DKStringRef name )
{
    if( name )
    {
        struct NameDatabaseEntry _key;
        _key.name = name;
        _key.hash = DKStringHash( name );
        
        struct NameDatabaseEntry * key = &_key;

        DKSpinLockLock( &SelectorNameDatabaseSpinLock );
        const DKSEL * sel = DKGenericHashTableFind( &SelectorNameDatabase, &key );
        DKSpinLockUnlock( &SelectorNameDatabaseSpinLock );
        
        if( sel )
            return *sel;
    }
    
    return NULL;
}






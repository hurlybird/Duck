// =======================================================================================
//
// DKRuntime+Reflection.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#define DK_RUNTIME_PRIVATE 1

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKGenericArray.h"
#include "DKGenericHashTable.h"
#include "DKRuntime.h"
#include "DKString.h"


struct NameDatabaseEntry
{
    const DKObject  _obj;
    DKStringRef     name;
};


static DKGenericHashTable ClassNameDatabase;
static DKSpinlock ClassNameDatabaseSpinLock = DKSpinlockInit;

static DKGenericHashTable SelectorNameDatabase;
static DKSpinlock SelectorNameDatabaseSpinLock = DKSpinlockInit;



// GenericHashTable Callbacks ============================================================
static DKRowStatus NameDatabaseRowStatus( const void * _row, void * not_used )
{
    struct NameDatabaseEntry ** row = (void *)_row;
    return (DKRowStatus)(*row);
}

static DKHashCode NameDatabaseRowHash( const void * _row, void * not_used )
{
    struct NameDatabaseEntry ** row = (void *)_row;
    return DKStringHash( (*row)->name );
}

static bool NameDatabaseRowEqual( const void * _row1, const void * _row2, void * not_used )
{
    struct NameDatabaseEntry ** row1 = (void *)_row1;
    struct NameDatabaseEntry ** row2 = (void *)_row2;

    return DKStringEqualToString( (*row1)->name, (*row2)->name );
}

static void NameDatabaseRowInit( void * _row, void * not_used )
{
    struct NameDatabaseEntry ** row = _row;
    *row = DKRowStatusEmpty;
}

static void NameDatabaseRowUpdate( void * _row, const void * _src, void * not_used )
{
    struct NameDatabaseEntry ** row = _row;
    struct NameDatabaseEntry ** src = (void *)_src;
    *row = *src;
}

static void NameDatabaseRowDelete( void * _row, void * not_used )
{
    struct NameDatabaseEntry ** row = _row;
    *row = DKRowStatusDeleted;
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

    DKGenericHashTableInit( &ClassNameDatabase, sizeof(DKObjectRef), &nameDatabaseCallbacks, NULL );
    DKGenericHashTableInit( &SelectorNameDatabase, sizeof(DKObjectRef), &nameDatabaseCallbacks, NULL );
}


///
//  DKNameDatabaseInsertClass()
//
void DKNameDatabaseInsertClass( DKClassRef _class )
{
    if( _class->name == NULL )
        return;
    
    DKSpinlockLock( &ClassNameDatabaseSpinLock );

    bool inserted = DKGenericHashTableInsert( &ClassNameDatabase, &_class, DKInsertIfNotFound );
    
    DKSpinlockUnlock( &ClassNameDatabaseSpinLock );
    
    if( !inserted )
    {
        DKFatalError( "DKRuntime: A class named '%@' already exists.", _class->name );
    }
}


///
//  DKNameDatabaseRemoveClass()
//
void DKNameDatabaseRemoveClass( DKClassRef _class )
{
    DKSpinlockLock( &ClassNameDatabaseSpinLock );
    DKGenericHashTableRemove( &ClassNameDatabase, &_class );
    DKSpinlockUnlock( &ClassNameDatabaseSpinLock );
}


///
//  DKNameDatabaseInsertSelector()
//
void DKNameDatabaseInsertSelector( DKSEL sel )
{
    if( sel->name == NULL )
        return;

    DKSpinlockLock( &SelectorNameDatabaseSpinLock );

    bool inserted = DKGenericHashTableInsert( &SelectorNameDatabase, &sel, DKInsertIfNotFound );
    
    DKSpinlockUnlock( &SelectorNameDatabaseSpinLock );
    
    if( !inserted )
    {
        DKFatalError( "DKRuntime: A selector named '%@' already exists.", sel->name );
    }
}


///
//  DKNameDatabaseRemoveSelector()
//
void DKNameDatabaseRemoveSelector( DKSEL sel )
{
    DKSpinlockLock( &SelectorNameDatabaseSpinLock );
    DKGenericHashTableRemove( &SelectorNameDatabase, &sel );
    DKSpinlockUnlock( &SelectorNameDatabaseSpinLock );
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
//  DKIsClass()
//
bool DKIsClass( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        // If the source object object is a class, bail out
        if( (cls == DKClassClass()) || (cls == DKRootClass()) )
            return true;
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
DKClassRef DKClassFromString( DKStringRef className )
{
    DKClassRef cls = NULL;
    
    if( className )
    {
        struct NameDatabaseEntry _key = { { NULL, 0, 0 }, className };
        struct NameDatabaseEntry * key = &_key;

        DKSpinlockLock( &ClassNameDatabaseSpinLock );
        
        DKClassRef * entry = (DKClassRef *)DKGenericHashTableFind( &ClassNameDatabase, &key );
        
        if( entry )
            cls = *entry;
        
        DKSpinlockUnlock( &ClassNameDatabaseSpinLock );
    }
    
    return cls;
}


///
//  DKClassFromCString()
//
DKClassRef DKClassFromCString( const char * className )
{
    if( className )
    {
        DKStringRef _className = DKStringInitWithCStringNoCopy( DKAlloc( DKStringClass() ), className );
        DKClassRef _class = DKClassFromString( _className );
        DKRelease( _className );
        
        return _class;
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
    DKSEL sel = NULL;
    
    if( name )
    {
        struct NameDatabaseEntry _key = { { NULL, 0, 0 }, name };
        struct NameDatabaseEntry * key = &_key;

        DKSpinlockLock( &SelectorNameDatabaseSpinLock );
        
        DKSEL * entry = (DKSEL *)DKGenericHashTableFind( &SelectorNameDatabase, &key );
        
        if( entry )
            sel = *entry;
        
        DKSpinlockUnlock( &SelectorNameDatabaseSpinLock );
    }
    
    return sel;
}






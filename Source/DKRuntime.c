/*****************************************************************************************

  DKRuntime.c

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

#include "DKPlatform.h"
#include "DKGenericArray.h"
#include "DKGenericHashTable.h"
#include "DKRuntime.h"
#include "DKString.h"
#include "DKHashTable.h"
#include "DKArray.h"
#include "DKProperty.h"
#include "DKStream.h"
#include "DKEgg.h"


static void DKRuntimeInit( void );

static void DKClassFinalize( DKObjectRef _self );
static void DKSelectorFinalize( DKObjectRef _self );
static void DKInterfaceFinalize( DKObjectRef _self );

static DKInterface * DKLookupInterface( const struct DKClass * cls, DKSEL sel );



// Internal Types ========================================================================

// Objects are at least 16 bytes long so there must exist a location in memory
// that is 16-byte aligned and inside the object. Given that, we can generate a
// hash code from the object pointer that strips out the uninteresting lower
// bits to make things a bit more random. This is particularly important in a
// hash table that uses hash % prime to derive an internal hash code.
#define ObjectUniqueHash( obj )     ((((uintptr_t)obj) + 15) >> 4)

// Get a dynamic cache index for a selector
#define GetDynamicCacheline( sel )  (int)((ObjectUniqueHash(sel) & (DKDynamicCacheSize-1)) + DKStaticCacheSize)



// DKClass
struct DKClass
{
    const DKObject  _obj;

    // The name database requires that the name and hash fields of DKClass and DKSEL are
    // in the same position in the structure (i.e. right after the object header).
    DKStringRef     name;
    DKHashCode      hash;
    
    DKClassRef      superclass;
    size_t          structSize;
    DKClassOptions  options;
    
    DKInitMethod    init;
    DKFinalizeMethod finalize;

    DKSpinLock      interfacesLock;
    DKSpinLock      propertiesLock;

    DKInterface *   cache[DKStaticCacheSize + DKDynamicCacheSize];
    
    // Classes usually have fewer than 10 interfaces and selectors are compared by
    // pointer value (not name). It's hard to say whether a linear search on a small
    // array is faster or slower than a hash table lookup. The search result is also
    // cached, further mitigating any performance problems.
    DKGenericArray  interfaces;
    
    DKMutableHashTableRef properties;
};


// DKWeak
struct DKWeak
{
    DKObject        _obj;
    DKSpinLock      lock;
    DKObjectRef     target;
};




// Root Classes ==========================================================================
static struct DKClass __DKMetaClass__;
static struct DKClass __DKClassClass__;
static struct DKClass __DKSelectorClass__;
static struct DKClass __DKInterfaceClass__;
static struct DKClass __DKMsgHandlerClass__;
static struct DKClass __DKWeakClass__;
static struct DKClass __DKObjectClass__;


DKClassRef DKClassClass( void )
{
    DKRuntimeInit();
    return &__DKClassClass__;
}

DKClassRef DKSelectorClass( void )
{
    DKRuntimeInit();
    return &__DKSelectorClass__;
}

DKClassRef DKInterfaceClass( void )
{
    DKRuntimeInit();
    return &__DKInterfaceClass__;
}

DKClassRef DKMsgHandlerClass( void )
{
    DKRuntimeInit();
    return &__DKMsgHandlerClass__;
}

DKClassRef DKWeakClass( void )
{
    DKRuntimeInit();
    return &__DKWeakClass__;
}

DKClassRef DKObjectClass( void )
{
    DKRuntimeInit();
    return &__DKObjectClass__;
}




// Selectors Required for the Root Classes ===============================================
#define DKStaticSelectorInit( name )                                                    \
    static struct _DKSEL DKSelector_ ## name ##_StaticObject =                          \
    {                                                                                   \
        DKStaticObject( &__DKSelectorClass__ ),                                         \
        NULL,                                                                           \
        DKDynamicCache                                                                  \
    };                                                                                  \
                                                                                        \
    DKSEL DKSelector_ ## name( void )                                                   \
    {                                                                                   \
        return &DKSelector_ ## name ##_StaticObject;                                    \
    }


#define DKStaticFastSelectorInit( name )                                                \
    static struct _DKSEL DKSelector_ ## name ##_StaticObject =                          \
    {                                                                                   \
        DKStaticObject( &__DKSelectorClass__ ),                                         \
        NULL,                                                                           \
        DKStaticCache_ ## name                                                          \
    };                                                                                  \
                                                                                        \
    DKSEL DKSelector_ ## name( void )                                                   \
    {                                                                                   \
        return &DKSelector_ ## name ##_StaticObject;                                    \
    }


#define DKStaticInterfaceObject( sel )                                                  \
    {                                                                                   \
        DKStaticObject( &__DKInterfaceClass__ ),                                        \
        sel                                                                             \
    }


DKStaticSelectorInit( Allocation );
DKStaticFastSelectorInit( Comparison );
DKStaticSelectorInit( Copying );
DKStaticSelectorInit( Description );
DKStaticSelectorInit( Stream );
DKStaticSelectorInit( Egg );




// Error Handling Interfaces =============================================================
#define DK_MAX_INTERFACE_SIZE   32

DKDeclareInterfaceSelector( InterfaceNotFound );
DKStaticSelectorInit( InterfaceNotFound );

DKDeclareMessageSelector( MsgHandlerNotFound );
DKStaticSelectorInit( MsgHandlerNotFound );

typedef void (*DKInterfaceNotFoundFunction)( DKObjectRef _self );

struct DKInterfaceNotFoundInterface
{
    DKInterface _interface;
    DKInterfaceNotFoundFunction func[DK_MAX_INTERFACE_SIZE];
};

static void DKInterfaceNotFoundCallback( DKObjectRef _self )
{
    // Note: '_self' is for debugging only and may not be valid. Most interface functions
    // take an object as a first arguement, but it's not actually required.

    // The only time this code is ever likely to be called is when calling an interface
    // function on a NULL object. We don't have the Objective-C luxury of tweaking the
    // call stack and return value in assembly for such cases.

    DKFatalError( "DKRuntime: Invalid interface call.\n" );
}

static intptr_t DKMsgHandlerNotFoundCallback( DKObjectRef _self, DKSEL sel )
{
    // This handles silently failing when sending messages to NULL objects.
    return 0;
}

DKThreadSafeSharedObjectInit( DKInterfaceNotFound, DKInterfaceRef )
{
    struct DKInterfaceNotFoundInterface * interface = DKAllocInterface( DKSelector(InterfaceNotFound), sizeof(struct DKInterfaceNotFoundInterface) );

    for( int i = 0; i < DK_MAX_INTERFACE_SIZE; i++ )
        interface->func[i] = DKInterfaceNotFoundCallback;
    
    return interface;
}

DKThreadSafeSharedObjectInit( DKMsgHandlerNotFound, DKMsgHandlerRef )
{
    struct DKMsgHandler * msgHandler = DKAllocInterface( DKSelector(MsgHandlerNotFound), sizeof(struct DKMsgHandler) );

    msgHandler->func = DKMsgHandlerNotFoundCallback;

    return msgHandler;
}




// Default Interfaces ====================================================================

// DefaultComparison ---------------------------------------------------------------------
static struct DKComparisonInterface DKDefaultComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    DKPointerEqual,
    DKPointerEqual,
    DKPointerCompare,
    DKPointerHash
};

DKInterfaceRef DKDefaultComparison( void )
{
    return &DKDefaultComparison_StaticObject;
}

bool DKPointerEqual( DKObjectRef _self, DKObjectRef other )
{
    return _self == other;
}

int DKPointerCompare( DKObjectRef _self, DKObjectRef other )
{
    if( _self < other )
        return 1;
    
    if( _self > other )
        return -1;
    
    return 0;
}

DKHashCode DKPointerHash( DKObjectRef _self )
{
    return ObjectUniqueHash( _self );
}


// DefaultCopying ------------------------------------------------------------------------
static struct DKCopyingInterface DKDefaultCopying_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Copying_StaticObject ),
    DKRetain,
    (DKMutableCopyMethod)DKRetain
};

DKInterfaceRef DKDefaultCopying( void )
{
    return &DKDefaultCopying_StaticObject;
}


// DefaultDescription --------------------------------------------------------------------
static struct DKDescriptionInterface DKDefaultDescription_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Description_StaticObject ),
    DKDefaultCopyDescription
};

DKInterfaceRef DKDefaultDescription( void )
{
    return &DKDefaultDescription_StaticObject;
}

DKStringRef DKDefaultCopyDescription( DKObjectRef _self )
{
    return DKCopy( DKGetClassName( _self ) );
}




// Root Class Interfaces =================================================================

// Selector Comparison -------------------------------------------------------------------
#define DKFastSelectorEqual( a, b )     (a == b)
#define DKSelectorEqual( a, b )         DKPointerEqual( a, b );
#define DKSelectorCompare( a, b )       DKPointerCompare( a, b );
#define DKSelectorHash( _self )         DKPointerHash( _self )

static struct DKComparisonInterface DKSelectorComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    DKPointerEqual,
    DKPointerEqual,
    DKPointerCompare,
    DKPointerHash
};

static DKInterfaceRef DKSelectorComparison( void )
{
    return &DKSelectorComparison_StaticObject;
}


// Interface Comparison ------------------------------------------------------------------
static int DKInterfaceEqual( DKInterface * a, DKInterface * b )
{
    return DKSelectorEqual( a->sel, b->sel );
}

static int DKInterfaceCompare( DKInterface * a, DKInterface * b )
{
    return DKSelectorCompare( a->sel, b->sel );
}

static DKHashCode DKInterfaceHash( DKInterface * a )
{
    return DKSelectorHash( a->sel );
}

static struct DKComparisonInterface DKInterfaceComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    (DKEqualityMethod)DKInterfaceEqual,
    (DKEqualityMethod)DKInterfaceEqual,
    (DKCompareMethod)DKInterfaceCompare,
    (DKHashMethod)DKInterfaceHash
};

static DKInterfaceRef DKInterfaceComparison( void )
{
    return &DKInterfaceComparison_StaticObject;
}




// Class and Selector Name Databases =====================================================

struct NameDatabaseEntry
{
    const DKObject  _obj;
    DKStringRef     name;
    DKHashCode      hash;
};

static DKGenericHashTable ClassNameDatabase;
static DKSpinLock ClassNameDatabaseSpinLock = DKSpinLockInit;

static DKGenericHashTable SelectorNameDatabase;
static DKSpinLock SelectorNameDatabaseSpinLock = DKSpinLockInit;

#define NAME_DATABASE_DELETED_ENTRY ((void *)-1)

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


///
//  NameDatabaseInsertClass()
//
static void NameDatabaseInsertClass( DKClassRef _class )
{
    DKSpinLockLock( &ClassNameDatabaseSpinLock );
    DKGenericHashTableInsert( &ClassNameDatabase, &_class, DKInsertIfNotFound );
    DKSpinLockUnlock( &ClassNameDatabaseSpinLock );
}


///
//  NameDatabaseRemoveClass()
//
static void NameDatabaseRemoveClass( DKClassRef _class )
{
    DKSpinLockLock( &ClassNameDatabaseSpinLock );
    DKGenericHashTableRemove( &ClassNameDatabase, &_class );
    DKSpinLockUnlock( &ClassNameDatabaseSpinLock );
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
//  NameDatabaseInsertSelector()
//
static void NameDatabaseInsertSelector( DKSEL sel )
{
    DKSpinLockLock( &SelectorNameDatabaseSpinLock );
    DKGenericHashTableInsert( &ClassNameDatabase, &sel, DKInsertIfNotFound );
    DKSpinLockUnlock( &SelectorNameDatabaseSpinLock );
}


///
//  NameDatabaseRemoveSelector()
//
static void NameDatabaseRemoveSelector( DKSEL sel )
{
    DKSpinLockLock( &SelectorNameDatabaseSpinLock );
    DKGenericHashTableRemove( &ClassNameDatabase, &sel );
    DKSpinLockUnlock( &SelectorNameDatabaseSpinLock );
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




// Runtime Init ==========================================================================
static DKSpinLock DKRuntimeInitLock = DKSpinLockInit;
static bool DKRuntimeInitialized = false;


///
//  InitRootClass()
//
static void InitRootClass( struct DKClass * cls, struct DKClass * superclass, size_t structSize,
    DKClassOptions options, DKInitMethod init, DKFinalizeMethod finalize )
{
    memset( cls, 0, sizeof(struct DKClass) );
    
    DKObject * obj = (DKObject *)cls;
    obj->isa = &__DKMetaClass__;
    obj->refcount = 1;

    cls->name = NULL;
    cls->superclass = DKRetain( superclass );
    cls->structSize = structSize;
    cls->options = options;
    cls->init = init;
    cls->finalize = finalize;
    cls->interfacesLock = DKSpinLockInit;
    cls->propertiesLock = DKSpinLockInit;
    
    DKGenericArrayInit( &cls->interfaces, sizeof(DKObjectRef) );
}


///
//  InstallRootClassInterface()
//
static void InstallRootClassInterface( struct DKClass * _class, DKInterfaceRef interface )
{
    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.
    DKGenericArrayAppendElements( &_class->interfaces, &interface, 1 );
}


///
//  SetRootClassName()
//
static void SetRootClassName( struct DKClass * _class, DKStringRef name )
{
    _class->name = DKCopy( name );
    _class->hash = DKStringHash( name );
    
    NameDatabaseInsertClass( _class );
}


///
//  SetStaticSelectorName()
//
static void SetStaticSelectorName( struct _DKSEL * sel, DKStringRef name )
{
    sel->name = DKCopy( name );
    sel->hash = DKStringHash( name );
    
    NameDatabaseInsertSelector( sel );
}


///
//  DKRuntimeInit()
//
static void DKRuntimeInit( void )
{
    DKSpinLockLock( &DKRuntimeInitLock );
    
    if( !DKRuntimeInitialized )
    {
        DKRuntimeInitialized = true;

        InitRootClass( &__DKMetaClass__,       NULL,                  sizeof(struct DKClass), DKAbstractBaseClass | DKDisableReferenceCounting, NULL, DKClassFinalize );
        InitRootClass( &__DKClassClass__,      NULL,                  sizeof(struct DKClass), 0, NULL, DKClassFinalize );
        InitRootClass( &__DKSelectorClass__,   NULL,                  sizeof(struct _DKSEL),  0, NULL, DKSelectorFinalize );
        InitRootClass( &__DKInterfaceClass__,  NULL,                  sizeof(DKInterface),    0, NULL, DKInterfaceFinalize );
        InitRootClass( &__DKMsgHandlerClass__, &__DKInterfaceClass__, sizeof(DKMsgHandler),   0, NULL, NULL );
        InitRootClass( &__DKWeakClass__,       NULL,                  sizeof(struct DKWeak),  0, NULL, NULL );
        InitRootClass( &__DKObjectClass__,     NULL,                  sizeof(DKObject),       0, NULL, NULL );
        
        InstallRootClassInterface( &__DKMetaClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKMetaClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKClassClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKClassClass__, DKDefaultCopying() );
        InstallRootClassInterface( &__DKClassClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKSelectorClass__, DKSelectorComparison() );
        InstallRootClassInterface( &__DKSelectorClass__, DKDefaultCopying() );
        InstallRootClassInterface( &__DKSelectorClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKInterfaceClass__, DKInterfaceComparison() );
        InstallRootClassInterface( &__DKInterfaceClass__, DKDefaultCopying() );
        InstallRootClassInterface( &__DKInterfaceClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKMsgHandlerClass__, DKInterfaceComparison() );
        InstallRootClassInterface( &__DKMsgHandlerClass__, DKDefaultCopying() );
        InstallRootClassInterface( &__DKMsgHandlerClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKWeakClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKWeakClass__, DKDefaultCopying() );
        InstallRootClassInterface( &__DKWeakClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKObjectClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKObjectClass__, DKDefaultCopying() );
        InstallRootClassInterface( &__DKObjectClass__, DKDefaultDescription() );

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

        DKSpinLockUnlock( &DKRuntimeInitLock );
        
        // Initialize the base class names now that constant strings are available.
        SetRootClassName( &__DKMetaClass__, DKSTR( "DKMetaClass" ) );
        SetRootClassName( &__DKClassClass__, DKSTR( "DKClass" ) );
        SetRootClassName( &__DKSelectorClass__, DKSTR( "DKSelector" ) );
        SetRootClassName( &__DKInterfaceClass__, DKSTR( "DKInterface" ) );
        SetRootClassName( &__DKMsgHandlerClass__, DKSTR( "DKMsgHandler" ) );
        SetRootClassName( &__DKObjectClass__, DKSTR( "DKObject" ) );
        SetRootClassName( &__DKWeakClass__, DKSTR( "DKWeak" ) );

        SetStaticSelectorName( &DKSelector_InterfaceNotFound_StaticObject, DKSTR( "InterfaceNotFound" ) );
        SetStaticSelectorName( &DKSelector_MsgHandlerNotFound_StaticObject, DKSTR( "MsgHandlerNotFound" ) );
        SetStaticSelectorName( &DKSelector_Allocation_StaticObject, DKSTR( "Allocation" ) );
        SetStaticSelectorName( &DKSelector_Comparison_StaticObject, DKSTR( "Comparison" ) );
        SetStaticSelectorName( &DKSelector_Copying_StaticObject, DKSTR( "Copying" ) );
        SetStaticSelectorName( &DKSelector_Description_StaticObject, DKSTR( "Description" ) );
        SetStaticSelectorName( &DKSelector_Stream_StaticObject, DKSTR( "Stream" ) );
        SetStaticSelectorName( &DKSelector_Egg_StaticObject, DKSTR( "Egg" ) );
        
        SetStaticSelectorName( (struct _DKSEL *)DKStringClass(), DKSTR( "DKString" ) );
        SetStaticSelectorName( (struct _DKSEL *)DKConstantStringClass(), DKSTR( "DKConstantString" ) );
    }
    
    else
    {
        DKSpinLockUnlock( &DKRuntimeInitLock );
    }
}




// Alloc/Free Objects ====================================================================

///
//  DKAllocObject()
//
void * DKAllocObject( DKClassRef cls, size_t extraBytes )
{
    if( !cls )
    {
        DKError( "DKAllocObject: Specified class object is NULL.\n" );
        return NULL;
    }
    
    if( cls->structSize < sizeof(DKObject) )
    {
        DKFatalError( "DKAllocObject: Requested struct size is smaller than DKObject.\n" );
        return NULL;
    }
    
    if( (cls->options & DKAbstractBaseClass) != 0 )
    {
        DKFatalError( "DKAllocObject: Class '%s' is an abstract base class.\n", DKStringGetCStringPtr( cls->name ) );
        return NULL;
    }
    
    // Allocate the structure + extra bytes
    DKObject * obj = dk_malloc( cls->structSize + extraBytes );
    
    // Zero the structure bytes
    memset( obj, 0, cls->structSize );
    
    // Setup the object header
    obj->isa = DKRetain( cls );
    obj->weakref = NULL;
    obj->refcount = 1;
    
    return obj;
}


///
//  DKDeallocObject()
//
void DKDeallocObject( DKObjectRef _self )
{
    DKObject * obj = (DKObject *)_self;
    
    DKAssert( obj );
    DKAssert( obj->refcount == 0 );
    DKAssert( obj->weakref == NULL );

    DKClassRef cls = obj->isa;

    // Deallocate
    dk_free( obj );
    
    // Finally release the class object
    DKRelease( cls );
}




// Creating Classes ======================================================================

///
//  DKAllocClass()
//
DKClassRef DKAllocClass( DKStringRef name, DKClassRef superclass, size_t structSize,
    DKClassOptions options, DKInitMethod init, DKFinalizeMethod finalize )
{
    if( superclass && ((superclass->options & DKPreventSubclassing) != 0) )
    {
        DKFatalError( "DKAllocClass: Class '%s' does not allow subclasses.\n", DKStringGetCStringPtr( superclass->name ) );
        return NULL;
    }

    struct DKClass * cls = DKCreate( DKClassClass() );

    cls->name = DKCopy( name );
    cls->hash = DKStringHash( name );
    cls->superclass = DKRetain( superclass );
    cls->structSize = structSize;
    cls->options = options;

    cls->init = init;
    cls->finalize = finalize;

    cls->interfacesLock = DKSpinLockInit;
    cls->propertiesLock = DKSpinLockInit;

    memset( cls->cache, 0, sizeof(cls->cache) );
    
    DKGenericArrayInit( &cls->interfaces, sizeof(DKObjectRef) );
    
    NameDatabaseInsertClass( cls );
    
    return cls;
}


///
//  DKClassFinalize()
//
static void DKClassFinalize( DKObjectRef _self )
{
    struct DKClass * cls = (struct DKClass *)_self;
    
    DKAssert( cls->_obj.isa == &__DKClassClass__ );

    NameDatabaseRemoveClass( cls );
    
    // Note: The finalizer chain is still running at this point so make sure to set
    // the members to NULL to avoid accessing dangling pointers.
    
    DKRelease( cls->name );
    cls->name = NULL;
    
    DKRelease( cls->superclass );
    cls->superclass = NULL;

    // Release interfaces
    DKIndex count = cls->interfaces.length;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKInterface * interface = DKGenericArrayGetElementAtIndex( &cls->interfaces, i, DKInterface * );
        DKRelease( interface );
    }
    
    DKGenericArrayFinalize( &cls->interfaces );
    
    // Release properties
    DKRelease( cls->properties );
    cls->properties = NULL;
}


///
//  DKAllocSelector()
//
DKSEL DKAllocSelector( DKStringRef name )
{
    struct _DKSEL * sel = DKInit( DKAlloc( DKSelectorClass(), 0 ) );

    DKAssert( sel != NULL );

    sel->name = DKCopy( name );
    sel->cacheline = DKDynamicCache;

    NameDatabaseInsertSelector( sel );

    return sel;
}


///
//  DKSelectorFinalize()
//
static void DKSelectorFinalize( DKObjectRef _self )
{
    DKSEL sel = _self;

    NameDatabaseRemoveSelector( sel );

    DKRelease( sel->name );
}


///
//  DKAllocInterface()
//
void * DKAllocInterface( DKSEL sel, size_t structSize )
{
    if( sel )
    {
        size_t extraBytes = structSize - sizeof(DKInterface);
        
        // DKGetInterface returns a generic interface filled with DK_MAX_INTERFACE_SIZE
        // pointers to DKInterfaceNotFound. The size needs to be large enough to deal
        // with the largest interface or it'll cause an access error at run time.
        DKAssert( (extraBytes / sizeof(void *)) <= DK_MAX_INTERFACE_SIZE );
        
        DKInterface * interface = DKInit( DKAlloc( DKInterfaceClass(), extraBytes ) );
        DKAssert( interface != NULL );

        interface->sel = DKRetain( sel );
    
        // Init all the function pointers to NULL
        memset( interface + 1, 0, extraBytes );
    
        return interface;
    }
    
    return NULL;
}


///
//  DKInterfaceFinalize()
//
static void DKInterfaceFinalize( DKObjectRef _self )
{
    DKInterface * interface = (DKInterface *)_self;
    DKRelease( interface->sel );
}


///
//  DKInstallInterface()
//
void DKInstallInterface( DKClassRef _class, DKInterfaceRef _interface )
{
    DKAssertMemberOfClass( _class, DKClassClass() );
    DKAssertKindOfClass( _interface, DKInterfaceClass() );

    struct DKClass * cls = (struct DKClass *)_class;
    DKInterface * interface = (DKInterface *)_interface;

    // Retain the new interface
    DKRetain( interface );

    // Resolve the cache line from the selector
    int cacheline = interface->sel->cacheline;
    DKAssert( (cacheline >= 0) && (cacheline < DKStaticCacheSize) );

    if( cacheline == DKDynamicCache )
        cacheline = GetDynamicCacheline( interface->sel );
    
    DKAssert( (cacheline > 0) && (cacheline < (DKStaticCacheSize + DKDynamicCacheSize)) );
    
    // Lock while we make changes
    DKSpinLockLock( &cls->interfacesLock );
    
    // Invalidate the cache
    
    // *** WARNING ***
    // This doesn't invalidate the caches of any subclasses, so it's possible that
    // subclasses will still reference the old interface, or even crash if the old
    // interface is released (the cache doesn't maintain a reference).
    // *** WARNING ***
    
    cls->cache[cacheline] = NULL;

    // Replace the interface in the interface table
    DKIndex count = cls->interfaces.length;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKInterface * oldInterface = DKGenericArrayGetElementAtIndex( &cls->interfaces, i, DKInterface * );
        
        if( DKEqual( oldInterface->sel, interface->sel ) )
        {
            DKGenericArrayGetElementAtIndex( &cls->interfaces, i, DKInterface * ) = interface;

            DKSpinLockUnlock( &cls->interfacesLock );

            // Release the old interface after unlocking
            DKRelease( oldInterface );
            return;
        }
    }
    
    // Add the interface to the interface table
    DKGenericArrayAppendElements( &cls->interfaces, &interface, 1 );

    DKSpinLockUnlock( &cls->interfacesLock );
}


///
//  DKInstallMsgHandler()
//
void DKInstallMsgHandler( DKClassRef _class, DKSEL sel, DKMsgFunction func )
{
    struct DKMsgHandler * msgHandler = DKInit( DKAlloc( DKMsgHandlerClass(), sizeof(void *) ) );
    DKAssert( msgHandler != NULL );

    msgHandler->sel = DKRetain( sel );
    msgHandler->func = func;
    
    DKInstallInterface( _class, msgHandler );
    
    DKRelease( msgHandler );
}


///
//  DKInstallProperty()
//
void DKInstallProperty( DKClassRef _class, DKStringRef name, DKPropertyRef property )
{
    DKAssert( _class && property && name );
    
    struct DKClass * cls = (struct DKClass *)_class;
    
    DKSpinLockLock( &cls->propertiesLock );
    
    if( cls->properties == NULL )
        cls->properties = DKHashTableCreateMutable();
    
    DKHashTableInsertObject( cls->properties, name, property, DKInsertAlways );
    
    DKSpinLockUnlock( &cls->propertiesLock );
}




// Retrieving Interfaces, Message Handlers and Properties ================================

///
//  DKLookupInterface()
//
static DKInterface * DKLookupInterface( DKClassRef _class, DKSEL sel )
{
    DKAssert( (_class->_obj.isa == &__DKClassClass__) || (_class->_obj.isa == &__DKMetaClass__) );
    DKAssert( sel->_obj.isa == &__DKSelectorClass__ );

    struct DKClass * cls = (struct DKClass *)_class;

    // Get the static cache line from the selector
    int cacheline = sel->cacheline;
    DKAssert( (cacheline >= 0) && (cacheline < DKStaticCacheSize) );

    // We shoudn't need to acquire the spin lock while reading and writing to the cache
    // since the worst that can happen is doing an extra lookup after reading a stale
    // cache line.
    #define SPIN_LOCKED_CACHE_ACCESS 0

    // Lock while we lookup the interface
    #if SPIN_LOCKED_CACHE_ACCESS
    DKSpinLockLock( &cls->interfacesLock );
    #endif
    
    // First check the static cache (line 0 will always be NULL)
    DKInterface * interface = cls->cache[cacheline];
    
    if( interface )
    {
        DKAssert( DKFastSelectorEqual( interface->sel, sel ) );
    
        #if SPIN_LOCKED_CACHE_ACCESS
        DKSpinLockUnlock( &cls->interfacesLock );
        #endif
        
        return interface;
    }
    
    // Next check the dynamic cache
    if( cacheline == DKDynamicCache )
    {
        cacheline = GetDynamicCacheline( sel );
        DKAssert( (cacheline > 0) && (cacheline < (DKStaticCacheSize + DKDynamicCacheSize)) );
        
        interface = cls->cache[cacheline];
        
        if( interface && DKFastSelectorEqual( interface->sel, sel ) )
        {
            #if SPIN_LOCKED_CACHE_ACCESS
            DKSpinLockUnlock( &cls->interfacesLock );
            #endif
            
            return interface;
        }
    }

    // Search our interface table
    #if !SPIN_LOCKED_CACHE_ACCESS
    DKSpinLockLock( &cls->interfacesLock );
    #endif
    
    DKIndex count = cls->interfaces.length;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKInterface * interface = DKGenericArrayGetElementAtIndex( &cls->interfaces, i, DKInterface * );
        DKAssert( interface != NULL );
        
        if( DKFastSelectorEqual( interface->sel, sel ) )
        {
            // Update the cache
            cls->cache[cacheline] = interface;

            DKSpinLockUnlock( &cls->interfacesLock );
            return interface;
        }
    }

    // Lookup the interface in our superclasses
    DKSpinLockUnlock( &cls->interfacesLock );
    
    if( cls->superclass )
    {
        interface = DKLookupInterface( cls->superclass, sel );

        // Update the cache
        if( interface )
        {
            #if SPIN_LOCKED_CACHE_ACCESS
            DKSpinLockLock( &cls->interfacesLock );
            #endif
            
            cls->cache[cacheline] = interface;
            
            #if SPIN_LOCKED_CACHE_ACCESS
            DKSpinLockUnlock( &cls->interfacesLock );
            #endif
        }
    }

    return interface;
}


///
//  DKGetInterface()
//
DKInterfaceRef DKGetInterface( DKObjectRef _self, DKSEL sel )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)_self;
        
        DKInterfaceRef interface = DKLookupInterface( cls, sel );
        
        if( interface )
            return interface;

        DKFatalError( "DKRuntime: Interface '%s' not found on object '%s'\n", sel->name, cls->name );
    }

    return DKInterfaceNotFound();
}


///
//  DKQueryInterface()
//
bool DKQueryInterface( DKObjectRef _self, DKSEL sel, DKInterfaceRef * _interface )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = _self;
        
        DKInterfaceRef interface = DKLookupInterface( cls, sel );

        if( interface )
        {
            if( _interface )
                *_interface = interface;
            
            return true;
        }
    }
    
    return false;
}


///
//  DKGetMsgHandler()
//
DKMsgHandlerRef DKGetMsgHandler( DKObjectRef _self, DKSEL sel )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)_self;

        DKMsgHandlerRef msgHandler = (DKMsgHandlerRef)DKLookupInterface( cls, sel );

        if( msgHandler )
        {
            DKCheckKindOfClass( msgHandler, DKMsgHandlerClass(), DKMsgHandlerNotFound() );
            return msgHandler;
        }

        DKWarning( "DKRuntime: Message handler for '%s' not found on object '%s'\n", sel->name, obj->isa->name );
    }

    return DKMsgHandlerNotFound();
}


///
//  DKQueryMsgHandler()
//
bool DKQueryMsgHandler( DKObjectRef _self, DKSEL sel, DKMsgHandlerRef * _msgHandler )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)_self;

        DKMsgHandlerRef msgHandler = (DKMsgHandlerRef)DKLookupInterface( cls, sel );

        if( msgHandler )
        {
            DKCheckKindOfClass( msgHandler, DKMsgHandlerClass(), 0 );

            if( _msgHandler )
                *_msgHandler = msgHandler;
            
            return true;
        }
    }

    return false;
}


///
//  DKGetAllPropertyDefinitions()
//
DKListRef DKGetAllPropertyDefinitions( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        // If this object is a class, look in its own properties
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)_self;

        DKSpinLockLock( &cls->propertiesLock );
        DKListRef properties = DKDictionaryGetAllObjects( cls->properties );
        DKSpinLockUnlock( &cls->propertiesLock );
        
        return properties;
    }
    
    return NULL;
}


///
//  DKGetPropertyDefinition()
//
DKPropertyRef DKGetPropertyDefinition( DKObjectRef _self, DKStringRef name )
{
    if( _self )
    {
        const DKObject * obj = _self;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        // If this object is a class, look in its own properties
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)_self;

        DKSpinLockLock( &cls->propertiesLock );
        DKPropertyRef property = DKHashTableGetObject( cls->properties, name );
        DKSpinLockUnlock( &cls->propertiesLock );
        
        return property;
    }
    
    return NULL;
}




// Reference Counting ====================================================================

DKObjectRef DKRetain( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = (DKObject *)_self;

        if( (obj->isa->options & DKDisableReferenceCounting) == 0 )
        {
            DKAtomicIncrement32( &obj->refcount );
        }
    }

    return _self;
}

void DKRelease( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = (DKObject *)_self;

        if( (obj->isa->options & DKDisableReferenceCounting) == 0 )
        {
            struct DKWeak * weakref = (struct DKWeak *)obj->weakref;
            
            int32_t n;
            
            if( weakref == NULL )
            {
                n = DKAtomicDecrement32( &obj->refcount );
                DKAssert( n >= 0 );
            }
            
            else
            {
                DKSpinLockLock( &weakref->lock );
                
                n = DKAtomicDecrement32( &obj->refcount );
                DKAssert( n >= 0 );

                if( n == 0 )
                {
                    obj->weakref = NULL;
                    weakref->target = NULL;
                }
                
                DKSpinLockUnlock( &weakref->lock );
            }
            
            if( n == 0 )
            {
                DKRelease( weakref );
                DKFinalize( _self );
                DKDealloc( _self );
            }
        }
    }
}


///
//  DKRetainWeak()
//
DKWeakRef DKRetainWeak( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        
        // It doesn't make sense to get a weak reference to a weak reference.
        if( obj->isa == DKWeakClass() )
        {
            return DKRetain( obj );
        }
        
        if( !obj->weakref )
        {
            struct DKWeak * weakref = DKCreate( DKWeakClass() );
            
            weakref->lock = DKSpinLockInit;
            weakref->target = obj;
            
            if( !DKAtomicCmpAndSwapPtr( (void * volatile *)&obj->weakref, NULL, weakref ) )
                DKRelease( weakref );
        }
        
        return DKRetain( obj->weakref );
    }
    
    return NULL;
}


///
//  DKResolveWeak()
//
DKObjectRef DKResolveWeak( DKWeakRef weak_ref )
{
    if( weak_ref )
    {
        struct DKWeak * weakref = (struct DKWeak *)weak_ref;
        
        if( weakref->target )
        {
            DKSpinLockLock( &weakref->lock );
            
            DKObjectRef target = DKRetain( weakref->target );
            
            DKSpinLockUnlock( &weakref->lock );
            
            return target;
        }
    }
    
    return NULL;
}




// Reflection ============================================================================

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
        
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)_self;
        
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
//  DKStringFromSelector()
//
DKStringRef DKStringFromSelector( DKSEL sel )
{
    if( sel )
        return sel->name;
    
    return DKSTR( "null" );
}




// Polymorphic Wrappers ==================================================================

///
//  DKAlloc()
//
void * DKAlloc( DKClassRef _class, size_t extraBytes )
{
    DKObject * obj = NULL;
    
    if( _class )
    {
        DKAllocationInterfaceRef allocation;
        
        if( DKQueryInterface( _class, DKSelector(Allocation), (DKInterfaceRef *)&allocation ) )
            obj = allocation->alloc( _class, extraBytes );
        
        else
            obj = DKAllocObject( _class, extraBytes );
    }
    
    return obj;
}


///
//  DKDealloc()
//
void DKDealloc( DKObjectRef _self )
{
    DKObject * obj = (DKObject *)_self;
    
    DKAssert( obj );
    DKAssert( obj->refcount == 0 );
    DKAssert( obj->weakref == NULL );

    DKAllocationInterfaceRef allocation;

    // *** allocation->dealloc caused an exception once ***
    
    if( DKQueryInterface( obj, DKSelector(Allocation), (DKInterfaceRef *)&allocation ) )
        allocation->dealloc( obj );
    
    else
        DKDeallocObject( obj );
}


///
//  DKInit()
//
void * DKInit( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        
        if( obj->isa->init )
            _self = obj->isa->init( _self );
        
        else
            _self = DKSuperInit( _self, DKGetSuperclass( _self ) );
    }
    
    return (void *)_self;
}


///
//  DKSuperInit()
//
void * DKSuperInit( DKObjectRef _self, DKClassRef superclass )
{
    if( _self && superclass )
    {
        DKAssertKindOfClass( _self, superclass );
    
        if( superclass->init )
            _self = superclass->init( _self );
        
        else
            _self = DKSuperInit( _self, superclass->superclass );
    }
    
    return (void *)_self;
}


///
//  DKFinalize()
//
void DKFinalize( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;

        for( DKClassRef cls = obj->isa; cls != NULL; cls = cls->superclass )
        {
            if( cls->finalize )
                cls->finalize( obj );
        }
    }
}


///
//  DKEqual()
//
bool DKEqual( DKObjectRef a, DKObjectRef b )
{
    if( a && b )
    {
        if( a == b )
            return true;
        
        DKComparisonInterfaceRef comparison = DKGetInterface( a, DKSelector(Comparison) );
        return comparison->equal( a, b );
    }
    
    return false;
}


///
//  DKLike()
//
bool DKLike( DKObjectRef a, DKObjectRef b )
{
    if( a && b )
    {
        if( a == b )
            return true;
        
        DKComparisonInterfaceRef comparison = DKGetInterface( a, DKSelector(Comparison) );
        return comparison->like( a, b );
    }
    
    return false;
}


///
//  DKCompare()
//
int DKCompare( DKObjectRef a, DKObjectRef b )
{
    if( a == b )
    {
        return 0;
    }

    if( a && b )
    {
        DKComparisonInterfaceRef comparison = DKGetInterface( a, DKSelector(Comparison) );
        return comparison->compare( a, b );
    }
    
    return a < b ? -1 : 1;
}


///
//  DKHash()
//
DKHashCode DKHash( DKObjectRef _self )
{
    if( _self )
    {
        DKComparisonInterfaceRef comparison = DKGetInterface( _self, DKSelector(Comparison) );
        return comparison->hash( _self );
    }
    
    return 0;
}


///
//  DKCopy()
//
DKObjectRef DKCopy( DKObjectRef _self )
{
    if( _self )
    {
        DKCopyingInterfaceRef copying = DKGetInterface( _self, DKSelector(Copying) );
        return copying->copy( _self );
    }

    return _self;
}


///
//  DKMutableCopy()
//
DKMutableObjectRef DKMutableCopy( DKObjectRef _self )
{
    if( _self )
    {
        DKCopyingInterfaceRef copying = DKGetInterface( _self, DKSelector(Copying) );
        return copying->mutableCopy( _self );
    }

    return NULL;
}


///
//  DKCopyDescription()
//
DKStringRef DKCopyDescription( DKObjectRef _self )
{
    if( _self )
    {
        DKDescriptionInterfaceRef description = DKGetInterface( _self, DKSelector(Description) );
        return description->copyDescription( _self );
    }
    
    return DKSTR( "null" );
}








//
//  DKObject.h
//  Duck
//
//  Created by Derek Nylen on 2013-04-11.
//  Copyright (c) 2013 Hurlybird Media. All rights reserved.
//
#ifndef _DK_OBJECT_H_
#define _DK_OBJECT_H_

#include "DKEnv.h"


// Base Object Type
DKDeclareSUID( DKObjectTypeID );

// Object Interface
DKDeclareSUID( DKObjectInterfaceID );

// Type of all interface objects
DKDeclareSUID( DKInterfaceTypeID );

// Type of all class objects
DKDeclareSUID( DKClassTypeID );


enum
{
    DKObjectMutable =           (1 << 0),
    DKObjectExternalStorage =   (1 << 1),
};


typedef struct
{
    DKTypeRef       _isa;
    DKAtomicInt     _refcount;
    DKOptionFlags   _flags;
    
} DKObjectHeader;


typedef struct
{
    const DKObjectHeader _obj;

    // Reflection
    DKTypeRef   (* const getInterface)( DKTypeRef ref, DKSUID suid );
    DKSUID      (* const getTypeID)( DKTypeRef ref );
    
    // Life-Cycle
    DKTypeRef   (* const retain)( DKTypeRef ref );
    void        (* const release)( DKTypeRef ref );
    
    DKTypeRef   (* const allocate)( void );
    DKTypeRef   (* const initialize)( DKTypeRef ref );
    void        (* const finalize)( DKTypeRef ref );
    
    // Comparison
    int         (* const equal)( DKTypeRef a, DKTypeRef b );
    int         (* const compare)( DKTypeRef a, DKTypeRef b );
    DKHashIndex (* const hash)( DKTypeRef ref );

} DKObjectInterface;


DKTypeRef DKObjectClass( void );


extern const DKObjectInterface __DKClassClass__;
#define DK_CLASS_OBJECT { &__DKClassClass__, 1 }


extern const DKObjectInterface __DKInterfaceClass__;
#define DK_INTERFACE_OBJECT { &__DKInterfaceClass__, 1 }


// Concrete DKObjectInterface Implementation
DKTypeRef   DKObjectGetInterface( DKTypeRef ref, DKSUID suid );
DKSUID      DKObjectGetTypeID( DKTypeRef ref );

DKTypeRef   DKObjectRetain( DKTypeRef ref );
void        DKObjectRelease( DKTypeRef ref );

DKTypeRef   DKObjectAllocate( void );
DKTypeRef   DKObjectInitialize( DKTypeRef ref );
void        DKObjectFinalize( DKTypeRef ref );


// Retain/Release versions that do nothing
DKTypeRef   DKDoNothingRetain( DKTypeRef ref );
void        DKDoNothingRelease( DKTypeRef ref );

DKTypeRef   DKDisallowAllocate( void );
DKTypeRef   DKDisallowInitialize( DKTypeRef ref );
void        DKDisallowFinalize( DKTypeRef ref );


// Allocate a new object
DKTypeRef   DKNewObject( DKTypeRef _class, size_t size, DKOptionFlags flags );


// Polymorphic DKObjectInterface Wrappers
DKTypeRef   DKCreate( DKTypeRef _class );

DKTypeRef   DKGetClass( DKTypeRef ref );

DKTypeRef   DKGetInterface( DKTypeRef ref, DKSUID suid );
DKSUID      DKGetTypeID( DKTypeRef ref );

DKTypeRef   DKRetain( DKTypeRef ref );
void        DKRelease( DKTypeRef ref );

int         DKEqual( DKTypeRef a, DKTypeRef b );
int         DKCompare( DKTypeRef a, DKTypeRef b );
DKHashIndex DKHash( DKTypeRef ref );

// Flags
int         DKTestFlag( DKTypeRef ref, int flag );
#define     DKIsMutable( ref )  DKGetFlag( (ref), DKObjectMutable )



#endif // DK_OBJECT_H

















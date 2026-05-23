// =======================================================================================
//
// DKAllocation.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_ALLOCATION_H_
#define _DK_ALLOCATION_H_

#ifdef __cplusplus
extern "C"
{
#endif


DK_API DKDeclareInterfaceSelector( Allocation );


typedef DKObjectRef (*DKAllocMethod)( DKClassRef _class, size_t extraBytes );
typedef void (*DKDeallocMethod)( DKObjectRef _self );

struct DKAllocationInterface
{
    const DKInterface _interface;
 
    DKAllocMethod       alloc;
    DKDeallocMethod     dealloc;
};


typedef const struct DKAllocationInterface * DKAllocationInterfaceRef;


// Default allocation interface that maps to DKAllocObject and DKDeallocObject. This
// is used by the root classes so it's defined in DKRuntime.c.
DK_API DKInterfaceRef DKDefaultAllocation( void );



#ifdef __cplusplus
}
#endif

#endif // _DK_ALLOCATION_H_




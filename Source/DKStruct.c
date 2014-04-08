//
//  DKStruct.c
//  Duck
//
//  Created by Derek Nylen on 2014-04-08.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKStruct.h"
#include "DKString.h"
#include "DKStream.h"


struct DKStruct
{
    const DKObject _obj;
    DKStringRef semantic;
};


static void DKStructFinalize( DKStructRef _self );
static DKStringRef DKStructCopyDescription( DKStructRef _self );


DKThreadSafeClassInit( DKStructClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKStruct" ), DKObjectClass(), sizeof(struct DKStruct), 0 );
    
    // Allocation
    struct DKAllocationInterface * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(struct DKAllocationInterface) );
    allocation->finalize = (DKFinalizeMethod)DKStructFinalize;

    DKInstallInterface( cls, allocation );
    DKRelease( allocation );

    // Description
    struct DKDescriptionInterface * description = DKAllocInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->copyDescription = (DKCopyDescriptionMethod)DKStructCopyDescription;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    return cls;
}



///
//  DKStructFinalize()
//
static void DKStructFinalize( DKStructRef _self )
{
    DKRelease( _self->semantic );
}


///
//  DKStructCreate()
//
DKStructRef DKStructCreate( DKStringRef semantic, const void * bytes, size_t size )
{
    // Arbitrary size limit, but > 1K is probably an error
    DKAssert( size < 1024 );

    struct DKStruct * structure = DKAllocObject( DKStructClass(), size );
    structure = DKInitializeObject( structure );
    
    if( structure )
    {
        structure->semantic = DKRetain( semantic );
        DKSetObjectTag( structure, (int32_t)size );
        
        void * value = structure + 1;
        memcpy( value, bytes, size );
    }
    
    return structure;
}


///
//  DKStructGetSemantic()
//
DKStringRef DKStructGetSemantic( DKStructRef _self )
{
    if( _self )
        return _self->semantic;
    
    return NULL;
}


///
//  DKStructGetValue()
//
size_t DKStructGetValue( DKStructRef _self, DKStringRef semantic, void * bytes, size_t size )
{
    if( _self )
    {
        if( size == (size_t)DKGetObjectTag( _self ) )
        {
            if( DKEqual( _self->semantic, semantic ) )
            {
                size_t size = DKGetObjectTag( _self );

                const void * value = _self + 1;
                memcpy( bytes, value, size );
            
                return size;
            }
            
            else
            {
                DKWarning( "DKStructGetValue: Semantic mismatch '%s' != '%s'.\n",
                    DKStringGetCStringPtr( _self->semantic ),
                    DKStringGetCStringPtr( semantic ) );
            }
        }
        
        else
        {
            DKWarning( "DKStructGetValue: Size mismatch %u != %u.\n",
                (unsigned int)DKGetObjectTag( _self ), (unsigned int)size );
        }
    }
    
    return 0;
}


///
//  DKStructCopyDescription()
//
static DKStringRef DKStructCopyDescription( DKStructRef _self )
{
    if( _self )
    {
        DKMutableStringRef desc = DKStringCreateMutable();
        
        DKSPrintf( desc, "%@ (%@)", DKGetClassName( _self ), _self->semantic );
        
        return desc;
    }
    
    return NULL;
}








/*****************************************************************************************

  DKStruct.c

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

#include "DKStruct.h"
#include "DKString.h"
#include "DKStream.h"
#include "DKComparison.h"
#include "DKDescription.h"


struct DKStruct
{
    const DKObject _obj;
    DKStringRef semantic;
};


static void DKStructFinalize( DKStructRef _self );
static DKStringRef DKStructGetDescription( DKStructRef _self );


DKThreadSafeClassInit( DKStructClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKStruct" ), DKObjectClass(), sizeof(struct DKStruct), 0, NULL, (DKFinalizeMethod)DKStructFinalize );
    
    // Description
    struct DKDescriptionInterface * description = DKAllocInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->getDescription = (DKGetDescriptionMethod)DKStructGetDescription;
    description->getSizeInBytes = DKDefaultGetSizeInBytes;
    
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

    struct DKStruct * structure = DKInit( DKAlloc( DKStructClass(), size ) );
    
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
        size_t structSize = (size_t)DKGetObjectTag( _self );
    
        if( size == structSize )
        {
            if( DKEqual( _self->semantic, semantic ) )
            {
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
//  DKStructGetDescription()
//
static DKStringRef DKStructGetDescription( DKStructRef _self )
{
    if( _self )
    {
        DKMutableStringRef desc = (DKMutableStringRef)DKAutorelease( DKStringCreateMutable() );
        
        DKSPrintf( desc, "%@ (%@)", DKGetClassName( _self ), _self->semantic );
        
        return desc;
    }
    
    return NULL;
}








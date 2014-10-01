/*****************************************************************************************

  DKRuntime+Interfaces.h

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

#ifndef _DK_RUNTIME_INTERFACES_H_
#define _DK_RUNTIME_INTERFACES_H_



// DKInterface ===========================================================================
#define DK_MAX_INTERFACE_SIZE   32

typedef struct _DKInterface
{
    const DKObject  _obj;
    DKSEL           sel;
    // void *       methods[?];
    
} DKInterface;

typedef const void * DKInterfaceRef;

// Declare an interface selector.
#define DKDeclareInterfaceSelector( name )                                              \
    DKSEL DKSelector_ ## name( void )

// A generic interface where calling any method causes a fatal error. Returned by
// DKGetMsgHandler() when a matching message handler cannot be located.
DKInterfaceRef DKInterfaceNotFound( void );

// Allocate a new interface object.
void * DKAllocInterface( DKSEL sel, size_t structSize );

// Install an interface on a class.
//
// *** WARNING ***
// Replacing interfaces after a class is in use (i.e. implementation swizzling) is not
// currently supported.
void DKInstallInterface( DKClassRef cls, DKInterfaceRef interface );
void DKInstallClassInterface( DKClassRef _class, DKInterfaceRef _interface );

// Retrieve an installed interface. If a matching interface cannot be found on the class
// or any of its superclasses, DKGetInterace() will report an error and return the
// DKInterfaceNotFound() interface.
DKInterfaceRef DKGetInterface( DKObjectRef _self, DKSEL sel );
DKInterfaceRef DKGetClassInterface( DKClassRef _class, DKSEL sel );

// Check to see if an interface is available for an object.
bool DKQueryInterface( DKObjectRef _self, DKSEL sel, DKInterfaceRef * interface );
bool DKQueryClassInterface( DKClassRef _class, DKSEL sel, DKInterfaceRef * interface );




// DKMsgHandler ==========================================================================
typedef intptr_t (*DKMsgFunction)( DKObjectRef _self, DKSEL sel );

typedef struct DKMsgHandler
{
    const DKObject  _obj;
    DKSEL           sel;
    DKMsgFunction   func;
    
} DKMsgHandler;

typedef const struct DKMsgHandler * DKMsgHandlerRef;

// Declare a message handler selector. This also defines a callback type used by
// DKMsgSend() for type safety.
#define DKDeclareMessageSelector( name, ... )                                           \
    DKSEL DKSelector_ ## name( void );                                                  \
    typedef intptr_t (*DKMsgHandler_ ## name)( DKObjectRef, DKSEL , ## __VA_ARGS__ )

// A generic message handler that does nothing. Returned by DKGetMsgHandler() when a
// matching message handler cannot be located.
DKMsgHandlerRef DKMsgHandlerNotFound( void );

// Install a message handler on a class.
//
// *** WARNING ***
// Replacing message handlers after a class is in use (i.e. implementation swizzling) is
// not currently supported.
void DKInstallMsgHandler( DKClassRef cls, DKSEL sel, DKMsgFunction func );
void DKInstallClassMsgHandler( DKClassRef cls, DKSEL sel, DKMsgFunction func );

// Retrieve an installed message handler. If a matching message handler cannot be found on
// the class or any of its superclasses, DKGetMsgHandler() will report a warning and
// return the DKMsgHandlerNotFound() message handler.
DKMsgHandlerRef DKGetMsgHandler( DKObjectRef _self, DKSEL sel );
DKMsgHandlerRef DKGetClassMsgHandler( DKClassRef _class, DKSEL sel );

// Check to see if a message handler is available for an object.
bool DKQueryMsgHandler( DKObjectRef _self, DKSEL sel, DKMsgHandlerRef * msgHandler );
bool DKQueryClassMsgHandler( DKClassRef _class, DKSEL sel, DKMsgHandlerRef * msgHandler );




// Message Passing =======================================================================

// This monstrosity makes method calling somewhat "pretty".
//
// DKMsgSend does three things:
//
// 1) Retrieve a DKMsgHandler object from REF using DKSelector(msg). This is equivalent to
//    the selector returned by DKSelector( METHOD ).
//
// 2) Cast the method implementation to the DKMethod_METHOD type defined by
//    DKDeclareMsgHandlerSelector( msg ). This provides a modicum of compile-time type
//    checking.
//
// 3) Call the imp function with _self, DKSelector(msg) and the remaining arguments.
//
//    Note that the GNU C Preprocessor concat operator ## has a special case when used
//    between a comma and __VA_ARGS__: if no variable arguments are supplied, the comma
//    is omitted as well.
//
//    The preprocesser used by Clang seems to support the special case ## syntax as well.
//
//    If the method isn't defined for the object, DKGetMsgHandler returns a generic
//    implementation that produces an error.

#define DKMsgSend( _self, msg, ... ) \
    ((DKMsgHandler_ ## msg)DKGetMsgHandler( _self, DKSelector(msg) )->func)( _self, DKSelector(msg) , ## __VA_ARGS__ )
    






// Default Interfaces ====================================================================

// Allocation ----------------------------------------------------------------------------
DKDeclareInterfaceSelector( Allocation );

typedef void * (*DKAllocMethod)( DKClassRef _class, size_t extraBytes );
typedef void (*DKDeallocMethod)( DKObjectRef _self );

struct DKAllocationInterface
{
    const DKInterface _interface;
 
    DKAllocMethod       alloc;
    DKDeallocMethod     dealloc;
};

typedef const struct DKAllocationInterface * DKAllocationInterfaceRef;




// Comparison ----------------------------------------------------------------------------
DKDeclareInterfaceSelector( Comparison );

typedef DKEqualityFunction DKEqualityMethod;
typedef DKCompareFunction DKCompareMethod;
typedef DKHashFunction DKHashMethod;

struct DKComparisonInterface
{
    const DKInterface _interface;
    
    DKEqualityMethod    equal;
    DKEqualityMethod    like;
    DKCompareMethod     compare;
    DKHashMethod        hash;
};

typedef const struct DKComparisonInterface * DKComparisonInterfaceRef;

DKInterfaceRef DKDefaultComparison( void );


// Pointer equality, comparison and hashing
bool        DKPointerEqual( DKObjectRef _self, DKObjectRef other );
int         DKPointerCompare( DKObjectRef _self, DKObjectRef other );
DKHashCode  DKPointerHash( DKObjectRef ptr );




// Copying -------------------------------------------------------------------------------
DKDeclareInterfaceSelector( Copying );

typedef DKObjectRef        (*DKCopyMethod)( DKObjectRef );
typedef DKMutableObjectRef (*DKMutableCopyMethod)( DKObjectRef );

struct DKCopyingInterface
{
    const DKInterface _interface;

    DKCopyMethod        copy;
    DKMutableCopyMethod mutableCopy;
};

typedef const struct DKCopyingInterface * DKCopyingInterfaceRef;

DKInterfaceRef DKDefaultCopying( void );




// Description ---------------------------------------------------------------------------
DKDeclareInterfaceSelector( Description );

typedef DKStringRef (*DKCopyDescriptionMethod)( DKObjectRef _self );

struct DKDescriptionInterface
{
    const DKInterface _interface;
    
    DKCopyDescriptionMethod copyDescription;
};

typedef const struct DKDescriptionInterface * DKDescriptionInterfaceRef;

DKInterfaceRef DKDefaultDescription( void );


// A default copyDescription method that returns a copy of the class name
DKStringRef DKDefaultCopyDescription( DKObjectRef _self );




// Private ===============================================================================
#if DK_RUNTIME_PRIVATE

void DKInterfaceFinalize( DKObjectRef _self );

#endif // DK_RUNTIME_PRIVATE


#endif // _DK_RUNTIME_INTERFACES_H_

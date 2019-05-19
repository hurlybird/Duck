/*****************************************************************************************

  DKRuntime+Reflection.h

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

#ifndef _DK_RUNTIME_REFLECTION_
#define _DK_RUNTIME_REFLECTION_


// Returns the object itself (useful for callbacks).
DK_API DKObjectRef DKGetSelf( DKObjectRef _self );

// Returns false if the object's class was created with the DKImmutableInstances option.
DK_API bool        DKIsMutable( DKObjectRef _self );

// Retrieve the class, superclass and class name. These functions return the same values
// for classes and instances (i.e. DKGetClass(DKObjectClass()) == DKObjectClass()).
DK_API DKClassRef  DKGetClass( DKObjectRef _self );
DK_API DKStringRef DKGetClassName( DKObjectRef _self );
DK_API DKClassRef  DKGetSuperclass( DKObjectRef _self );

// Returns true if the object is a instance of the class.
DK_API bool        DKIsMemberOfClass( DKObjectRef _self, DKClassRef _class );

// Returns true if the object is a instance of the class or one of its subclasses.
DK_API bool        DKIsKindOfClass( DKObjectRef _self, DKClassRef _class );

// Returns true if the class is a subclass of (or equal to) another class
DK_API bool        DKIsSubclass( DKClassRef _class, DKClassRef otherClass );

// Convert between classes and strings
DK_API DKClassRef  DKClassFromString( DKStringRef className );
DK_API DKClassRef  DKClassFromCString( const char * className );
DK_API DKStringRef DKStringFromClass( DKClassRef _class );

// Convert between selectors and strings
DK_API DKSEL       DKSelectorFromString( DKStringRef name );
DK_API DKStringRef DKStringFromSelector( DKSEL sel );




// Private ===============================================================================
#if DK_RUNTIME_PRIVATE

void DKNameDatabaseInit( void );
void DKNameDatabaseInsertClass( DKClassRef _class );
void DKNameDatabaseRemoveClass( DKClassRef _class );
void DKNameDatabaseInsertSelector( DKSEL sel );
void DKNameDatabaseRemoveSelector( DKSEL sel );

#endif // DK_RUNTIME_PRIVATE


#endif // _DK_RUNTIME_REFLECTION_

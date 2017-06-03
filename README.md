# The Duck Object Library

Duck is an object system and library for "duck-typed" programming in C. It takes
many cues from Objective-C, CoreFoundation and COM, yet is small and readable
enough to be easily ported to platforms where using those is inconvenient.


## License

The Duck source is distributed under the MIT License. Any packages under the
'ThirdParty' directory are distributed according to their respective licenses.


## Major Features

* Classes, interfaces, properties and messages.
* Thread-safe reference counting and zeroing weak references.
* String class with practical UTF-8 support.
* Container classes for lists, dictionaries and sets.
* Object graph serialization.


## Development Status

Duck is nearly ready for a 1.0 release. The main changes to the API involve filling
in gaps and tweaking to match expected/intuitive use in a real project.


## Porting

Duck is mainly written in XCode and includes framework targets for Mac OS and iOS. It
should compile anywhere with a C99 compliant compiler. A few types and functions,
particularly those related to atomic operations, will likely need to be redefined
on non-Apple or non-POSIX systems.

(Eventually Duck should transition to C11 once compiler and OS support for that
specification is better.)

While Duck objects can be used by C++ code, C++ compilers will no doubt complain
about some of the C99 code that Duck uses--namely explicit casts from void and
inline struct initialization. The main workaround for these issues is to add extra
casts where needed and avoid any convenience macros that aren't C++ friendly.

See the CMakeLists.h file is included for building with Android Studio.

A basic SConstruct file is included for building with scons.

See *DKConfig.h* for build configuration options.


## Quick Start

Here's a snippet of code that shows what using Duck Objects looks like.

```C
#include <Duck/Duck.h>

int main( int argc, const char * argv[] )
{
    // Initialize the library
    DKRuntimeInit( 0 );
    
    // Create a mutable list
    DKMutableListRef list = DKMutableList();
    
    for( int i = 0; i < 10; i++ )
    {
        // Create a string
        DKMutableStringRef str = DKMutableString();
        
        // Strings support the stream interface so we can print to them thusly
        DKSPrintf( str, "Hello World %d", i );

        // Add the string to the list
        DKListAppendObject( list, str );
    }
    
    // Print the list to stdout
    DKPrintf( "%@\n", list );
    
    // Release the temporary objects in the autorelease pool
    DKDrainAutoreleasePool();

    return 0;
}
```


## Library Overview

### Classes

Classes in Duck are similar to classes in any object-oriented system, except
they are defined entirely at run time. In fact, you can create and destroy class
objects just like any other object.

### Interfaces

Duck interfaces are a cross between COM interfaces and Objective-C methods. As
in COM, a Duck interface is a table of function pointers that provide access to
an object's methods in a polymorphic way. Like Objective-C methods, interfaces
are bound to class objects and identified by Selectors.

### Messages

Message passing in Duck is a specialization of the interface functionality, and
works almost exactly like Objective-C message passing. A message handler is an
interface containing a single method with the following form:

```C
intptr_t MyMethod( DKObjectRef _self, DKSEL sel, ... );
```

Unlike Objective-C, Duck message handlers are limited to returning a single
pointer-sized integer. The Objective-C compiler can look at the expected return
type and pick a message send function to deal with non-integer return types.
Trying to do that in Duck risks stack corruption if the actual value returned
doesn't match the caller.

Message handlers are a useful way of adding one-off methods for target-action
paradigms, events, notifications, etc., without having to define a new interface
structure.

### Properties

Duck classes can also define properties to provide an abstract mechanism to
access instance data. The runtime handles most property get/set behaviour
automatically (including translating between object and base types), but it's
also possible to use custom getter/setter/observer methods.

### Reference Counting

All Duck objects are reference counted with the usual retain, release and
autorelease facilities.

DKWeakRef stores a weak reference to an object. The DKResolveWeak() function
resolves a weak reference into a strong reference, or returns NULL if the
target object has been deallocated.

### Built-In Container Classes

Duck provides several built-in container and data classes:

* *DKString* for UTF-8 strings.
* *DKData* for binary data.
* *DKNumber* for integer and floating-point scalars and vectors.
* *DKStruct* for storing arbitrary C structs.
* *DKArray* and *DKLinkedList* for ordered sequences.
* *DKHashTable* and *DKBinaryTree* for key-value storage.
* *DKGraph* for directed and undirected graphs

Duck also defines some common interfaces:

* *DKCollection* contains foreach style iteration methods for containers.
* *DKList* is an abstract interface for *DKArray* and *DKLinkedList*.
* *DKDictionary* is the key-value interface to *DKHashTable* and *DKBinaryTree*.
* *DKSet* has methods for set operations on *DKHashTable*, *DKBinaryTree* and *DKArray*.
* *DKCopying* has methods for creating mutable and immutable copies of objects.
* *DKConversion* provides methods to convert between strings and numbers.
* *DKStream* provides stream-style access to *DKData*, *DKString*, and *DKFile*.

### Other Useful Stuff

* *DKThread* and friends - wrapper classes for pthreads and thread synchronization.
* *DKPredicate* - evaluation of logical predicates.
* *DKEgg* - object graph serialization.
* *DKJSON* - JSON serialization functions.
* *DKXML* - XML Parsing
* Low-level utilites for generic arrays, hash tables and node/block pools.

## Future Development

Some possible/probable areas for further work:

Classes for useful system-level stuff like notifications, URLs, HTTP, etc.

A proper time/date class, or formatting functions that work with DKNumber.

More unicode support. DKString currently supports enough UTF-8 for basic string
functions, but more powerful string comparison and locale support would be nice.
The main question is how much of the ICU package to include before it makes
sense to just link against the whole library.






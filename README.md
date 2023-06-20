# The Duck Object Library

The Duck Object Library is an object system and library for "duck-typed"
programming in C. It takes many cues from Objective-C, CoreFoundation and COM,
yet is small and readable enough to be easily ported to platforms where using
those is inconvenient.


## License

The Duck source is free software distributed under the [MIT License](LICENSE).
The packages in the 'ThirdParty' directory are distributed according to their
respective licenses, but are similarly unrestrictive.


## Major Features

* Classes, interfaces, properties and messages.
* Reference counted objects with autorelease pools and zeroing weak references.
* String class with practical UTF-8 support.
* Several container types including lists, dictionaries and sets.
* I/O classes for handling JSON, XML and binary object serialization.


## Development Status

The latest stable version is 1.0.

The majority of the changes happening on the development branch involve filling
in gaps and tweaking to match expected/intuitive use. None of this should cause
widespread incompatibilites for the foreseeable future.

***While Duck is being actively used in several applications, the code has not
undergone exhaustive testing in real-world situations, particularly regarding
security and thread safety.***


## Building

Duck is mainly developed in Xcode and includes framework targets for Mac OS and
iOS. The Xcode project also contains unit tests.

A Visual Studio project is also available and has configurations for both 32 and
64-bit Windows builds.

A [CMakeLists](CMakeLists.txt) file is included for building from the command line,
Visual Studio Code, CLion, Android Studio, etc. That file also contains some
documentation on including Duck in another CMake project.

The [cmake.sh](cmake.sh) script is a convenience wrapper for building with CMake
from the command line.

From the project directory, run:
* `./cmake.sh` to build static and shared libraries.
* `./cmake.sh --debug` to build debug versions of the library.
* `./cmake.sh --examples` to build the HelloWorld example.
* `./cmake.sh --install` to install the library to */usr/local*.


## Porting

Duck is known to work on on MacOS, iOS, Linux, Windows, Android, Nintendo Switch[^1],
PlayStation 5[^2], and XBox Series X|S. It should build anywhere with a C11 compliant
compiler. Some types and functions--system calls, atomic operations, thread support,
etc--will likely need to be ported for non-Apple/POSIX/Microsoft environments.

[^1]: On Ninendo Switch the time, date, and UUID generation functions are implemented
      using proprietary Nintendo SDK APIS and are not part of this repository.
[^2]: On PlayStation 5 the time, date, and UUID generation functions are implemented
      using proprietary Sony SDK APIS and are not part of this repository.

While Duck objects can be (and are) used in C++ code, C++ compilers will no doubt
complain about some of the C11 code that Duck uses--namely explicit casts from void
and inline struct initialization. The main workaround for these issues is to add
extra casts where needed and avoid any convenience macros that aren't C++ friendly.


## Hello World

Here's a snippet of code that shows what using Duck Objects looks like. This example
is included in the HelloWorld directory.

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

Duck Classes are similar to classes in any object-oriented system, except they
are defined entirely at run time. In fact, you can create and destroy class
objects just like any other object.

*See [DKRuntime.h](Source/DKRuntime.h)*

### Interfaces

Interfaces are a cross between COM interfaces and Objective-C methods. As in
COM, a Duck interface is a table of function pointers that provide access to an
object's methods in a polymorphic way. Like Objective-C methods, interfaces are
bound to class objects and identified by Selectors.

*See [DKRuntime+Interfaces.h](Source/DKRuntime+Interfaces.h)*

### Messages

Message passing in Duck is a specialization of the interface functionality and
works much like Objective-C message passing. A message handler is an interface
containing a single method with the following form:

```C
intptr_t MyMethod( DKObjectRef _self, DKSEL sel, ... );
```

Unlike Objective-C, Duck message handlers are limited to returning a single
pointer-sized integer. The Objective-C compiler can look at the expected return
type and pick a message dispatch function to deal with non-integer return types.
Trying to do that in plain C risks stack corruption if the actual value returned
doesn't match the type expected by the caller.

Message handlers are most useful for adding one-off methods for target-action
paradigms, events, notifications, etc., without having to define a new interface
structure.

*See [DKRuntime+Interfaces.h](Source/DKRuntime+Interfaces.h)*

### Properties

Duck classes can also define properties to provide an abstract mechanism to
access instance data. The runtime handles most property get/set behaviour
automatically (including translating between object and base types), but it's
also possible to use custom getter/setter/observer methods.

*See [DKRuntime+Properties.h](Source/DKRuntime+Properties.h)*

### Reference Counting

All Duck objects are reference counted with the usual retain, release and
autorelease facilities.

Zeroing Weak references are handled by the DKWeakRef type, which stores a weak
reference to a target object. The DKResolveWeak() function resolves a weak
reference into a strong reference, or returns NULL if the target object has
been deallocated.

*See [DKRuntime+RefCount.h](Source/DKRuntime+RefCount.h)*

### Built-In Classes

The library provides several container and data classes:

* *[DKString](Source/DKString.h)* for UTF-8 strings.
* *[DKData](Source/DKData.h)* for binary data.
* *[DKNumber](Source/DKNumber.h)* for integer and floating-point scalars and vectors.
* *[DKStruct](Source/DKStruct.h)* for storing arbitrary C structs.
* *[DKArray](Source/DKArray.h)* and *[DKLinkedList](Source/DKLinkedList.h)* for ordered sequences.
* *[DKHashTable](Source/DKHashTable.h)* and *[DKBinaryTree](Source/DKBinaryTree.h)* for key-value storage.
* *[DKGraph](Source/DKGraph.h)* for directed and undirected graphs.

And some common interfaces:

* *[DKCollection](Source/DKCollection.h)* contains foreach style iteration methods for containers.
* *[DKList](Source/DKList.h)* is an abstract interface for *DKArray* and *DKLinkedList*.
* *[DKDictionary](Source/DKDictionary.h)* is the key-value interface to *DKHashTable* and *DKBinaryTree*.
* *[DKSet](Source/DKSet.h)* has methods for set operations on container types.
* *[DKCopying](Source/DKCopying.h)* has methods for creating mutable and immutable copies of objects.
* *[DKConversion](Source/DKConversion.h)* provides methods to convert between strings and numbers.
* *[DKStream](Source/DKStream.h)* provides stream-style access to *DKData*, *DKString*, and *DKFile*.

And other useful stuff:

* *[DKThread](Source/DKThread.h)* and friends - wrapper classes for pthreads and thread synchronization.
* *[DKPredicate](Source/DKPredicate.h)* - logical predicates.
* *[DKEgg](Source/DKEgg.h)* - object graph serialization.
* *[DKShell](Source/DKShell.h)* - multipart binary/text aggregates.
* *[DKJSON](Source/DKJSON.h)* - JSON serialization functions.
* *[DKXML](Source/DKXML.h)* - XML parsing.






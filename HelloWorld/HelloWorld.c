// HelloWorld.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

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

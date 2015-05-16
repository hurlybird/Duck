/*****************************************************************************************

  DKComparison.h

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

#include "DKComparison.h"


// The comparison selector is initialized by DKRuntimeInit() so that constant strings can
// be used during initialization.
//DKThreadSafeSelectorInit( Comparison );


///
//  DKPointerEqual()
//
bool DKPointerEqual( DKObjectRef _self, DKObjectRef other )
{
    return _self == other;
}


///
//  DKPointerCompare()
//
int DKPointerCompare( DKObjectRef _self, DKObjectRef other )
{
    if( _self < other )
        return 1;
    
    if( _self > other )
        return -1;
    
    return 0;
}


///
//  DKPointerHash()
//
DKHashCode DKPointerHash( DKObjectRef _self )
{
    return DKObjectUniqueHash( _self );
}


///
//  DKEqual()
//
bool DKEqual( DKObjectRef a, DKObjectRef b )
{
    if( a == b )
    {
        return true;
    }

    if( a && b )
    {
        DKComparisonInterfaceRef comparison = DKGetInterface( a, DKSelector(Comparison) );
        return comparison->equal( a, b );
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





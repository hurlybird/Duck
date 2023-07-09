/*****************************************************************************************

  DKDescription.h

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

#ifndef _DK_DESCRIPTION_H_
#define _DK_DESCRIPTION_H_

#ifdef __cplusplus
extern "C"
{
#endif


DK_API DKDeclareInterfaceSelector( Description );


typedef DKStringRef (*DKGetDescriptionMethod)( DKObjectRef _self );
typedef size_t      (*DKGetSizeInBytesMethod)( DKObjectRef _self );

struct DKDescriptionInterface
{
    const DKInterface _interface;
    
    DKGetDescriptionMethod getDescription;
    DKGetSizeInBytesMethod getSizeInBytes;
};

typedef const struct DKDescriptionInterface * DKDescriptionInterfaceRef;


// Default description interface. This is used by the root classes so it's defined in
// DKRuntime.c.
DK_API DKInterfaceRef DKDefaultDescription( void );


// A default copyDescription method that returns the class name
DK_API DKStringRef DKDefaultGetDescription( DKObjectRef _self );
DK_API size_t      DKDefaultGetSizeInBytes( DKObjectRef _self );


DK_API DKStringRef DKGetDescription( DKObjectRef _self );
DK_API size_t      DKGetSizeInBytes( DKObjectRef _self );



#ifdef __cplusplus
}
#endif

#endif // _DK_DESCRIPTION_H_




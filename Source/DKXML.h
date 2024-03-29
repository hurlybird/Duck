/*****************************************************************************************

  DKXML.h

  Copyright (c) 2016 Derek W. Nylen

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

#ifndef _DK_XML_H_
#define _DK_XML_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DKXMLElement * DKXMLElementRef;


DK_API DKClassRef DKXMLElementClass( void );

DK_API DKStringRef DKXMLElementGetName( DKXMLElementRef _self );
DK_API DKDictionaryRef DKXMLElementGetAttributes( DKXMLElementRef _self );
DK_API DKListRef DKXMLElementGetElements( DKXMLElementRef _self );
DK_API DKStringRef DKXMLElementGetDescription( DKXMLElementRef _self );


DK_API DKXMLElementRef DKXMLParse( DKStringRef xml, int options );


#ifdef __cplusplus
}
#endif

#endif // _DK_XML_H_

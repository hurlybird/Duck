/*****************************************************************************************

  DKShell.h

  Copyright (c) 2017 Derek W. Nylen

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

#ifndef _DK_SHELL_H_
#define _DK_SHELL_H_

#ifdef __cplusplus
extern "C"
{
#endif


/*

The Shell format is a lightweight wrapper for multipart files based loosely on MIME. The
key difference from MIME is the Content-Length header that replaces the boundary field
of the Content-Type.

Shell files may be concatenated together -- the result is a valid shell file.


Shell Header Format:

SHELL-Version: 1.0
Content-Type: CONTENT_TYPE
Content-Length: BYTES
(newline)
User supplied UTF-8 annotation string. '\r' and '\n' characters are not allowed.
(newline)

The annotation line is optional. If the annotation is omitted, the header ends on the
SECOND blank line (i.e. the annotation line is excluded entirely).


Content Types:

Binary types are returned as DKData and text types are returned as DKStrings unless a
registered content-type encoder changes the data type. If the Content-Type header is
omitted or unrecognized, the content is assumed to be binary and is returned as DKData.

binary
binary/egg  -- DKEgg serialized data
binary/?    -- User defined binary data

text        -- UTF-8 text
text/json   -- JSON
text/xml    -- XML (*** DECODE ONLY ***)
text/?      -- User defined text data

*/


#define DKShellContentTypeBinary    DKSTR( "binary" )
#define DKShellContentTypeEgg       DKSTR( "binary/egg" )

#define DKShellContentTypeText      DKSTR( "text" )
#define DKShellContentTypeJSON      DKSTR( "text/json" )
#define DKShellContentTypeXML       DKSTR( "text/xml" )


enum
{
    DKShellNoAutoEncoding =     (1 << 0),   // Skip content-type encoding and read/write raw data
};


typedef DKObjectRef (*DKShellEncodeFunction)( DKObjectRef object, DKObjectRef context );

// Register encode/decode callbacks for a contentType
DK_API void DKShellRegisterContentType( DKStringRef contentType, DKShellEncodeFunction encode, DKShellEncodeFunction decode, DKObjectRef context );

// Returns the number of objects read (i.e. 1) on success
DK_API int DKShellRead( DKStreamRef stream, DKObjectRef * object, DKStringRef * contentType, DKStringRef * annotation, int options );

// Returns the number of objects written (i.e. 1) on success
DK_API int DKShellWrite( DKStreamRef stream, DKObjectRef object, DKStringRef contentType, DKStringRef annotation, int options );


#ifdef __cplusplus
}
#endif

#endif // _DK_SHELL_H_



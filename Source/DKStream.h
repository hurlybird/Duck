/*****************************************************************************************

  DKStream.h

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

#ifndef _DK_STREAM_H_
#define _DK_STREAM_H_


#include "DKRuntime.h"

DKDeclareInterfaceSelector( Stream );


typedef DKObjectRef DKStreamRef;


typedef int     (*DKStreamSeekMethod)( DKObjectRef _self, long offset, int origin );
typedef long    (*DKStreamTellMethod)( DKObjectRef _self );
typedef size_t  (*DKStreamReadMethod)( DKObjectRef _self, void * data, size_t size, size_t count );
typedef size_t  (*DKStreamWriteMethod)( DKObjectRef _self, const void * data, size_t size, size_t count );

struct DKStreamInterface
{
    const DKInterface _interface;
    
    DKStreamSeekMethod seek;
    DKStreamTellMethod tell;
    DKStreamReadMethod read;
    DKStreamWriteMethod write;
};

typedef const struct DKStreamInterface * DKStreamInterfaceRef;


// Sets the current stream position.
//
// For DKData and binary DKFile streams, the new position is 'offset' bytes from the
// beginning of the stream if origin is SEEK_SET, from the current position if origin is
// SEEK_CUR, or from the end of the stream if origin is SEEK_END.
//
// For DKString and text DKFile streams the only supported values for 'offset' are zero or
// a value returned by an earlier call to DKTell() on the same stream (which only works
// when 'origin' is SEEK_SET).
//
int DKSeek( DKStreamRef _self, long offset, int origin );

// Returns the current stream position.
//
// For DKData and binary DKFile streams the position is the offset in bytes from the
// beginning of the stream.
//
// For DKString and text DKFile streams the returned value is only useful for a future
// call to DKSeek() (and only works with SEEK_SET).
//
long DKTell( DKStreamRef _self );

// Read up to 'count' items into a buffer. Returns the number of items read (which may
// be less than 'count' on partial reads or negative if an error occurs.
size_t DKRead( DKStreamRef _self, void * data, size_t size, size_t count );

// Write up to 'count' items from a buffer. Returns the number of items written (which may
// be less than 'count' on partial writes or negative if an error occurs.
size_t DKWrite( DKStreamRef _self, const void * data, size_t size, size_t count );

// Write a formatted string to a buffer.
int DKSPrintf( DKStreamRef _self, const char * format, ... );
int DKVSPrintf( DKStreamRef _self, const char * format, va_list arg_ptr );

// Read characters from a stream until a newline character is found or end-of-file occurs.
// The newline character is discarded but not stored in the returned string.
DKStringRef DKGets( DKStreamRef _self );

// Write a string to the stream. Returns EOF on failure or a non-negative value on success.
int         DKPuts( DKStreamRef _self, DKStringRef s );

// Read a character from the stream. Returns EOF on failure.
int         DKGetc( DKStreamRef _self );

// Write a character to the stream. Returns EOF on failure or the character written on success.
int         DKPutc( DKStreamRef _self, int ch );


#endif // _DK_STREAM_H_

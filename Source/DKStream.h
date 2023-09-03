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

#ifdef __cplusplus
extern "C"
{
#endif


DK_API DKDeclareInterfaceSelector( Stream );


typedef DKObjectRef DKStreamRef;


typedef int     (*DKStreamSeekMethod)( DKObjectRef _self, long offset, int origin );
typedef long    (*DKStreamTellMethod)( DKObjectRef _self );
typedef size_t  (*DKStreamReadMethod)( DKObjectRef _self, void * data, size_t size, size_t count );
typedef size_t  (*DKStreamWriteMethod)( DKObjectRef _self, const void * data, size_t size, size_t count );

typedef int     (*DKStreamGetStatusMethod)( DKObjectRef _self );
typedef DKIndex (*DKStreamGetLengthMethod)( DKObjectRef _self );

struct DKStreamInterface
{
    const DKInterface _interface;
    
    DKStreamSeekMethod seek;
    DKStreamTellMethod tell;
    DKStreamReadMethod read;
    DKStreamWriteMethod write;
    
    DKStreamGetStatusMethod getStatus;
    DKStreamGetLengthMethod getLength;
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
DK_API int DKSeek( DKStreamRef _self, long offset, int origin );

// Returns the current stream position.
//
// For DKData and binary DKFile streams the position is the offset in bytes from the
// beginning of the stream.
//
// For DKString and text DKFile streams the returned value is only useful for a future
// call to DKSeek() (and only works with SEEK_SET).
//
DK_API long DKTell( DKStreamRef _self );

// Read up to 'count' items into a buffer. Returns the number of items read (which may
// be less than 'count' on partial reads or negative if an error occurs.
DK_API size_t DKRead( DKStreamRef _self, void * data, size_t size, size_t count );

// Write up to 'count' items from a buffer. Returns the number of items written (which may
// be less than 'count' on partial writes or negative if an error occurs.
DK_API size_t DKWrite( DKStreamRef _self, const void * data, size_t size, size_t count );

// Get the status of the stream
DK_API int DKStreamGetStatus( DKStreamRef _self );

// Get the current length of the stream in bytes. Returns DKNotFound if the length cannot
// be retrieved (e.g. for a socket backed stream).
DK_API DKIndex DKStreamGetLength( DKStreamRef _self );

// Write a formatted string to a buffer.
DK_API int DKSPrintf( DKStreamRef _self, const char * format, ... );
DK_API int DKVSPrintf( DKStreamRef _self, const char * format, va_list arg_ptr );

// Read characters from a stream until a newline character is found or end-of-file occurs.
// The newline character is discarded but not stored in the returned string.
DK_API DKStringRef DKGets( DKStreamRef _self );

// Write a string to the stream. Returns EOF on failure or a non-negative value on success.
DK_API int         DKPuts( DKStreamRef _self, DKStringRef s );

// Read a character from the stream. Returns EOF on failure.
DK_API int         DKGetc( DKStreamRef _self );

// Write a character to the stream. Returns EOF on failure or the character written on success.
DK_API int         DKPutc( DKStreamRef _self, int ch );



#ifdef __cplusplus
}
#endif

#endif // _DK_STREAM_H_

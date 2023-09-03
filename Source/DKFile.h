/*****************************************************************************************

  DKFile.h

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

#ifndef _DK_FILE_H_
#define _DK_FILE_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DKFile * DKFileRef;


DK_API DKClassRef  DKFileClass( void );

#define DKFile( filename, mode )                            DKAutorelease( DKFileOpen( filename, mode ) )
#define DKNewFile( filename, mode )                         DKFileOpen( filename, mode )

#define DKFileWithStreamPtr( stream, closeOnDealloc )       DKAutorelease( DKFileInitWithStreamPtr( DKAlloc( DKFileClass() ), stream, closeOnDealloc ) )
#define DKNewFileWithStreamPtr( stream, closeOnDealloc )    DKFileInitWithStreamPtr( DKAlloc( DKFileClass() ), stream, closeOnDealloc )

DK_API DKFileRef   DKFileInitWithStreamPtr( DKObjectRef _self, FILE * stream, bool closeOnDealloc );

// Returns true if the file exists
DK_API bool        DKFileExists( DKStringRef filename );

// Create and open a new file - the returned object must be closed or released
DK_API DKFileRef   DKFileOpen( DKStringRef filename, const char * mode );

// Close the file and release the object reference
DK_API int         DKFileClose( DKFileRef _self );

DK_API FILE *      DKFileGetStreamPtr( DKFileRef _self );

DK_API int         DKFileSeek( DKFileRef _self, long offset, int origin );
DK_API long        DKFileTell( DKFileRef _self );

DK_API int         DKFileGetStatus( DKFileRef _self );
DK_API DKIndex     DKFileGetLength( DKFileRef _self );

DK_API size_t      DKFileRead( DKFileRef _self, void * buffer, size_t size, size_t count );
DK_API size_t      DKFileWrite( DKFileRef _self, const void * buffer, size_t size, size_t count );


#ifdef __cplusplus
}
#endif

#endif // _DK_FILE_H_

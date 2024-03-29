/*****************************************************************************************

  DKFile.c

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

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKFile.h"
#include "DKStream.h"
#include "DKString.h"


struct DKFile
{
    DKObject _obj;
    FILE * file;
    bool closeOnDealloc;
};


static void DKFileFinalize( DKObjectRef _self );


///
//  DKFileClass()
//
DKThreadSafeClassInit( DKFileClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKFile" ), DKObjectClass(), sizeof(struct DKFile), 0, NULL, DKFileFinalize );
    
    // Stream
    struct DKStreamInterface * stream = DKNewInterface( DKSelector(Stream), sizeof(struct DKStreamInterface) );
    stream->seek = (DKStreamSeekMethod)DKFileSeek;
    stream->tell = (DKStreamTellMethod)DKFileTell;
    stream->read = (DKStreamReadMethod)DKFileRead;
    stream->write = (DKStreamWriteMethod)DKFileWrite;
    stream->getStatus = (DKStreamGetStatusMethod)DKFileGetStatus;
    stream->getLength = (DKStreamGetLengthMethod)DKFileGetLength;
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );
    
    return cls;
}


///
//  DKFileInitWithStreamPtr()
//
DKFileRef DKFileInitWithStreamPtr( DKObjectRef _untyped_self, FILE * stream, bool closeOnDealloc )
{
    DKFileRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
        _self->file = stream;
        _self->closeOnDealloc = closeOnDealloc;
    }
    
    return _self;
}


///
//  DKFileFinalize()
//
static void DKFileFinalize( DKObjectRef _untyped_self )
{
    DKFileRef _self = _untyped_self;
    
    if( _self->file )
    {
        if( _self->closeOnDealloc )
            fclose( _self->file );
        
        _self->file = NULL;
    }
}


///
//  DKFileExists()
//
bool DKFileExists( DKStringRef filename )
{
#if DK_PLATFORM_POSIX
    const char * fname = DKStringGetCStringPtr( filename );

    struct stat fileStats;
    return stat( fname, &fileStats ) == 0;
#elif DK_PLATFORM_WINDOWS
    const char * fname = DKStringGetCStringPtr( filename );

    DWORD attr = GetFileAttributesA( fname );
    return (attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    DKAssert( 0 );
#endif
}


///
//  DKFileOpen()
//
DKFileRef DKFileOpen( DKStringRef filename, const char * mode )
{
    const char * fname = DKStringGetCStringPtr( filename );
    FILE * file = fopen( fname, mode );
    
    if( file )
    {
        DKFileRef _self = DKNew( DKFileClass() );
    
        if( _self )
        {
            _self->file = file;
            _self->closeOnDealloc = true;
            
            return _self;
        }
        
        DKAssert( 0 );
        fclose( file );
    }
    
    return NULL;
}


///
//  DKFileClose()
//
int DKFileClose( DKFileRef _self )
{
    int result = EOF;

    if( _self )
    {
        DKAssertKindOfClass( _self, DKFileClass() );

        if( _self->file )
        {
            result = fclose( _self->file );
            _self->file = NULL;
        }
        
        DKRelease( _self );
    }
    
    return result;
}


///
//  DKFileGetStreamPtr()
//
FILE * DKFileGetStreamPtr( DKFileRef _self )
{
    if( _self )
        return _self->file;
    
    return NULL;
}


///
//  DKFileSeek()
//
int DKFileSeek( DKFileRef _self, long offset, int origin )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKFileClass() );

        if( _self->file )
            return fseek( _self->file, offset, origin );
    }
    
    return -1;
}


///
//  DKFileTell()
//
long DKFileTell( DKFileRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKFileClass() );

        if( _self->file )
            return ftell( _self->file );
    }
    
    return -1;
}


///
//  DKFileGetStatus()
//
int DKFileGetStatus( DKFileRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKFileClass() );

        if( _self->file )
            return ferror( _self->file );
    }
    
    return -1;
}


///
//  DKFileGetLength()
//
DKIndex DKFileGetLength( DKFileRef _self )
{
    // Using a DKIndex for the length effectively limits file length to ~3 GB on 32-bit builds
    DKIndex length = 0;

    if( _self )
    {
        DKAssertKindOfClass( _self, DKFileClass() );

        if( _self->file )
        {
#if DK_PLATFORM_POSIX
            int fd = fileno( _self->file );
            struct stat fileStats;
            fstat( fd, &fileStats );
            length = (DKIndex)fileStats.st_size;
#elif DK_PLATFORM_WINDOWS
            int fd = _fileno( _self->file );
            struct _stat64 fileStats;
            _fstat64( fd, &fileStats );
            length = (DKIndex)fileStats.st_size;
#else
            #pragma message( "Using fseek + ftell for file stream length - the behaviour is platform dependent and may be unsupported." )
            
            long cursor = ftell( _self->file );
            fseek( _self->file, 0, SEEK_END );
            length = ftell( _self->file );
            fseek( _self->file, cursor, SEEK_SET );
#endif
        }
    }

    return length;
}


///
//  DKFileRead()
//
size_t DKFileRead( DKFileRef _self, void * buffer, size_t size, size_t count )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKFileClass() );

        if( _self->file )
            return fread( buffer, size, count, _self->file );
    }
    
    return 0;
}


///
//  DKFileWrite()
//
size_t DKFileWrite( DKFileRef _self, const void * buffer, size_t size, size_t count )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKFileClass() );

        if( _self->file )
            return fwrite( buffer, size, count, _self->file );
    }
    
    return 0;
}











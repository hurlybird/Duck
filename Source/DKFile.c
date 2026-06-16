// =======================================================================================
//
// DKFile.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
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
    struct DKStreamInterface * stream = DKNewInterface( DKSelector(Stream) );
    stream->seek = (DKStreamSeekMethod)DKFileSeek;
    stream->tell = (DKStreamTellMethod)DKFileTell;
    stream->read = (DKStreamReadMethod)DKFileRead;
    stream->write = (DKStreamWriteMethod)DKFileWrite;
    stream->flush = (DKStreamFlushMethod)DKFileFlush;
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
//  DKDeleteFile()
//
void DKDeleteFile( DKStringRef filename )
{
    remove( DKStringGetCStringPtr( filename ) );
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
//  DKFileFlush()
//
int DKFileFlush( DKFileRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKFileClass() );

        if( _self->file )
            return fflush( _self->file );
    }
    
    return 0;
}


///
//  DKFileGetStatus()
//
int DKFileGetStatus( DKFileRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKFileClass() );

        if( _self->file == NULL )
            return DKStreamClosed;
            
        else if( feof( _self->file ) )
            return DKStreamEOF;
            
        else if( ferror( _self->file ) )
            return DKStreamError;
            
        else
            return DKStreamOK;
    }
    
    return DKStreamEOF;
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











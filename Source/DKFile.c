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

#include "DKFile.h"
#include "DKStream.h"
#include "DKString.h"


struct DKFile
{
    DKObject _obj;
    FILE * file;
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
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );
    
    return cls;
}


///
//  DKFileFinalize()
//
static void DKFileFinalize( DKObjectRef _untyped_self )
{
    DKFileRef _self = _untyped_self;
    
    if( _self->file )
    {
        fclose( _self->file );
        _self->file = NULL;
    }
}


///
//  DKFileOpen()
//
DKFileRef DKFileOpen( DKStringRef filename, const char * mode )
{
    struct DKFile * file = DKNew( DKFileClass() );
    
    if( file )
    {
        const char * fname = DKStringGetCStringPtr( filename );
        
        file->file = fopen( fname, mode );
        
        if( file->file )
            return file;
        
        DKRelease( file );
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
//  DKFileSeek()
//
int DKFileSeek( DKFileRef _self, DKIndex offset, int origin )
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
DKIndex DKFileTell( DKFileRef _self )
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
//  DKFileGetLength()
//
DKIndex DKFileGetLength( DKFileRef _self )
{
    DKIndex length = 0;

    if( _self )
    {
        DKAssertKindOfClass( _self, DKFileClass() );

        if( _self->file )
        {
#if DK_PLATFORM_BSD
            int fd = fileno( _self->file );
            struct stat fileStats;
            fstat( fd, &fileStats );
            length = fileStats.st_size;
#else
            DKIndex cursor = ftell( _self->file );
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
DKIndex DKFileRead( DKFileRef _self, void * buffer, DKIndex size, DKIndex count )
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
DKIndex DKFileWrite( DKFileRef _self, const void * buffer, DKIndex size, DKIndex count )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKFileClass() );

        if( _self->file )
            return fwrite( buffer, size, count, _self->file );
    }
    
    return 0;
}











//
//  DKFile.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKFile.h"
#include "DKStream.h"
#include "DKString.h"


struct DKFile
{
    DKObjectHeader _obj;
    FILE * file;
};


static void DKFileFinalize( DKObjectRef _self );


///
//  DKFileClass()
//
DKThreadSafeClassInit( DKFileClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKFile" ), DKObjectClass(), sizeof(struct DKFile), 0 );
    
    // Allocation
    struct DKAllocation * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(DKAllocation) );
    allocation->finalize = DKFileFinalize;

    DKInstallInterface( cls, allocation );
    DKRelease( allocation );

    // Stream
    struct DKStream * stream = DKAllocInterface( DKSelector(Stream), sizeof(DKStream) );
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
static void DKFileFinalize( DKObjectRef _self )
{
    DKFileClose( _self );
}


///
//  DKFileCreate()
//
DKFileRef DKFileCreate( void )
{
    return DKAllocObject( DKFileClass(), 0 );
}


///
//  DKFileOpen()
//
int DKFileOpen( DKFileRef _self, const char * fname, const char * mode )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKFileClass() );
    
        DKFileClose( _self );

        struct DKFile * file = (struct DKFile *)_self;
        
        file->file = fopen( fname, mode );
        
        if( file->file )
            return 0;
    }
    
    return -1;
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

        struct DKFile * file = (struct DKFile *)_self;
        
        if( file->file )
        {
            result = fclose( file->file );
            file->file = NULL;
        }
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

        struct DKFile * file = (struct DKFile *)_self;
        
        if( file->file )
            return fwrite( buffer, size, count, file->file );
    }
    
    return 0;
}











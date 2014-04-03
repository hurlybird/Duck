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


static void DKFileFinalize( DKObjectRef ref );


///
//  DKFileClass()
//
DKThreadSafeClassInit( DKFileClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKFile" ), DKObjectClass(), sizeof(struct DKFile) );
    
    // LifeCycle
    struct DKLifeCycle * lifeCycle = DKAllocInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
    lifeCycle->finalize = DKFileFinalize;

    DKInstallInterface( cls, lifeCycle );
    DKRelease( lifeCycle );

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
static void DKFileFinalize( DKObjectRef ref )
{
    DKFileClose( ref );
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
int DKFileOpen( DKFileRef ref, const char * fname, const char * mode )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKFileClass(), -1 );
    
        DKFileClose( ref );

        struct DKFile * file = (struct DKFile *)ref;
        
        file->file = fopen( fname, mode );
        
        if( file->file )
            return 0;
    }
    
    return -1;
}


///
//  DKFileClose()
//
int DKFileClose( DKFileRef ref )
{
    int result = EOF;

    if( ref )
    {
        DKVerifyKindOfClass( ref, DKFileClass(), -1 );

        struct DKFile * file = (struct DKFile *)ref;
        
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
int DKFileSeek( DKFileRef ref, DKIndex offset, int origin )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKFileClass(), -1 );

        if( ref->file )
            return fseek( ref->file, offset, origin );
    }
    
    return -1;
}


///
//  DKFileTell()
//
DKIndex DKFileTell( DKFileRef ref )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKFileClass(), -1 );

        if( ref->file )
            return ftell( ref->file );
    }
    
    return -1;
}


///
//  DKFileRead()
//
DKIndex DKFileRead( DKFileRef ref, void * buffer, DKIndex size, DKIndex count )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKFileClass(), -1 );

        if( ref->file )
            return fread( buffer, size, count, ref->file );
    }
    
    return 0;
}


///
//  DKFileWrite()
//
DKIndex DKFileWrite( DKFileRef ref, const void * buffer, DKIndex size, DKIndex count )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKFileClass(), -1 );

        struct DKFile * file = (struct DKFile *)ref;
        
        if( file->file )
            return fwrite( buffer, size, count, file->file );
    }
    
    return 0;
}











//
//  DKFile.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKFile.h"
#include "DKLifeCycle.h"
#include "DKStream.h"


struct DKFile
{
    DKObjectHeader _obj;
    FILE * file;
};


static DKTypeRef DKFileAllocate( void );
static DKTypeRef DKFileInitialize( DKTypeRef ref );
static void DKFileFinalize( DKTypeRef ref );


///
//  DKFileClass()
//
DKTypeRef DKFileClass( void )
{
    DKTypeRef fileClass = NULL;
    
    if( !fileClass )
    {
        fileClass = DKAllocClass( DKObjectClass() );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKAllocInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKFileAllocate;
        lifeCycle->initialize = DKFileInitialize;
        lifeCycle->finalize = DKFileFinalize;

        DKInstallInterface( fileClass, lifeCycle );
        DKRelease( lifeCycle );

        // Stream
        struct DKStream * stream = (struct DKStream *)DKAllocInterface( DKSelector(Stream), sizeof(DKStream) );
        stream->seek = DKFileSeek;
        stream->tell = DKFileTell;
        stream->read = DKFileRead;
        stream->write = DKFileWrite;
        
        DKInstallInterface( fileClass, stream );
        DKRelease( stream );
    }
    
    return fileClass;
}


///
//  DKFileAllocate()
//
static DKTypeRef DKFileAllocate( void )
{
    return DKAllocObject( DKFileClass(), sizeof(struct DKFile), 0 );
}


///
//  DKFileInitialize()
//
static DKTypeRef DKFileInitialize( DKTypeRef ref )
{
    ref = DKObjectInitialize( ref );
    
    if( ref )
    {
        struct DKFile * file = (struct DKFile *)ref;
        file->file = NULL;
    }
    
    return ref;
}


///
//  DKFileFinalize()
//
static void DKFileFinalize( DKTypeRef ref )
{
    DKFileClose( ref );
}


///
//  DKFileCreate()
//
DKTypeRef DKFileCreate( void )
{
    return DKCreate( DKFileClass() );
}


///
//  DKFileOpen()
//
int DKFileOpen( DKTypeRef ref, const char * fname, const char * mode )
{
    if( ref )
    {
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
int DKFileClose( DKTypeRef ref )
{
    int result = EOF;

    if( ref )
    {
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
int DKFileSeek( DKTypeRef ref, DKIndex offset, int origin )
{
    if( ref )
    {
        struct DKFile * file = (struct DKFile *)ref;
        
        if( file->file )
            return fseek( file->file, offset, origin );
    }
    
    return -1;
}


///
//  DKFileTell()
//
DKIndex DKFileTell( DKTypeRef ref )
{
    if( ref )
    {
        struct DKFile * file = (struct DKFile *)ref;
        
        if( file->file )
            return ftell( file->file );
    }
    
    return -1;
}


///
//  DKFileRead()
//
DKIndex DKFileRead( DKTypeRef ref, void * buffer, DKIndex size, DKIndex count )
{
    if( ref )
    {
        struct DKFile * file = (struct DKFile *)ref;
        
        if( file->file )
            return fread( buffer, size, count, file->file );
    }
    
    return 0;
}


///
//  DKFileWrite()
//
DKIndex DKFileWrite( DKTypeRef ref, const void * buffer, DKIndex size, DKIndex count )
{
    if( ref )
    {
        struct DKFile * file = (struct DKFile *)ref;
        
        if( file->file )
            return fwrite( buffer, size, count, file->file );
    }
    
    return 0;
}











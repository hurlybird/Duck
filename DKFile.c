//
//  DKFile.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKFile.h"
#include "DKStream.h"


struct DKFile
{
    DKObjectHeader _obj;
    FILE * file;
};


static void DKFileFinalize( DKTypeRef ref );


///
//  DKFileClass()
//
DKTypeRef DKFileClass( void )
{
    DKTypeRef SharedClassObject = NULL;
    
    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( "DKFile", DKObjectClass(), sizeof(struct DKFile) );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->finalize = DKFileFinalize;

        DKInstallInterface( SharedClassObject, lifeCycle );
        DKRelease( lifeCycle );

        // Stream
        struct DKStream * stream = (struct DKStream *)DKCreateInterface( DKSelector(Stream), sizeof(DKStream) );
        stream->seek = DKFileSeek;
        stream->tell = DKFileTell;
        stream->read = DKFileRead;
        stream->write = DKFileWrite;
        
        DKInstallInterface( SharedClassObject, stream );
        DKRelease( stream );
    }
    
    return SharedClassObject;
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
        DKAssert( DKIsKindOfClass( ref, DKFileClass() ) );
    
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
        DKAssert( DKIsKindOfClass( ref, DKFileClass() ) );
        
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
        DKAssert( DKIsKindOfClass( ref, DKFileClass() ) );

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
        DKAssert( DKIsKindOfClass( ref, DKFileClass() ) );

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
        DKAssert( DKIsKindOfClass( ref, DKFileClass() ) );

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
        DKAssert( DKIsKindOfClass( ref, DKFileClass() ) );

        struct DKFile * file = (struct DKFile *)ref;
        
        if( file->file )
            return fwrite( buffer, size, count, file->file );
    }
    
    return 0;
}











//
//  DKStream.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKStream.h"

DKDefineInterface( Stream );


///
//  DKSeek()
//
int DKSeek( DKTypeRef ref, DKIndex offset, int origin )
{
    if( ref )
    {
        DKStream * stream = DKGetInterface( ref, DKSelector(Stream) );
        return stream->seek( ref, offset, origin );
    }
    
    return -1;
}


///
//  DKTell()
//
DKIndex DKTell( DKTypeRef ref )
{
    if( ref )
    {
        DKStream * stream = DKGetInterface( ref, DKSelector(Stream) );
        return stream->tell( ref );
    }
    
    return -1;
}


///
//  DKRead()
//
DKIndex DKRead( DKTypeRef ref, void * data, DKIndex size, DKIndex count )
{
    if( ref )
    {
        DKStream * stream = DKGetInterface( ref, DKSelector(Stream) );
        return stream->read( ref, data, size, count );
    }
    
    return 0;
}


///
//  DKWrite()
//
DKIndex DKWrite( DKTypeRef ref, const void * data, DKIndex size, DKIndex count )
{
    if( ref )
    {
        DKStream * stream = DKGetInterface( ref, DKSelector(Stream) );
        return stream->write( ref, data, size, count );
    }
    
    return 0;
}


///
//  DKSPrintf()
//
DKIndex DKSPrintf( DKTypeRef ref, const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    DKIndex result = DKVSPrintf( ref, format, arg_ptr );
    
    va_end( arg_ptr );
    
    return result;
}


///
//  DKSScanf()
//
DKIndex DKSScanf( DKTypeRef ref, const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    DKIndex result = DKVSScanf( ref, format, arg_ptr );
    
    va_end( arg_ptr );
    
    return result;
}


///
//  DKVSPrintf()
//
DKIndex DKVSPrintf( DKTypeRef ref, const char * format, va_list arg_ptr )
{
    return 0;
}


///
//  DKVSScanf()
//
DKIndex DKVSScanf( DKTypeRef ref, const char * format, va_list arg_ptr )
{
    return 0;
}








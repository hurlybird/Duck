//
//  DKStream.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_STREAM_H_
#define _DK_STREAM_H_


#include "DKRuntime.h"

DKDeclareInterface( Stream );


enum
{
    DKSeekSet = SEEK_SET,
    DKSeekCur = SEEK_CUR,
    DKSeekEnd = SEEK_END
};


struct DKStream
{
    DKInterface _interface;
    
    int     (*seek)( DKTypeRef ref, DKIndex offset, int origin );
    DKIndex (*tell)( DKTypeRef ref );
    
    DKIndex (*read)( DKTypeRef ref, void * data, DKIndex size, DKIndex count );
    DKIndex (*write)( DKTypeRef ref, const void * data, DKIndex size, DKIndex count );
};

typedef const struct DKStream DKStream;


int DKSeek( DKTypeRef ref, DKIndex offset, int origin );
DKIndex DKTell( DKTypeRef ref );

DKIndex DKRead( DKTypeRef ref, void * data, DKIndex size, DKIndex count );
DKIndex DKWrite( DKTypeRef ref, const void * data, DKIndex size, DKIndex count );

DKIndex DKSPrintf( DKTypeRef ref, const char * format, ... );

DKIndex DKVSPrintf( DKTypeRef ref, const char * format, va_list arg_ptr );



#endif // _DK_STREAM_H_

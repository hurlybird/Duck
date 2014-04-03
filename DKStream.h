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

DKDeclareInterfaceSelector( Stream );


enum
{
    DKSeekSet = SEEK_SET,
    DKSeekCur = SEEK_CUR,
    DKSeekEnd = SEEK_END
};

typedef int (*DKStreamSeekMethod)( DKObjectRef ref, DKIndex offset, int origin );
typedef DKIndex (*DKStreamTellMethod)( DKObjectRef ref );
typedef DKIndex (*DKStreamReadMethod)( DKObjectRef ref, void * data, DKIndex size, DKIndex count );
typedef DKIndex (*DKStreamWriteMethod)( DKMutableObjectRef ref, const void * data, DKIndex size, DKIndex count );

struct DKStream
{
    DKInterface _interface;
    
    DKStreamSeekMethod seek;
    DKStreamTellMethod tell;
    DKStreamReadMethod read;
    DKStreamWriteMethod write;
};

typedef const struct DKStream DKStream;


int DKSeek( DKObjectRef ref, DKIndex offset, int origin );
DKIndex DKTell( DKObjectRef ref );

DKIndex DKRead( DKObjectRef ref, void * data, DKIndex size, DKIndex count );
DKIndex DKWrite( DKMutableObjectRef ref, const void * data, DKIndex size, DKIndex count );

DKIndex DKSPrintf( DKMutableObjectRef ref, const char * format, ... );

DKIndex DKVSPrintf( DKMutableObjectRef ref, const char * format, va_list arg_ptr );



#endif // _DK_STREAM_H_

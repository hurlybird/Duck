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

typedef int (*DKStreamSeekMethod)( DKObjectRef _self, DKIndex offset, int origin );
typedef DKIndex (*DKStreamTellMethod)( DKObjectRef _self );
typedef DKIndex (*DKStreamReadMethod)( DKObjectRef _self, void * data, DKIndex size, DKIndex count );
typedef DKIndex (*DKStreamWriteMethod)( DKMutableObjectRef _self, const void * data, DKIndex size, DKIndex count );

struct DKStream
{
    DKInterface _interface;
    
    DKStreamSeekMethod seek;
    DKStreamTellMethod tell;
    DKStreamReadMethod read;
    DKStreamWriteMethod write;
};

typedef const struct DKStream DKStream;


int DKSeek( DKObjectRef _self, DKIndex offset, int origin );
DKIndex DKTell( DKObjectRef _self );

DKIndex DKRead( DKObjectRef _self, void * data, DKIndex size, DKIndex count );
DKIndex DKWrite( DKMutableObjectRef _self, const void * data, DKIndex size, DKIndex count );

DKIndex DKSPrintf( DKMutableObjectRef _self, const char * format, ... );

DKIndex DKVSPrintf( DKMutableObjectRef _self, const char * format, va_list arg_ptr );



#endif // _DK_STREAM_H_

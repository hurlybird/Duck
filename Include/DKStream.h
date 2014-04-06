/*******************************************************************************

  DKStream.h

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

*******************************************************************************/

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

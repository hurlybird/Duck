/*****************************************************************************************

  DKCollection.h

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

#ifndef _DK_COLLECTION_H_
#define _DK_COLLECTION_H_

#include "DKRuntime.h"


DKDeclareInterfaceSelector( Collection );


typedef DKIndex (*DKGetCountMethod)( DKObjectRef _self );
typedef int     (*DKForeachObjectMethod)( DKObjectRef _self, DKApplierFunction callback, void * context );
typedef int     (*DKForeachKeyAndObjectMethod)( DKObjectRef _self, DKKeyedApplierFunction callback, void * context );

struct DKCollectionInterface
{
    const DKInterface _interface;

    DKGetCountMethod            getCount;
    DKForeachObjectMethod       foreachObject;

    // These may be NULL for non-keyed collections
    DKForeachObjectMethod       foreachKey;
    DKForeachKeyAndObjectMethod foreachKeyAndObject;
};

typedef const struct DKCollectionInterface * DKCollectionInterfaceRef;



DKIndex     DKGetCount( DKObjectRef _self );

int         DKForeachKey( DKObjectRef _self, DKApplierFunction callback, void * context );
int         DKForeachObject( DKObjectRef _self, DKApplierFunction callback, void * context );
int         DKForeachKeyAndObject( DKObjectRef _self, DKKeyedApplierFunction callback, void * context );


DKStringRef DKCollectionCopyDescription( DKObjectRef _self );




#endif // _DK_COLLECTION_H_



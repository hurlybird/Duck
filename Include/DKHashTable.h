/*******************************************************************************

  DKHashTable.h

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

#ifndef _DK_HASHTABLE_H_
#define _DK_HASHTABLE_H_

#include "DKDictionary.h"


typedef const struct DKHashTable * DKHashTableRef;
typedef struct DKHashTable * DKMutableHashTableRef;


DKClassRef DKHashTableClass( void );
DKClassRef DKMutableHashTableClass( void );

DKHashTableRef DKHashTableCreate( void );
DKHashTableRef DKHashTableCreateWithKeysAndObjects( DKObjectRef firstKey, ... );
DKHashTableRef DKHashTableCreateCopy( DKDictionaryRef srcDictionary );

DKMutableHashTableRef DKHashTableCreateMutable( void );
DKMutableHashTableRef DKHashTableCreateMutableCopy( DKDictionaryRef srcDictionary );

DKIndex     DKHashTableGetCount( DKHashTableRef _self );
DKObjectRef DKHashTableGetObject( DKHashTableRef _self, DKObjectRef key );

int         DKHashTableApplyFunction( DKHashTableRef _self, DKDictionaryApplierFunction callback, void * context );

void        DKHashTableInsertObject( DKMutableHashTableRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
void        DKHashTableRemoveObject( DKMutableHashTableRef _self, DKObjectRef key );
void        DKHashTableRemoveAllObjects( DKMutableHashTableRef _self );




#endif // _DK_HASHTABLE_H_

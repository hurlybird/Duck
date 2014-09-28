/*****************************************************************************************

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

*****************************************************************************************/

#ifndef _DK_HASHTABLE_H_
#define _DK_HASHTABLE_H_

#include "DKDictionary.h"
#include "DKSet.h"


typedef const struct DKHashTable * DKHashTableRef;
typedef struct DKHashTable * DKMutableHashTableRef;


DKClassRef  DKHashTableClass( void );
DKClassRef  DKMutableHashTableClass( void );

#define     DKHashTableCreateEmpty()    DKCreate( DKHashTableClass() )
#define     DKHashTableCreateMutable()  DKCreate( DKMutableHashTableClass() )

DKObjectRef DKHashTableCreateDictionaryWithVAKeysAndObjects( DKClassRef _class, va_list keysAndObjects );
DKObjectRef DKHashTableCreateDictionaryWithDictionary( DKClassRef _class, DKDictionaryRef srcDictionary );

DKObjectRef DKHashTableCreateSetWithVAObjects( DKClassRef _class, va_list objects );
DKObjectRef DKHashTableCreateSetWithCArray( DKClassRef _class, DKObjectRef objects[], DKIndex count );
DKObjectRef DKHashTableCreateSetWithCollection( DKClassRef _class, DKObjectRef srcCollection );

DKHashTableRef DKHashTableCopy( DKHashTableRef _self );
DKMutableHashTableRef DKHashTableMutableCopy( DKHashTableRef _self );

DKIndex     DKHashTableGetCount( DKHashTableRef _self );
DKObjectRef DKHashTableGetObject( DKHashTableRef _self, DKObjectRef key );

int         DKHashTableApplyFunction( DKHashTableRef _self, DKKeyedApplierFunction callback, void * context );
int         DKHashTableApplyFunctionToKeys( DKHashTableRef _self, DKApplierFunction callback, void * context );
int         DKHashTableApplyFunctionToObjects( DKHashTableRef _self, DKApplierFunction callback, void * context );

void        DKHashTableInsertObject( DKMutableHashTableRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
void        DKHashTableRemoveObject( DKMutableHashTableRef _self, DKObjectRef key );
void        DKHashTableRemoveAllObjects( DKMutableHashTableRef _self );

void        DKHashTableAddObjectToSet( DKMutableHashTableRef _self, DKObjectRef object );



#endif // _DK_HASHTABLE_H_

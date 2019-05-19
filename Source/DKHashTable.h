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


typedef struct DKHashTable * DKHashTableRef;
typedef struct DKHashTable * DKMutableHashTableRef;


DK_API DKClassRef  DKHashTableClass( void );
DK_API DKClassRef  DKMutableHashTableClass( void );

#define     DKEmptyHashTable()      DKAutorelease( DKNew( DKHashTableClass() ) )
#define     DKMutableHashTable()    DKAutorelease( DKNew( DKMutableHashTableClass() ) )

#define     DKNewMutableHashTable() DKNew( DKMutableHashTableClass() )

DK_API DKObjectRef DKHashTableInitDictionaryWithVAKeysAndObjects( DKHashTableRef _self, va_list keysAndObjects );
DK_API DKObjectRef DKHashTableInitDictionaryWithDictionary( DKHashTableRef _self, DKDictionaryRef srcDictionary );

DK_API DKObjectRef DKHashTableInitSetWithVAObjects( DKHashTableRef _self, va_list objects );
DK_API DKObjectRef DKHashTableInitSetWithCArray( DKHashTableRef _self, DKObjectRef objects[], DKIndex count );
DK_API DKObjectRef DKHashTableInitSetWithCollection( DKHashTableRef _self, DKObjectRef srcCollection );

DK_API DKHashTableRef DKHashTableCopy( DKHashTableRef _self );
DK_API DKMutableHashTableRef DKHashTableMutableCopy( DKHashTableRef _self );

DK_API DKIndex     DKHashTableGetCount( DKHashTableRef _self );
DK_API DKObjectRef DKHashTableGetObject( DKHashTableRef _self, DKObjectRef key );

DK_API int         DKHashTableApplyFunction( DKHashTableRef _self, DKKeyedApplierFunction callback, void * context );
DK_API int         DKHashTableApplyFunctionToKeys( DKHashTableRef _self, DKApplierFunction callback, void * context );
DK_API int         DKHashTableApplyFunctionToObjects( DKHashTableRef _self, DKApplierFunction callback, void * context );

DK_API void        DKHashTableInsertObject( DKMutableHashTableRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
DK_API void        DKHashTableRemoveObject( DKMutableHashTableRef _self, DKObjectRef key );
DK_API void        DKHashTableRemoveAllObjects( DKMutableHashTableRef _self );

DK_API void        DKHashTableAddObjectToSet( DKMutableHashTableRef _self, DKObjectRef object );



#endif // _DK_HASHTABLE_H_

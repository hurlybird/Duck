// =======================================================================================
//
// DKHashTable.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_HASHTABLE_H_
#define _DK_HASHTABLE_H_

#ifdef __cplusplus
extern "C"
{
#endif


// typedef struct DKHashTable * DKHashTableRef;  -- Declared in DKPlatform.h
// typedef struct DKHashTable * DKMutableHashTableRef;  -- Declared in DKPlatform.h


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

DK_API void        DKHashTableReserve( DKMutableHashTableRef _self, size_t reserve );
DK_API void        DKHashTableInsertObject( DKMutableHashTableRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
DK_API void        DKHashTableRemoveObject( DKMutableHashTableRef _self, DKObjectRef key );
DK_API void        DKHashTableRemoveAllObjects( DKMutableHashTableRef _self );

DK_API void        DKHashTableAddObjectToSet( DKMutableHashTableRef _self, DKObjectRef object );


#ifdef __cplusplus
}
#endif

#endif // _DK_HASHTABLE_H_

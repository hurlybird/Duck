// =======================================================================================
//
// DKGenericHashTable.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_GENERIC_HASH_TABLE_H_
#define _DK_GENERIC_HASH_TABLE_H_

#ifdef __cplusplus
extern "C"
{
#endif


// Sentinels and related macros

// NOTE: A row is considered active if it is not empty and not deleted. The 'rowStatus'
// callback may therefore return DKRowStatusAcive or any other non-empty/non-deleted
// pointer value for active rows.

typedef void * DKRowStatus;

DK_API extern void * const DKRowStatusActive;
DK_API extern void * const DKRowStatusEmpty;
DK_API extern void * const DKRowStatusDeleted;


#define DKRowIsActive(x)        (((x) != DKRowStatusEmpty) && ((x) != DKRowStatusDeleted))
#define DKRowIsEmpty(x)         ((x) == DKRowStatusEmpty)
#define DKRowIsDeleted(x)       ((x) == DKRowStatusDeleted)

#define DKRowIsSentinel(x)      (((x) == DKRowStatusEmpty) || ((x) == DKRowStatusDeleted))
#define DKRowStatusString(x)    (const char *)(((x) == DKRowStatusEmpty) ? DKRowStatusEmpty : (((x) == DKRowStatusDeleted) ? DKRowStatusDeleted : DKRowStatusActive))



typedef struct
{
    DKRowStatus (*rowStatus)( const void * row, void * context );
    DKHashCode  (*rowHash)( const void * row, void * context );
    bool        (*rowEqual)( const void * row1, const void * row2, void * context );
    void        (*rowInit)( void * row, void * context );
    void        (*rowUpdate)( void * row, const void * src, void * context );
    void        (*rowDelete)( void * row, void * context );

} DKGenericHashTableCallbacks;


typedef struct
{
    uint8_t * rows;
    
    size_t activeCount;     // number of active rows in the table
    size_t deletedCount;    // number of deleted rows in the table
    
    size_t rowSize;         // row size in bytes
    size_t rowCount;        // total number of rows in the table
    size_t maxActive;       // maximum number of active rows
    
    DKGenericHashTableCallbacks callbacks;
    void * context;
    
} DKGenericHashTable;


DK_API void DKGenericHashTableInit( DKGenericHashTable * hashTable, size_t rowSize, const DKGenericHashTableCallbacks * callbacks, void * context );
DK_API void DKGenericHashTableFinalize( DKGenericHashTable * hashTable );

DK_API void DKGenericHashTableReserve( DKGenericHashTable * hashTable, size_t reserve );

#define DKGenericHashTableGetContext( table )     ((table)->context)
#define DKGenericHashTableGetCount( table )     ((table)->activeCount)
#define DKGenericHashTableGetRow( table, i )    (const void *)((table)->rows + ((table)->rowSize * i))
#define DKGenericHashTableGetRowCount( table )  ((DKIndex)((table)->rowCount))

DK_API const void * DKGenericHashTableFind( DKGenericHashTable * hashTable, const void * entry );
DK_API bool DKGenericHashTableInsert( DKGenericHashTable * hashTable, const void * entry, DKInsertPolicy policy );
DK_API void DKGenericHashTableRemove( DKGenericHashTable * hashTable, const void * entry );
DK_API void DKGenericHashTableRemoveAll( DKGenericHashTable * hashTable );

DK_API typedef void (*DKGenericHashTableForeachRowCallback)( const void * row, void * context );
DK_API void DKGenericHashTableForeachRow( DKGenericHashTable * hashTable, DKGenericHashTableForeachRowCallback callback, void * context );


#ifdef __cplusplus
}
#endif

#endif // _DK_GENERIC_HASH_TABLE_H_



// =======================================================================================
//
// DKDictionary.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_DICTIONARY_H_
#define _DK_DICTIONARY_H_

#ifdef __cplusplus
extern "C"
{
#endif


DK_API DKDeclareInterfaceSelector( Dictionary );


// typedef DKObjectRef DKDictionaryRef; -- Declared in DKPlatform.h
// typedef DKObjectRef DKMutableDictionaryRef;  -- Declared in DKPlatform.h


typedef DKObjectRef (*DKDictionaryInitWithVAKeysAndObjectsMethod)( DKDictionaryRef _self, va_list keysAndObjects );
typedef DKObjectRef (*DKDictionaryInitWithDictionaryMethod)( DKDictionaryRef _self, DKDictionaryRef srcDictionary );

typedef DKObjectRef (*DKDictionaryGetObjectMethod)( DKDictionaryRef _self, DKObjectRef key );

typedef void        (*DKDictionaryInsertObjectMethod)( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
typedef void        (*DKDictionaryRemoveObjectMethod)( DKMutableDictionaryRef _self, DKObjectRef key );
typedef void        (*DKDictionaryRemoveAllObjectsMethod)( DKMutableDictionaryRef _self );


struct DKDictionaryInterface
{
    const DKInterface _interface;

    DKDictionaryInitWithVAKeysAndObjectsMethod initWithVAKeysAndObjects;
    DKDictionaryInitWithDictionaryMethod       initWithDictionary;

    DKGetCountMethod                    getCount;
    DKDictionaryGetObjectMethod         getObject;
    
    // Mutable dictionaries -- these raise errors when called on immutable dictionaries
    DKDictionaryInsertObjectMethod      insertObject;
    DKDictionaryRemoveObjectMethod      removeObject;
    DKDictionaryRemoveAllObjectsMethod  removeAllObjects;
};

typedef const struct DKDictionaryInterface * DKDictionaryInterfaceRef;


DK_API DKClassRef  DKDictionaryClass( void );
DK_API void        DKSetDefaultDictionaryClass( DKClassRef _class );

DK_API DKClassRef  DKMutableDictionaryClass( void );
DK_API void        DKSetDefaultMutableDictionaryClass( DKClassRef _class );

#define            DKEmptyDictionary()         DKAutorelease( DKNew( DKDictionaryClass() ) )
#define            DKMutableDictionary()       DKAutorelease( DKNew( DKMutableDictionaryClass() ) )

#define            DKNewEmptyDictionary()      DKNew( DKDictionaryClass() )
#define            DKNewMutableDictionary()    DKNew( DKMutableDictionaryClass() )

#define            DKDictionaryWithKeysAndObjects( firstKey, ... )         DKAutorelease( DKDictionaryInitWithKeysAndObjects( DKAlloc( DKDictionaryClass() ), firstKey, __VA_ARGS__, NULL ) )
#define            DKDictionaryWithDictionary( srcDictionary )             DKAutorelease( DKDictionaryInitWithDictionary( DKAlloc( DKDictionaryClass() ), srcDictionary ) )

#define            DKNewDictionaryWithKeysAndObjects( firstKey, ... )      DKDictionaryInitWithKeysAndObjects( DKAlloc( DKDictionaryClass() ), firstKey, __VA_ARGS__, NULL )
#define            DKNewDictionaryWithDictionary( srcDictionary )          DKDictionaryInitWithDictionary( DKAlloc( DKDictionaryClass() ), srcDictionary )

#define            DKMutableDictionaryWithKeysAndObjects( firstKey, ... )  DKAutorelease( DKDictionaryInitWithKeysAndObjects( DKAlloc( DKMutableDictionaryClass() ), firstKey, __VA_ARGS__, NULL ) )
#define            DKMutableDictionaryWithDictionary( srcDictionary )      DKAutorelease( DKDictionaryInitWithDictionary( DKAlloc( DKMutableDictionaryClass() ), srcDictionary ) )

DK_API DKObjectRef DKDictionaryInitWithKeysAndObjects( DKDictionaryRef _self, ... );
DK_API DKObjectRef DKDictionaryInitWithDictionary( DKDictionaryRef _self, DKDictionaryRef srcDictionary );

DK_API DKIndex     DKDictionaryGetCount( DKDictionaryRef _self );
DK_API DKObjectRef DKDictionaryGetObject( DKDictionaryRef _self, DKObjectRef key );

DK_API int         DKDictionaryContainsKey( DKDictionaryRef _self, DKObjectRef key );
DK_API int         DKDictionaryContainsObject( DKDictionaryRef _self, DKObjectRef object );

DK_API DKListRef   DKDictionaryGetAllKeys( DKDictionaryRef _self );
DK_API DKListRef   DKDictionaryGetAllObjects( DKDictionaryRef _self );

DK_API void        DKDictionarySetObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
DK_API void        DKDictionaryAddObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
DK_API void        DKDictionaryReplaceObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
DK_API void        DKDictionaryInsertObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
DK_API void        DKDictionaryInsertEntriesFromDictionary( DKMutableDictionaryRef _self, DKDictionaryRef src, DKInsertPolicy policy );

DK_API void        DKDictionaryRemoveObject( DKMutableDictionaryRef _self, DKObjectRef key );
DK_API void        DKDictionaryRemoveAllObjects( DKMutableDictionaryRef _self );

DK_API bool        DKDictionaryEqual( DKDictionaryRef _self, DKDictionaryRef other );

DK_API bool        DKDictionaryIsSubsetOfDictionary( DKDictionaryRef _self, DKDictionaryRef other );


// Get/Set wrappers for base types
DK_API void        DKDictionarySetInt32( DKDictionaryRef _self, DKStringRef key, int32_t value );
DK_API int32_t     DKDictionaryGetInt32( DKDictionaryRef _self, DKStringRef key, int32_t defaultValue );

DK_API void        DKDictionarySetInt64( DKDictionaryRef _self, DKStringRef key, int64_t value );
DK_API int64_t     DKDictionaryGetInt64( DKDictionaryRef _self, DKStringRef key, int64_t defaultValue );

DK_API void        DKDictionarySetFloat( DKDictionaryRef _self, DKStringRef key, float value );
DK_API float       DKDictionaryGetFloat( DKDictionaryRef _self, DKStringRef key, float defaultValue );

DK_API void        DKDictionarySetDouble( DKDictionaryRef _self, DKStringRef key, double value );
DK_API double      DKDictionaryGetDouble( DKDictionaryRef _self, DKStringRef key, double defaultValue );


#ifdef __cplusplus
}
#endif

#endif // _DK_DICTIONARY_H_





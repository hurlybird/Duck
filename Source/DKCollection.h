// =======================================================================================
//
// DKCollection.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_COLLECTION_H_
#define _DK_COLLECTION_H_

#ifdef __cplusplus
extern "C"
{
#endif


DK_API DKDeclareInterfaceSelector( Collection );
DK_API DKDeclareInterfaceSelector( KeyedCollection );


typedef DKIndex (*DKGetCountMethod)( DKObjectRef _self );
typedef bool    (*DKContainsMethod)( DKObjectRef _self, DKObjectRef object );
typedef int     (*DKForeachObjectMethod)( DKObjectRef _self, DKApplierFunction callback, void * context );
typedef int     (*DKForeachKeyAndObjectMethod)( DKObjectRef _self, DKKeyedApplierFunction callback, void * context );

struct DKCollectionInterface
{
    const DKInterface _interface;

    DKGetCountMethod            getCount;
    DKContainsMethod            containsObject;
    DKForeachObjectMethod       foreachObject;
};

typedef const struct DKCollectionInterface * DKCollectionInterfaceRef;

struct DKKeyedCollectionInterface
{
    const DKInterface _interface;

    DKGetCountMethod            getCount;
    DKContainsMethod            containsObject;
    DKForeachObjectMethod       foreachObject;

    DKContainsMethod            containsKey;
    DKForeachObjectMethod       foreachKey;
    DKForeachKeyAndObjectMethod foreachKeyAndObject;
};

typedef const struct DKKeyedCollectionInterface * DKKeyedCollectionInterfaceRef;



DK_API DKIndex     DKGetCount( DKObjectRef _self );

DK_API DKObjectRef DKGetAnyKey( DKObjectRef _self );
DK_API DKObjectRef DKGetAnyObject( DKObjectRef _self );

DK_API bool        DKContainsKey( DKObjectRef _self, DKObjectRef key );
DK_API bool        DKContainsObject( DKObjectRef _self, DKObjectRef object );

DK_API int         DKForeachKey( DKObjectRef _self, DKApplierFunction callback, void * context );
DK_API int         DKForeachObject( DKObjectRef _self, DKApplierFunction callback, void * context );
DK_API int         DKForeachKeyAndObject( DKObjectRef _self, DKKeyedApplierFunction callback, void * context );

DK_API DKStringRef DKCollectionGetDescription( DKObjectRef _self );
DK_API DKStringRef DKKeyedCollectionGetDescription( DKObjectRef _self );

DK_API DKListRef   DKKeyedCollectionGetSortedEntries( DKObjectRef _self, DKCompareFunction cmp );


#ifdef __cplusplus
}
#endif

#endif // _DK_COLLECTION_H_




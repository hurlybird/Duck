// =======================================================================================
//
// DKJSON.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_JSON_H_
#define _DK_JSON_H_

#ifdef __cplusplus
extern "C"
{
#endif


enum
{
    DKJSONWritePretty =             (1 << 0),
    DKJSONWriteSorted =             (1 << 1),
    DKJSONVectorSyntaxExtension =   (1 << 2),
    DKJSONVectorRead32BitTypes =    (1 << 3),
    DKJSONObjectSerialization =     (1 << 4)
};


DK_API int DKJSONWrite( DKStreamRef stream, DKObjectRef object, int options );

DK_API DKObjectRef DKJSONParseEx( DKStringRef json, int options, DKStringRef * error );

#define DKJSONParse( json, options )    DKJSONParseEx( json, options, NULL )


// JSON Serialization
DK_API DKDeclareInterfaceSelector( JSONSerialization );

#define DKJSONSerializationClassNameKey DKSTR( "__class__" )

typedef DKObjectRef (*DKInitWithJSONObjectMethod)( DKObjectRef _self, DKDictionaryRef jsonObject );
typedef void (*DKWriteJSONObjectMethod)( DKObjectRef _self, DKMutableDictionaryRef jsonObject );

struct DKJSONSerializationInterface
{
    const DKInterface _interface;
    
    DKInitWithJSONObjectMethod  initWithJSONObject;
    DKWriteJSONObjectMethod     writeJSONObject;
};

typedef const struct DKJSONSerializationInterface * DKJSONSerializationInterfaceRef;



#ifdef __cplusplus
}
#endif

#endif // _DK_JSON_H_



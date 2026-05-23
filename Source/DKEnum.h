// =======================================================================================
//
// DKEnum.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_ENUM_H_
#define _DK_ENUM_H_

#ifdef __cplusplus
extern "C"
{
#endif


// typedef struct DKEnum * DKEnumRef; -- Declared in DKPlatform.h

DK_API DKClassRef DKEnumClass( void );

// NOTE: DKEnum stores its strings as DKConstantStrings which rely on external storage.
// Passing any transient string to the following functions and methods will result in a
// dangling pointer to that string.
DK_API DKObjectRef DKEnumInitWithCStringsAndValues( DKObjectRef _self, ... );
DK_API DKObjectRef DKEnumInitWithCStringsAndValues64( DKObjectRef _self, ... );

#define DKDefineEnum( accessor, ... )                                                   \
    DKThreadSafeSharedObjectInit( accessor, DKEnumRef )                                 \
    {                                                                                   \
        return DKEnumInitWithCStringsAndValues( DKAlloc( DKEnumClass() ),               \
            __VA_ARGS__,                                                                \
            NULL );                                                                     \
    }

#define DKDefineEnum64( accessor, ... )                                                 \
    DKThreadSafeSharedObjectInit( accessor, DKEnumRef )                                 \
    {                                                                                   \
        return DKEnumInitWithCStringsAndValues64( DKAlloc( DKEnumClass() ),             \
            __VA_ARGS__,                                                                \
            NULL );                                                                     \
    }

#define DKEnumFromString( _self, str )  ((int)DKEnumFromStringEx( (_self), (str), 0 ))

DK_API int64_t DKEnumFromStringEx( DKEnumRef _self, DKStringRef str, int64_t not_found );
DK_API DKStringRef DKStringFromEnum( DKEnumRef _self, int64_t value );

DK_API DKListRef DKEnumGetStrings( DKEnumRef _self, int sortOrder );


#ifdef __cplusplus
}
#endif

#endif // _DK_ENUM_H_



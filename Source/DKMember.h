// =======================================================================================
//
// DKMember.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_MEMBER_H_
#define _DK_MEMBER_H_

#ifdef __cplusplus
extern "C"
{
#endif


struct DKMember
{
    DKObject _obj;
    
    DKObjectRef object;
    size_t offset;
};

typedef struct DKMember * DKMemberRef;


DK_API DKClassRef  DKMemberClass( void );

#define DKMember( object, offset, encoding )     DKAutorelease( DKMemberInit( DKAlloc( DKMemberClass() ), object, offset, encoding ) )
#define DKNewMember( object, offset, encoding )  DKMemberInit( DKAlloc( DKMemberClass() ), object, offset, encoding )

DK_API DKObjectRef DKMemberInit( DKObjectRef _self, DKObjectRef object, size_t offset, DKEncoding encoding );

DK_API DKObjectRef DKMemberGetObject( DKMemberRef _self );
DK_API size_t      DKMemberGetOffset( DKMemberRef _self );
DK_API DKEncoding  DKMemberGetEncoding( DKMemberRef _self );

DK_API const void* DKMemberGetValuePtr( DKMemberRef _self );
DK_API const void* DKMemberQueryValuePtr( DKMemberRef _self, DKEncoding * encoding );


#ifdef __cplusplus
}
#endif

#endif // _DK_MEMBER_H_

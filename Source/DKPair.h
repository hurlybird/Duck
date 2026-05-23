// =======================================================================================
//
// DKPair.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_PAIR_H_
#define _DK_PAIR_H_

#ifdef __cplusplus
extern "C"
{
#endif


struct DKPair
{
    DKObject _obj;
    
    DKObjectRef first;
    DKObjectRef second;
};

//typedef struct DKPair * DKPairRef; -- Declared in DKPlatform.h


DK_API DKClassRef  DKPairClass( void );

#define            DKPair( first, second )         DKAutorelease( DKPairInit( DKAlloc( DKPairClass() ), first, second ) )
#define            DKNewPair( first, second )      DKPairInit( DKAlloc( DKPairClass() ), first, second )

DK_API DKObjectRef DKPairInit( DKObjectRef _self, DKObjectRef first, DKObjectRef second );

DK_API DKObjectRef DKPairGetFirstObject( DKPairRef _self );
DK_API DKObjectRef DKPairGetSecondObject( DKPairRef _self );


#ifdef __cplusplus
}
#endif

#endif // _DK_PAIR_H_

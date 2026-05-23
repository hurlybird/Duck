// =======================================================================================
//
// DKCondition.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_CONDITION_H_
#define _DK_CONDITION_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DKCondition * DKConditionRef;


DK_API DKClassRef DKConditionClass( void );

#define DKNewCondition()        DKNew( DKConditionClass() )


DK_API bool DKConditionWait( DKConditionRef _self, DKMutexRef mutex );
DK_API bool DKConditionTimedWait( DKConditionRef _self, DKMutexRef mutex, DKTimeInterval timeout );
DK_API void DKConditionSignal( DKConditionRef _self );
DK_API void DKConditionSignalAll( DKConditionRef _self );


#ifdef __cplusplus
}
#endif

#endif // _DK_CONDITION_H_


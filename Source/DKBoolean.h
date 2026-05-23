// =======================================================================================
//
// DKBoolean.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_BOOLEAN_H_
#define _DK_BOOLEAN_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define DKNumberBoolean DKEncodeIntegerType(bool)

typedef struct DKBoolean * DKBooleanRef;

DK_API DKClassRef   DKBooleanClass( void );

#define             DKBoolean( value ) ((value) ? DKTrue() : DKFalse())

DK_API DKBooleanRef DKTrue( void );
DK_API DKBooleanRef DKFalse( void );

#define             DKBooleanGetValue( b )     (((b) == DKTrue()) ? true : false)

DK_API DKStringRef  DKBooleanGetDescription( DKBooleanRef _self );


#ifdef __cplusplus
}
#endif

#endif // _DK_NUMBER_H_

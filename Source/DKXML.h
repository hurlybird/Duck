// =======================================================================================
//
// DKXML.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_XML_H_
#define _DK_XML_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DKXMLElement * DKXMLElementRef;


DK_API DKClassRef DKXMLElementClass( void );

DK_API DKStringRef DKXMLElementGetName( DKXMLElementRef _self );
DK_API DKDictionaryRef DKXMLElementGetAttributes( DKXMLElementRef _self );
DK_API DKListRef DKXMLElementGetElements( DKXMLElementRef _self );
DK_API DKStringRef DKXMLElementGetDescription( DKXMLElementRef _self );


DK_API DKXMLElementRef DKXMLParse( DKStringRef xml, int options );


#ifdef __cplusplus
}
#endif

#endif // _DK_XML_H_

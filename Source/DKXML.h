//
//  DKXML.h
//  Duck
//
//  Created by Derek Nylen on 2016-01-07.
//  Copyright © 2016 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_XML_H_
#define _DK_XML_H_

#include "DKPlatform.h"
#include "DKRuntime.h"



typedef struct DKXMLDocument * DKXMLDocumentRef;
typedef struct DKXMLElement * DKXMLElementRef;


DKClassRef DKXMLDocumentClass( void );

DKStringRef DKXMLDocumentGetXMLEncoding( DKXMLDocumentRef _self );
DKStringRef DKXMLDocumentGetDocType( DKXMLDocumentRef _self );
DKXMLElementRef DKXMLDocumentGetRootElement( DKXMLDocumentRef _self );



DKClassRef DKXMLElementClass( void );

DKStringRef DKXMLElementGetName( DKXMLElementRef _self );
DKDictionaryRef DKXMLElementGetAttributes( DKXMLElementRef _self );
DKListRef DKXMLElementGetElements( DKXMLElementRef _self );



DKXMLDocumentRef DKXMLParse( DKStringRef xml, int options );






#endif // _DK_XML_H_

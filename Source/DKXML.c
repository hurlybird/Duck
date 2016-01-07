//
//  DKXML.c
//  Duck
//
//  Created by Derek Nylen on 2016-01-07.
//  Copyright © 2016 Derek W. Nylen. All rights reserved.
//

#include "DKXML.h"
#include "DKDictionary.h"
#include "DKString.h"

#include "yxml/yxml.h"




// DKXMLDocument =========================================================================
struct DKXMLDocument
{
    DKObject _obj;
    
    DKStringRef xmlEncoding;
    DKStringRef docType;
    DKXMLElementRef rootElement;
};


static void DKXMLDocumentFinalize( DKObjectRef _untyped_self );


DKThreadSafeClassInit( DKXMLDocumentClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKXMLDocument" ), DKObjectClass(),
        sizeof(struct DKXMLDocument), 0, NULL, DKXMLDocumentFinalize );
    
    return cls;
}


///
//  DKXMLDocumentFinalize()
//
static void DKXMLDocumentFinalize( DKObjectRef _untyped_self )
{
    DKXMLDocumentRef _self = _untyped_self;
    
    DKRelease( _self->xmlEncoding );
    DKRelease( _self->docType );
    DKRelease( _self->rootElement );
}


///
//  DKXMLDocumentGetXMLEncoding()
//
DKStringRef DKXMLDocumentGetXMLEncoding( DKXMLDocumentRef _self )
{
    return _self->xmlEncoding;
}


///
//  DKXMLDocumentGetDocType()
//
DKStringRef DKXMLDocumentGetDocType( DKXMLDocumentRef _self )
{
    return _self->docType;
}


///
//  DKXMLDocumentGetRootElement()
//
DKXMLElementRef DKXMLDocumentGetRootElement( DKXMLDocumentRef _self )
{
    return _self->rootElement;
}




// DKXMLElement ==========================================================================
struct DKXMLElement
{
    DKObject _obj;
    
    DKStringRef name;
    DKMutableDictionaryRef attributes;
    DKMutableListRef elements;
};


static void DKXMLElementFinalize( DKObjectRef _untyped_self );


DKThreadSafeClassInit( DKXMLElementClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKXMLElement" ), DKObjectClass(),
        sizeof(struct DKXMLElement), 0, NULL, DKXMLElementFinalize );
    
    return cls;
}


///
//  DKXMLElementFinalize()
//
static void DKXMLElementFinalize( DKObjectRef _untyped_self )
{
    DKXMLElementRef _self = _untyped_self;
    
    DKRelease( _self->name );
    DKRelease( _self->attributes );
    DKRelease( _self->elements );
}


///
//  DKXMLElementGetName()
//
DKStringRef DKXMLElementGetName( DKXMLElementRef _self )
{
    return _self->name;
}


///
//  DKXMLElementGetAttributes()
//
DKDictionaryRef DKXMLElementGetAttributes( DKXMLElementRef _self )
{
    return _self->attributes;
}


///
//  DKXMLElementGetElements()
//
DKListRef DKXMLElementGetElements( DKXMLElementRef _self )
{
    return _self->elements;
}




// Parser ================================================================================

///
//  DKXMLParse_INTERNAL()
//
static DKXMLDocumentRef DKXMLParse_INTERNAL( DKStringRef xml, int options, size_t bufferSize )
{
    DKXMLDocumentRef doc = DKNew( DKXMLDocumentClass() );
    
    yxml_t * parser = dk_malloc( sizeof(yxml_t) + bufferSize );
    yxml_init( parser, parser + 1, bufferSize );
    const char * cursor = DKStringGetCStringPtr( xml );
    
    for( ; *cursor; cursor++ )
    {
        yxml_ret_t result = yxml_parse( parser, *cursor );

        switch( result )
        {
        case YXML_EEOF:         // Unexpected EOF
            DKError( "DKXMLParse: Unexpected end-of-file (line %ld, char %ld)\n", parser->line, parser->byte );
            dk_free( parser );
            DKRelease( doc );
            return NULL;
            
        case YXML_EREF:         // Invalid character or entity reference (&whatever;)
            DKError( "DKXMLParse: Invalid character or entity reference (line %ld, char %ld)\n", parser->line, parser->byte );
            dk_free( parser );
            DKRelease( doc );
            return NULL;
            
        case YXML_ECLOSE:       // Close tag does not match open tag (<Tag> .. </OtherTag>)
            DKError( "DKXMLParse: Closing tag does match opening tag (line %ld, char %ld)\n", parser->line, parser->byte );
            dk_free( parser );
            DKRelease( doc );
            return NULL;
            
        case YXML_ESTACK:       // Stack overflow (too deeply nested tags or too long element/attribute name)
            DKDebug( "DKXMLParse: Stack overflow, re-parsing with larger stack (%Invalid character or entity reference (line %ud, char %ud)\n", parser->line, parser->byte );
            dk_free( parser );
            DKRelease( doc );
            return DKXMLParse_INTERNAL( xml, options, bufferSize * 2 );
            
        case YXML_ESYN:         // Syntax error (unexpected byte)
            DKError( "DKXMLParse: Syntax error (line %ld, char %ld)\n", parser->line, parser->byte );
            dk_free( parser );
            DKRelease( doc );
            return NULL;
            
        case YXML_OK:           // Character consumed, no new token present
            break;
            
        case YXML_ELEMSTART:    // Start of an element:   '<Tag ..'
            break;
            
        case YXML_CONTENT:      // Element content
            break;
            
        case YXML_ELEMEND:      // End of an element:     '.. />' or '</Tag>'
            break;
            
        case YXML_ATTRSTART:    // Attribute:             'Name=..'
            break;
            
        case YXML_ATTRVAL:      // Attribute value
            break;
            
        case YXML_ATTREND:      // End of attribute       '.."'
            break;
            
        case YXML_PISTART:      // Start of a processing instruction
            break;
            
        case YXML_PICONTENT:    // Content of a PI
            break;
            
        case YXML_PIEND:        // End of a processing instruction
            break;
        }
    }
    
    if( yxml_eof( parser ) != YXML_OK )
    {
        DKError( "DKXMLParse: Incomplete document\n" );
        dk_free( parser );
        DKRelease( doc );
        return NULL;
    }
    
    dk_free( parser );
    
    return DKAutorelease( doc );
}


///
//  DKXMLParse()
//
DKXMLDocumentRef DKXMLParse( DKStringRef xml, int options )
{
    if( DKStringGetByteLength( xml ) == 0 )
        return NULL;

    return DKXMLParse_INTERNAL( xml, options, 16 * 1024 );
}








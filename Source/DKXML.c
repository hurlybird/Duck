/*****************************************************************************************

  DKXML.c

  Copyright (c) 2016 Derek W. Nylen

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*****************************************************************************************/

#include "DKXML.h"
#include "DKDescription.h"
#include "DKDictionary.h"
#include "DKString.h"
#include "DKStream.h"
#include "DKArray.h"
#include "DKUnicode.h"

#include "yxml/yxml.h"




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
    
    // Description
    struct DKDescriptionInterface * description = DKNewInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->getDescription = (DKGetDescriptionMethod)DKXMLElementGetDescription;
    description->getSizeInBytes = DKDefaultGetSizeInBytes;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

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


///
//  DKXMLElementGetDescription()
//
static void WriteIndent( DKMutableStringRef str, int indent )
{
    for( int i = 0; i < indent; i++ )
        DKSPrintf( str, "  " );
}

static int WriteAttribute( DKObjectRef key, DKObjectRef value, void * context )
{
    DKMutableStringRef str = context;
    DKSPrintf( str, " %@=\"%@\"", key, value );
    return 0;
}

static void INTERNAL_DKXMLElementGetDescription( DKXMLElementRef _self, DKMutableStringRef desc, int indent )
{
    WriteIndent( desc, indent );
    DKSPrintf( desc, "<%@", _self->name );
    DKForeachKeyAndObject( _self->attributes, WriteAttribute, desc );
    DKSPrintf( desc, ">\n" );
    
    DKIndex elementCount = DKListGetCount( _self->elements );
    
    for( DKIndex i = 0; i < elementCount; i++ )
    {
        DKObjectRef element = DKListGetObjectAtIndex( _self->elements, i );

        if( DKIsKindOfClass( element, DKXMLElementClass() ) )
        {
            INTERNAL_DKXMLElementGetDescription( element, desc, indent + 1 );
        }
           
        else if( DKIsKindOfClass( element, DKStringClass() ) )
        {
            WriteIndent( desc, indent + 1 );
            DKSPrintf( desc, "%@\n", element );
        }
    }
    
    WriteIndent( desc, indent );
    DKSPrintf( desc, "</%@>\n", _self->name );
}

DKStringRef DKXMLElementGetDescription( DKXMLElementRef _self )
{
    if( _self )
    {
        DKMutableStringRef desc = DKMutableString();
        INTERNAL_DKXMLElementGetDescription( _self, desc, 0 );
        return desc;
    }
    
    return NULL;
}


///
//  DKXMLAppendTextContent()
//
static void DKXMLAppendTextContent( DKXMLElementRef element, const char * ch )
{
    // Get the last element
    DKMutableStringRef text = DKListGetLastObject( element->elements );
                
    if( !DKIsKindOfClass( text, DKMutableStringClass() ) )
        text = NULL;

    // Skip leading whitespace
    bool ws = isspace( ch[0] );
    
    if( ws && !text )
        return;

    // Append a text element if we don't have one
    if( !text )
    {
        text = DKNewMutableString();
        DKListAppendObject( element->elements, text );
        DKRelease( text );
    }

    const char * str = DKStringGetCStringPtr( text );
    const char * last_ch = dk_ustrridx( str, 0 );

    // If the new character isn't whitespace, insert it
    if( !ws )
        DKStringAppendCString( text, ch );

    // If the last character wasn't whitespace, insert a space
    else if( last_ch && !isspace( last_ch[0] ) )
        DKStringAppendCString( text, " " );
}


///
//  INTERNAL_DKXMLParse()
//
static DKXMLElementRef INTERNAL_DKXMLParse( DKStringRef xml, int options, size_t bufferSize )
{
    DKXMLElementRef root = NULL;
    
    yxml_t * parser = dk_malloc( sizeof(yxml_t) + bufferSize );
    yxml_init( parser, parser + 1, bufferSize );
    const char * cursor = DKStringGetCStringPtr( xml );
    
    DKMutableListRef stack = DKNewMutableList();
    DKMutableStringRef attrValue = NULL;
    yxml_ret_t parseResult = 0;
    
    for( ; *cursor; cursor++ )
    {
        parseResult = yxml_parse( parser, *cursor );

        switch( parseResult )
        {
        case YXML_EEOF:         // Unexpected EOF
            DKError( "DKXMLParse: Unexpected end-of-file (line %ld, char %ld)\n", parser->line, parser->byte );
            goto error;
            
        case YXML_EREF:         // Invalid character or entity reference (&whatever;)
            DKError( "DKXMLParse: Invalid character or entity reference (line %ld, char %ld)\n", parser->line, parser->byte );
            goto error;
            
        case YXML_ECLOSE:       // Close tag does not match open tag (<Tag> .. </OtherTag>)
            DKError( "DKXMLParse: Closing tag does match opening tag (line %ld, char %ld)\n", parser->line, parser->byte );
            goto error;
            
        case YXML_ESTACK:       // Stack overflow (too deeply nested tags or too long element/attribute name)
            DKDebug( "DKXMLParse: Stack overflow, re-parsing with larger stack (%Invalid character or entity reference (line %ud, char %ud)\n", parser->line, parser->byte );
            goto error;
            
        case YXML_ESYN:         // Syntax error (unexpected byte)
            DKError( "DKXMLParse: Syntax error (line %ld, char %ld)\n", parser->line, parser->byte );
            goto error;
            
        case YXML_OK:           // Character consumed, no new token present
            break;
            
        case YXML_ELEMSTART:    // Start of an element:   '<Tag ..'
            {
                DKXMLElementRef element = DKNew( DKXMLElementClass() );
                element->name = DKNewStringWithCString( parser->elem );
                element->attributes = DKNewMutableDictionary();
                element->elements = DKNewMutableList();
                
                if( !root )
                {
                    DKAssert( DKListGetCount( stack ) == 0 );
                    root = element;
                }
                
                else
                {
                    DKAssert( DKListGetCount( stack ) > 0 );
                    DKXMLElementRef parent = DKListGetLastObject( stack );
                    DKListAppendObject( parent->elements, element );
                    DKRelease( element );
                }

                DKListAppendObject( stack, element );
            }
            break;
            
        case YXML_CONTENT:      // Element content
            {
                DKXMLElementRef parent = DKListGetLastObject( stack );
                DKXMLAppendTextContent( parent, parser->data );
            }
            break;
            
        case YXML_ELEMEND:      // End of an element:     '.. />' or '</Tag>'
            DKListRemoveLastObject( stack );
            break;
            
        case YXML_ATTRSTART:    // Attribute:             'Name=..'
            attrValue = DKNewMutableString();
            break;
            
        case YXML_ATTRVAL:      // Attribute value
            DKStringAppendCString( attrValue, parser->data );
            break;
            
        case YXML_ATTREND:      // End of attribute       '.."'
            {
                DKXMLElementRef parent = DKListGetLastObject( stack );
                DKStringRef attrName = DKNewStringWithCString( parser->attr );
                DKDictionarySetObject( parent->attributes, attrName, attrValue );
                DKRelease( attrName );
                DKRelease( attrValue );
                attrValue = NULL;
            }
            break;
            
        case YXML_PISTART:      // Start of a processing instruction
            break;
            
        case YXML_PICONTENT:    // Content of a PI
            break;
            
        case YXML_PIEND:        // End of a processing instruction
            break;
        }
    }
    
    if( yxml_eof( parser ) == YXML_OK )
    {
        dk_free( parser );
        DKRelease( stack );
        DKRelease( attrValue );
        
        return DKAutorelease( root );
    }

    DKError( "DKXMLParse: Incomplete document\n" );
    
error:
    dk_free( parser );
    DKRelease( root );
    DKRelease( stack );
    DKRelease( attrValue );
    
    if( parseResult == YXML_ESTACK )
        return INTERNAL_DKXMLParse( xml, options, bufferSize * 2 );
    
    return NULL;
}


///
//  DKXMLParse()
//
DKXMLElementRef DKXMLParse( DKStringRef xml, int options )
{
    if( DKStringGetByteLength( xml ) == 0 )
        return NULL;

    return INTERNAL_DKXMLParse( xml, options, 16 * 1024 );
}








/*****************************************************************************************

  DKShell.c

  Copyright (c) 2017 Derek W. Nylen

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

#include "DKShell.h"
#include "DKData.h"
#include "DKStream.h"
#include "DKString.h"
#include "DKList.h"
#include "DKBuffer.h"

#include "DKEgg.h"
#include "DKJSON.h"
#include "DKXML.h"


#define DKShellHeaderString    "SHELL-Version 1.0"


///
//  EncodeEgg()
//
static DKObjectRef EncodeEgg( DKObjectRef object )
{
    DKEggArchiverRef archiver = DKNewEggArchiverWithObject( object );
    
    DKDataRef data = DKEggArchiverCopyData( archiver );
    
    DKRelease( archiver );
    
    return DKAutorelease( data );
}


///
//  DecodeEgg()
//
static DKObjectRef DecodeEgg( DKObjectRef data )
{
    DKEggUnarchiverRef unarchiver = DKNewEggUnarchiverWithData( data );
    
    DKObjectRef object = DKRetain( DKEggGetRootObject( unarchiver ) );
    
    DKRelease( unarchiver );
    
    return DKAutorelease( object );
}


///
//  EncodeJSON()
//
static DKObjectRef EncodeJSON( DKObjectRef object )
{
    DKMutableStringRef json = DKMutableString();
    
    DKJSONWrite( json, object, DKJSONVectorSyntaxExtension );
    
    return json;
}


///
//  DecodeJSON()
//
static DKObjectRef DecodeJSON( DKObjectRef json )
{
    return DKJSONParse( json, DKJSONVectorSyntaxExtension );
}


///
//  DecodeXML()
//
static DKObjectRef DecodeXML( DKObjectRef xml )
{
    return DKXMLParse( xml, 0 );
}


///
//  GetHeaderValue()
//
static const char * GetHeaderValue( const char * s, int skip )
{
    const char * value = s + skip;
    
    while( isspace( *value ) )
        value++;
    
    return value;
}


///
//  DKShellRead()
//
int DKShellRead( DKStreamRef stream, DKObjectRef * outObject, DKStringRef * outContentType, DKStringRef * outAnnotation, int options )
{
    *outObject = NULL;
    *outContentType = NULL;
    *outAnnotation = NULL;

    // Read the shell prefix
    DKStringRef prefix = DKGets( stream );
    
    // EOF?
    if( DKStringGetLength( prefix ) == 0 )
    {
        return 0;
    }
    
    // Invalid prefix?
    if( strcmp( DKStringGetCStringPtr( prefix ), DKShellHeaderString ) )
    {
        DKError( "DKShellRead: The stream does not contain a valid DKShell segment.\n" );
        return 0;
    }
    
    // Read the headers
    const char * contentType = NULL;
    size_t contentLength = 0;
    
    while( true )
    {
        DKStringRef header = DKGets( stream );
        const char * headerString = DKStringGetCStringPtr( header );
        
        if( *headerString == '\0' )
            break;
        
        if( !strncmp( headerString, "Content-Type:", 13 ) )
        {
            contentType = GetHeaderValue( headerString, 13 );
            *outContentType = DKStringWithCString( contentType );
        }
        
        else if( !strncmp( headerString, "Content-Length:", 15 ) )
        {
            const char * value = GetHeaderValue( headerString, 15 );
            contentLength = strtoul( value, NULL, 10 );
        }
    }
    
    // Read the annotation
    DKStringRef annotation = DKGets( stream );
    
    if( DKStringGetLength( annotation ) > 0 )
    {
        if( DKGetc( stream ) != '\n' )
        {
            DKError( "DKShellRead: The segment annotation was not property terminated.\n" );
            return 0;
        }

        *outAnnotation = annotation;
    }
    
    // Read the data
    if( contentLength == 0 )
        return 1;
    
    DKObjectRef object;
    
    if( strncmp( contentType, "text", 4 ) == 0 )
        object = DKMutableString();

    else // if( strncmp( contentType, "binary", 6 ) == 0 )
        object = DKMutableData();
    
    DKBufferSetLength( object, contentLength );

    char * bufferPtr = DKBufferGetMutableBytePtr( object, 0 );

    if( DKRead( stream, bufferPtr, 1, contentLength ) != contentLength )
    {
        DKError( "DKShellRead: Error reading content data (expected &zu bytes).\n", contentLength );
        return 0;
    }
    
    // EGG Decoding
    if( (options & DKShellDecodeEgg) && DKStringEqualToString( *outContentType, DKShellContentTypeEgg ) )
        object = DecodeEgg( object );
    
    // JSON Decoding
    else if( (options & DKShellEncodeJSON) && DKStringEqualToString( *outContentType, DKShellContentTypeJSON ) )
        object = DecodeJSON( object );

    // XML Decoding
    else if( (options & DKShellEncodeJSON) && DKStringEqualToString( *outContentType, DKShellContentTypeXML ) )
        object = DecodeXML( object );

    *outObject = object;

    return 1;
}


///
//  DKShellWrite()
//
int DKShellWrite( DKStreamRef stream, DKObjectRef object, DKStringRef contentType, DKStringRef annotation, int options )
{
    DKObjectRef encodedObject = object;

    // Check the annonation for '\r' and '\n'
    const char * annotationString = DKStringGetCStringPtr( annotation );
    
    if( strpbrk( annotationString, "\r\n" ) )
    {
        DKError( "DKShellWrite: Annotations cannot contain carriage-return ('\\r') or line-feed ('\\n') characters." );
        return 0;
    }

    // EGG Encoding
    if( (options & DKShellEncodeEgg) && DKStringEqualToString( contentType, DKShellContentTypeEgg ) )
        encodedObject = EncodeEgg( object );
    
    // JSON Encoding
    else if( (options & DKShellEncodeJSON) && DKStringEqualToString( contentType, DKShellContentTypeJSON ) )
        encodedObject = EncodeJSON( object );

    // Make sure we can write the data
    if( !DKQueryInterface( encodedObject, DKSelector(Buffer), NULL ) )
    {
        DKError( "DKShellWrite: Class '%@ cannot be written (supported types must implement the 'Buffer' interface).", DKGetClassName( object ) );
        return 0;
    }

    size_t contentLength = DKBufferGetLength( encodedObject );
    
    // Write the header
    DKSPrintf( stream, "%s\n", DKShellHeaderString );
    DKSPrintf( stream, "Content-Type: %s\n", DKStringGetCStringPtr( contentType ) );
    DKSPrintf( stream, "Content-Length: %zu\n", contentLength );
    DKPutc( stream, '\n' );
    
    // Write the annotation
    if( *annotationString )
    {
        DKSPrintf( stream, "%s\n", annotationString );
        DKPutc( stream, '\n' );
    }
    
    // Write the data
    const void * bufferPtr = DKBufferGetBytePtr( encodedObject, 0 );

    if( DKWrite( stream, bufferPtr, 1, contentLength ) == contentLength )
        return 1;
    
    return 0;
}




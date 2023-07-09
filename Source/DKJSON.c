/*****************************************************************************************

  DKJSON.c

  Copyright (c) 2014 Derek W. Nylen

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

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKStream.h"
#include "DKJSON.h"
#include "DKCollection.h"
#include "DKList.h"
#include "DKDictionary.h"
#include "DKString.h"
#include "DKNumber.h"
#include "DKBoolean.h"
#include "DKArray.h"
#include "DKHashTable.h"
#include "DKCopying.h"
#include "DKEncoding.h"
#include "DKUnicode.h"
#include "DKGenericArray.h"


// Writer ================================================================================

typedef struct
{
    DKStreamRef stream;
    int options;
    int indent;
    int comma;
    
} WriteContext;

static int WriteObject( DKObjectRef obj, WriteContext * context );
static int WriteKeyAndObject( DKObjectRef key, DKObjectRef obj, WriteContext * context );

static void WriteEscapedString( DKStringRef str, WriteContext * context );
static void WriteComma( WriteContext * context );
static void BeginGroup( WriteContext * context, char delimiter );
static void EndGroup( WriteContext * context, char delimiter );


///
//  DKJSONWrite()
//

int DKJSONWrite( DKStreamRef stream, DKObjectRef object, int options )
{
    WriteContext context;
    context.stream = stream;
    context.options = options;
    context.indent = 0;
    context.comma = 0;
    
    int result = WriteObject( object, &context );

    if( options & DKJSONWritePretty )
        DKSPrintf( stream, "\n" );
    
    return result;
}


///
//  WriteObject()
//
static int WriteObject( DKObjectRef obj, WriteContext * context )
{
    int result = 0;

    DKCollectionInterfaceRef collection = NULL;
    DKKeyedCollectionInterfaceRef keyedCollection = NULL;

    WriteComma( context );
    
    if( obj == NULL )
    {
        DKSPrintf( context->stream, "null" );
    }
    
    else if( DKIsKindOfClass( obj, DKStringClass() ) )
    {
        WriteEscapedString( obj, context );
    }
    
    else if( DKIsKindOfClass( obj, DKNumberClass() ) )
    {
        DKEncoding encoding = DKNumberGetEncoding( obj );
        
        if( DKEncodingGetCount( encoding ) == 1 )
        {
            DKSPrintf( context->stream, "%@", obj );
        }
        
        else if( context->options & DKJSONVectorSyntaxExtension )
        {
            DKSPrintf( context->stream, "#[%@]", DKNumberFormatString( obj, ", " ) );
        }
        
        else
        {
            DKWarning( "DKJSON: Only scalar number encodings are supported. Writing vector as string." );
            DKSPrintf( context->stream, "\"%@\"", obj );
        }
    }
    
    else if( DKQueryInterface( obj, DKSelector(KeyedCollection), (DKInterfaceRef *)&keyedCollection ) )
    {
        BeginGroup( context, '{' );
        result = DKForeachKeyAndObject( obj, (DKKeyedApplierFunction)WriteKeyAndObject, context );
        EndGroup( context, '}' );
    }
    
    else if( DKQueryInterface( obj, DKSelector(Collection), (DKInterfaceRef *)&collection ) )
    {
        BeginGroup( context, '[' );
        result = DKForeachObject( obj, (DKApplierFunction)WriteObject, context );
        EndGroup( context, ']' );
    }

    context->comma = 1;
    
    return result;
}


///
//  WriteKeyAndObject()
//

static int WriteKeyAndObject( DKObjectRef key, DKObjectRef obj, WriteContext * context )
{
    WriteComma( context );
    
    const char * fmt = (context->options & DKJSONWritePretty) ? "\"%@\" : " : "\"%@\":";
    DKSPrintf( context->stream, fmt, key );

    context->comma = 0;
    WriteObject( obj, context );
    context->comma = 1;
    
    return 0;
}


///
//  WriteEscapedString()
//
typedef struct
{
    int pattern;
    const char * replacement;

} DKEscapedChar;

// The escaped characters are listed in descending order for fast rejection
static const DKEscapedChar EscapedChars[] =
{
    { '\\', "\\\\" },   // 92
    //{ '/', "\\/" },   // 47
    { '\"', "\\\"" },   // 34
    { '\r', "\\r" },    // 13
    { '\f', "\\f" },    // 12
    { '\n', "\\n" },    // 10
    { '\t', "\\t" },    // 09
    { '\b', "\\b" },    // 08
    { 0, NULL },
};

static void WriteEscapedString( DKStringRef str, WriteContext * context )
{
    DKStreamInterfaceRef stream = DKGetInterface( context->stream, DKSelector(Stream) );

    stream->write( context->stream, "\"", 1, 1 );
    
    const char * cursor = DKStringGetCStringPtr( str );
    const char * bufferStart = cursor;
    size_t bufferLength = 0;
    DKChar32 ch;
    size_t n;
    
    while( (n = dk_ustrscan( cursor, &ch )) != 0 )
    {
        const DKEscapedChar * ec = &EscapedChars[0];
        
        while( 1 )
        {
            if( (ec->pattern < ch) || (ec->replacement == NULL) )
            {
                bufferLength += n;
                break;
            }
        
            if( ec->pattern == ch )
            {
                if( bufferLength > 0 )
                    stream->write( context->stream, bufferStart, 1, bufferLength );

                DKSPrintf( context->stream, "%s", ec->replacement );

                bufferStart = cursor + n;
                bufferLength = 0;
                break;
            }
            
            ec++;
        }
        
        cursor += n;
    }

    if( bufferLength > 0 )
        stream->write( context->stream, bufferStart, 1, bufferLength );

    DKWrite( context->stream, "\"", 1, 1 );
}


///
//  WriteComma()
//
static void WriteComma( WriteContext * context )
{
    if( context->comma )
    {
        if( context->options & DKJSONWritePretty )
        {
            DKSPrintf( context->stream, ",\n" );

            for( int i = 0; i < context->indent; i++ )
                DKSPrintf( context->stream, "    " );
        }
        
        else
        {
            DKSPrintf( context->stream, "," );
        }
    }
}


///
//  BeginGroup()
//
static void BeginGroup( WriteContext * context, char delimiter )
{
    if( context->options & DKJSONWritePretty )
    {
        DKSPrintf( context->stream, "%c\n", delimiter );

        context->indent++;

        for( int i = 0; i < context->indent; i++ )
            DKSPrintf( context->stream, "    " );
    }
    
    else
    {
        DKSPrintf( context->stream, "%c", delimiter );
    }

    context->comma = 0;
}


///
//  EndGroup()
//
static void EndGroup( WriteContext * context, char delimiter )
{
    if( context->options & DKJSONWritePretty )
    {
        DKSPrintf( context->stream, "\n" );

        context->indent--;

        for( int i = 0; i < context->indent; i++ )
            DKSPrintf( context->stream, "    " );
    }
    
    DKSPrintf( context->stream, "%c", delimiter );
}




// Parser ================================================================================

typedef struct
{
    const char * start;
    const char * cursor;
    int options;
    
} ParseContext;

typedef struct
{
    const char * str;
    size_t length;

} Token;

static int ParseObject( ParseContext * context, DKObjectRef * obj );
static Token ScanToken( ParseContext * context, DKObjectRef * obj );


///
//  DKJSONParse()
//
DKObjectRef DKJSONParse( DKStringRef json, int options )
{
    ParseContext context;
    context.start = DKStringGetCStringPtr( json );
    context.cursor = context.start;
    context.options = options;
    
    DKObjectRef obj = NULL;
    
    if( ParseObject( &context, &obj ) )
    {
        DKRelease( obj );
    
        return NULL;
    }

    return DKAutorelease( obj );
}


///
//  ParseObject()
//
static int ParseObject( ParseContext * context, DKObjectRef * obj )
{
    DKObjectRef objToken = NULL;
    Token token = ScanToken( context, &objToken );
    
    if( token.length == 0 )
        return -1;
    
    char ch = *token.str;
    DKObjectRef key = NULL;
    DKObjectRef value = NULL;
    int result;

    // Object (i.e. string, vector, boolean)
    if( objToken != NULL )
    {
        *obj = objToken;
        return 0;
    }

    // Array
    else if( ch == '[' )
    {
        *obj = DKNewMutableList();
    
        while( true )
        {
            // Parse a value
            result = ParseObject( context, &value );
            
            if( result == ']' )
                return 0;
            
            else if( result != 0 )
                break;

            // Add it to the list
            DKListAppendObject( *obj, value );
            
            DKRelease( value );
            value = NULL;
            
            // Parse a comma
            token = ScanToken( context, NULL );
            ch = *token.str;
            
            if( ch == ']' )
                return 0;
            
            if( ch != ',' )
                break;
        }
    }
    
    else if( ch == ']' )
    {
        *obj = NULL;
        return ']';
    }

    // Object
    else if( ch == '{' )
    {
        *obj = DKNewMutableDictionary();
    
        while( true )
        {
            // Parse a key
            result = ParseObject( context, &key );
            
            if( result == '}' )
                return 0;
            
            else if( result != 0 )
                break;

            else if( !DKIsKindOfClass( key, DKStringClass() ) )
                break;

            // Parse a colon
            token = ScanToken( context, NULL );
            ch = *token.str;
            
            if( ch != ':' )
                break;
        
            // Parse a value
            result = ParseObject( context, &value );
            
            if( result != 0 )
                break;
            
            // Add it to the dictionary
            DKDictionarySetObject( *obj, key, value );

            DKRelease( key );
            key = NULL;

            DKRelease( value );
            value = NULL;
            
            // Parse a comma
            token = ScanToken( context, NULL );
            ch = *token.str;
            
            if( ch == '}' )
                return 0;
            
            if( ch != ',' )
                break;
        }
    }
    
    else if( ch == '}' )
    {
        *obj = NULL;
        return '}';
    }

    // Number
    else if( (ch == '-') || isdigit( ch ) )
    {
        int64_t ival;
        double fval;
        
        if( dk_strtonum( token.str, &ival, &fval, NULL ) )
        {
            *obj = DKNewNumberWithInt64( ival );
            return 0;
        }
        
        else
        {
            *obj = DKNewNumberWithDouble( fval );
            return 0;
        }
    }
    
    // True (should be handled by ScanToken)
    else if( strncmp( token.str, "true", 4 ) == 0 )
    {
        *obj = DKTrue();
        return 0;
    }

    // False (should be handled by ScanToken)
    else if( strncmp( token.str, "false", 5 ) == 0 )
    {
        *obj = DKFalse();
        return 0;
    }

    // Null
    else if( strncmp( token.str, "null", 4 ) == 0 )
    {
        *obj = NULL;
        return 0;
    }
    
    DKRelease( *obj );
    DKRelease( key );
    DKRelease( value );

    *obj = NULL;
    return -1;
}


///
//  ScanUnicodeCodePoint()
//
static size_t ScanUnicodeCodePoint( const char * str, DKMutableStringRef buffer )
{
    char hex[8];
    char utf[8];
    char * end;
    
    hex[0] = str[0];
    hex[1] = str[1];
    hex[2] = str[2];
    hex[3] = str[3];
    hex[4] = '\0';
    
    DKChar32 ch = (DKChar32)strtol( hex, &end, 16 );
    dk_ustrwrite( ch, utf, 8 );

    DKStringAppendCString( buffer, utf );

    return 4;
}


///
//  ScanStringToken()
//
static Token ScanStringToken( Token token, ParseContext * context, DKObjectRef * obj )
{
    DKChar32 ch;
    DKMutableStringRef buffer = NULL;
    
    if( obj != NULL )
    {
        buffer = DKNewMutableString();
        *obj = buffer;
    }

    while( true )
    {
        size_t n = dk_ustrscan( token.str + token.length, &ch );
        
        if( n == 0 )
        {
            context->cursor = token.str + token.length;
            return token;
        }
        
        if( ch == '\\' )
        {
            token.length += n;
            n = dk_ustrscan( token.str + token.length, &ch );
            
            switch( ch )
            {
            case '\\':
                DKStringAppendCString( buffer, "\\" );
                break;
                
            case '"':
                DKStringAppendCString( buffer, "\"" );
                break;
                
            case 'b':
                DKStringAppendCString( buffer, "\b" );
                break;
                
            case 'f':
                DKStringAppendCString( buffer, "\f" );
                break;
                
            case 'n':
                DKStringAppendCString( buffer, "\n" );
                break;
                
            case 'r':
                DKStringAppendCString( buffer, "\r" );
                break;
                
            case 't':
                DKStringAppendCString( buffer, "\t" );
                break;
                
            case '/':
                DKStringAppendCString( buffer, "/" );
                break;
                    
            case 'u':
                token.length += ScanUnicodeCodePoint( token.str + token.length + n, buffer );
                break;
                
            default:
                DKWarning( "DKJSON: Invalid control character: %c (%d)", ch, ch );
                break;
            }
        }

        else if( ch == '"' )
        {
            token.length += n;
            
            context->cursor = token.str + token.length;
            return token;
        }
        
        else
        {
            DKStringWrite( buffer, token.str + token.length, 1, n );
        }

        token.length += n;
    }
}


///
//  ScanVectorToken64()
//
static Token ScanVectorToken64( Token token, ParseContext * context, DKObjectRef * obj )
{
    DKChar32 ch;
    size_t n = dk_ustrscan( token.str + token.length, &ch );
    
    if( ch == '[' )
    {
        token.length += n;
        context->cursor = token.str + token.length;
        
        DKGenericArray buffer;
        DKGenericArrayInit( &buffer, sizeof(int64_t) );
        DKGenericArrayReserve( &buffer, 32 );
        
        DKEncodingType numberType = DKEncodingTypeInt64;
        
        while( true )
        {
            Token numberToken = ScanToken( context, NULL );
            ch = *numberToken.str;

            if( (ch == '-') || isdigit( ch ) )
            {
                int64_t ival;
                double fval;
                
                if( dk_strtonum( numberToken.str, &ival, &fval, NULL ) )
                {
                    if( numberType == DKEncodingTypeInt64 )
                    {
                        DKGenericArrayAppendElements( &buffer, &ival, 1 );
                    }
                    
                    else
                    {
                        double x = (double)ival;
                        DKGenericArrayAppendElements( &buffer, &x, 1 );
                    }
                }
                
                else
                {
                    // Convert previously read integers to doubles
                    if( numberType != DKEncodingTypeDouble )
                    {
                        numberType = DKEncodingTypeDouble;
                    
                        size_t len = DKGenericArrayGetLength( &buffer );
                        
                        if( len > 0 )
                        {
                            int64_t * src = DKGenericArrayGetPointerToElementAtIndex( &buffer, 0 );
                            double * dst = (double *)src;
                            
                            for( size_t i = 0; i < len; ++i )
                                dst[i] = (double)src[i];
                        }
                    }
                
                    DKGenericArrayAppendElements( &buffer, &fval, 1 );
                }
            }
            
            else
            {
                break;
            }
            
            numberToken = ScanToken( context, NULL );
            ch = *numberToken.str;
            
            if( ch == ',' )
            {
                // Continue parsing
            }
            
            else if( ch == ']' )
            {
                if( obj )
                {
                    void * value = DKGenericArrayGetPointerToElementAtIndex( &buffer, 0 );
                    int count = (int)DKGenericArrayGetLength( &buffer );
                    
                    *obj = DKNewNumber( value, DKEncode( numberType, count ) );
                }
                
                break;
            }
            
            else
            {
                break;
            }
        }
        
        DKGenericArrayFinalize( &buffer );
    }

    token.length = context->cursor - token.str;
    
    return token;
}


///
//  ScanVectorToken32()
//
static Token ScanVectorToken32( Token token, ParseContext * context, DKObjectRef * obj )
{
    DKChar32 ch;
    size_t n = dk_ustrscan( token.str + token.length, &ch );
    
    if( ch == '[' )
    {
        token.length += n;
        context->cursor = token.str + token.length;
        
        DKGenericArray buffer;
        DKGenericArrayInit( &buffer, sizeof(int32_t) );
        DKGenericArrayReserve( &buffer, 32 );
        
        DKEncodingType numberType = DKEncodingTypeInt32;
        
        while( true )
        {
            Token numberToken = ScanToken( context, NULL );
            ch = *numberToken.str;

            if( (ch == '-') || isdigit( ch ) )
            {
                int64_t ival;
                double fval;
                
                if( dk_strtonum( numberToken.str, &ival, &fval, NULL ) )
                {
                    if( numberType == DKEncodingTypeInt32 )
                    {
                        int32_t x = (int32_t)ival;
                        DKGenericArrayAppendElements( &buffer, &x, 1 );
                    }
                    
                    else
                    {
                        float x = (float)ival;
                        DKGenericArrayAppendElements( &buffer, &x, 1 );
                    }
                }
                
                else
                {
                    // Convert previously read integers to floats
                    if( numberType != DKEncodingTypeFloat )
                    {
                        numberType = DKEncodingTypeFloat;
                    
                        size_t len = DKGenericArrayGetLength( &buffer );
                        
                        if( len > 0 )
                        {
                            int32_t * src = DKGenericArrayGetPointerToElementAtIndex( &buffer, 0 );
                            float * dst = (float *)src;
                            
                            for( size_t i = 0; i < len; ++i )
                                dst[i] = (float)src[i];
                        }
                    }
                
                    float x = (float)fval;
                    DKGenericArrayAppendElements( &buffer, &x, 1 );
                }
            }
            
            else
            {
                break;
            }
            
            numberToken = ScanToken( context, NULL );
            ch = *numberToken.str;
            
            if( ch == ',' )
            {
                // Continue parsing
            }
            
            else if( ch == ']' )
            {
                if( obj )
                {
                    void * value = DKGenericArrayGetPointerToElementAtIndex( &buffer, 0 );
                    int count = (int)DKGenericArrayGetLength( &buffer );
                    
                    *obj = DKNewNumber( value, DKEncode( numberType, count ) );
                }
                
                break;
            }
            
            else
            {
                break;
            }
        }
        
        DKGenericArrayFinalize( &buffer );
    }

    token.length = context->cursor - token.str;
    
    return token;
}


///
//  ScanToken()
//
static Token ScanToken( ParseContext * context, DKObjectRef * obj )
{
    Token token;
    token.str = context->cursor;
    token.length = 0;

    DKChar32 ch;

    // Scan first character and skip whitespace
    while( true )
    {
        size_t n = dk_ustrscan( token.str, &ch );
        
        if( n == 0)
        {
            context->cursor = token.str + token.length;
            return token;
        }
        
        if( !isspace( ch ) )
        {
            token.length = n;
            break;
        }
        
        token.str += n;
    }
    
    // Scan to the end of a string
    if( ch == '"' )
    {
        return ScanStringToken( token, context, obj );
    }
    
    // Scan to the end of a vector
    else if( (ch == '#') && (context->options & DKJSONVectorSyntaxExtension) )
    {
        if( context->options & DKJSONVectorRead32BitTypes )
            return ScanVectorToken32( token, context, obj );

        else
            return ScanVectorToken64( token, context, obj );
    }
    
    // Scan to the end of other tokens
    else
    {
        // Special characters
        static const char * tokens = ",:[]{}";

        // Check for a single-character token
        if( strchr( tokens, *token.str ) )
        {
            context->cursor = token.str + token.length;
            return token;
        }

        while( true )
        {
            // Scan the next character
            size_t n = dk_ustrscan( token.str + token.length, &ch );
            
            // End of the token? (EOF, space or special character)
            if( (n == 0) || isspace( ch ) || strchr( tokens, ch ) )
            {
                context->cursor = token.str + token.length;
                return token;
            }
            
            token.length += n;
        }
    }
}

#if 0 // Test scanning
static Token ScanToken( ParseContext * context, DKObjectRef * obj )
{
    Token token = _ScanToken( context, obj );
    
    char tmp[128];
    strncpy( tmp, token.str, token.length );
    tmp[token.length] = '\0';
    
    printf( "ScanToken: %s\n", tmp );
    
    return token;
}
#endif





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

#include "DKJSON.h"
#include "DKString.h"
#include "DKNumber.h"
#include "DKBoolean.h"
#include "DKArray.h"
#include "DKHashTable.h"
#include "DKStream.h"
#include "DKCopying.h"
#include "DKEncoding.h"
#include "DKUnicode.h"


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

    if( options & DK_JSON_PRETTY )
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
        
        else
        {
            DKWarning( "DKJSON: Only scalar number encodings are supported." );
            DKSPrintf( context->stream, "0" );
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
    
    const char * fmt = (context->options & DK_JSON_PRETTY) ? "\"%@\" : " : "\"%@\":";
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

static const DKEscapedChar EscapedChars[] =
{
    { '\\', "\\\\" },
    { '\"', "\\\"" },
    { '\n', "\\n" },
    { '\r', "\\r" },
    { '\f', "\\f" },
    { '\t', "\\t" },
    { '\b', "\\b" },
    //{ '/', "\\/" },
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
            if( ec->pattern == ch )
            {
                if( bufferLength > 0 )
                    stream->write( context->stream, bufferStart, 1, bufferLength );

                DKSPrintf( context->stream, "%s", ec->replacement );

                bufferStart = cursor + n;
                bufferLength = 0;
                break;
            }
        
            if( ec->pattern == 0 )
            {
                bufferLength += n;
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
        if( context->options & DK_JSON_PRETTY )
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
    if( context->options & DK_JSON_PRETTY )
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
    if( context->options & DK_JSON_PRETTY )
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

static Token ScanToken( ParseContext * context, DKStringRef * stringValue );
static DKEncodingType ParseNumberType( Token token );


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
    DKStringRef stringValue = NULL;
    Token token = ScanToken( context, &stringValue );
    
    if( token.length == 0 )
        return -1;
    
    char ch = *token.str;
    DKObjectRef key = NULL;
    DKObjectRef value = NULL;
    int result;

    // String
    if( stringValue != NULL )
    {
        *obj = stringValue;
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
            int result = ParseObject( context, &value );
            
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
        if( ParseNumberType( token ) == DKEncodingTypeDouble )
        {
            double x;
            sscanf( token.str, "%lf", &x );
            
            *obj = DKNewNumberWithDouble( x );
            return 0;
        }
        
        else
        {
            int64_t x;
            sscanf( token.str, "%lld", &x );
            
            *obj = DKNewNumberWithInt64( x );
            return 0;
        }
    }
    
    // True
    else if( strncmp( token.str, "true", 4 ) == 0 )
    {
        *obj = DKTrue();
        return 0;
    }

    // False
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
//  ScanToken()
//
static Token ScanToken( ParseContext * context, DKStringRef * stringValue )
{
    Token token;
    token.str = context->cursor;
    token.length = 0;

    DKChar32 ch;

    // Skip whitespace
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
        DKAssert( stringValue != NULL );
        *stringValue = DKNewMutableString();
    
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
                    DKStringAppendCString( *stringValue, "\\" );
                    break;
                    
                case '"':
                    DKStringAppendCString( *stringValue, "\"" );
                    break;
                    
                case 'b':
                    DKStringAppendCString( *stringValue, "\b" );
                    break;
                    
                case 'f':
                    DKStringAppendCString( *stringValue, "\f" );
                    break;
                    
                case 'n':
                    DKStringAppendCString( *stringValue, "\n" );
                    break;
                    
                case 'r':
                    DKStringAppendCString( *stringValue, "\r" );
                    break;
                    
                case 't':
                    DKStringAppendCString( *stringValue, "\t" );
                    break;
                    
                case '/':
                    DKStringAppendCString( *stringValue, "/" );
                    break;
                        
                case 'u':
                    DKWarning( "DKJSON: Escaped unicode characters (\\uXXXX) are not supported.\n" );
                    token.length += 4;
                    break;
                    
                default:
                    DKWarning( "DKJSON: Invalid control character: %c (%d)\n", ch, ch );
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
                DKStringWrite( *stringValue, token.str + token.length, 1, n );
            }

            token.length += n;
        }
    }
    
    // Scan to the end of other tokens
    else
    {
        while( true )
        {
            size_t n = dk_ustrscan( token.str + token.length, &ch );
            
            if( (n == 0) || (ch == ',') || isspace( ch ) )
            {
                context->cursor = token.str + token.length;
                return token;
            }
            
            token.length += n;
        }
    }
}


///
//  ParseNumberType()
//
static DKEncodingType ParseNumberType( Token token )
{
    DKEncodingType encoding = DKEncodingTypeInt64;

    const char * str = token.str;
    const char * end = str + token.length;
    
    if( *str == '-' )
        str++;
    
    while( str < end )
    {
        char ch = *str;
        
        if( (ch == '.') || (ch == 'e') || (ch == 'E') )
        {
            encoding = DKEncodingTypeDouble;
            break;
        }
        
        str++;
    }
    
    return encoding;
}









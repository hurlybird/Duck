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
#include "DKArray.h"
#include "DKHashTable.h"
#include "DKStream.h"
#include "DKCopying.h"



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
//  WriteObject();
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
        DKRange quoted = DKStringGetRangeOfString( obj, DKSTR( "\"" ), 0 );
        
        if( quoted.location == DKNotFound )
        {
            DKSPrintf( context->stream, "\"%@\"", obj );
        }
        
        else
        {
            DKMutableStringRef escapedString = DKMutableCopy( obj );
            DKStringReplaceOccurrencesOfString( escapedString, DKSTR( "\"" ), DKSTR( "\\\"" ) );
            
            DKSPrintf( context->stream, "\"%@\"", escapedString );
            
            DKRelease( escapedString );
        }
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

///
//  DKJSONParse()
//
DKObjectRef DKJSONParse( DKStreamRef json, int options )
{
    return NULL;
}






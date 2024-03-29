/*****************************************************************************************

  DKEgg.h

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

#ifndef _DK_EGG_H_
#define _DK_EGG_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DKEggUnarchiver * DKEggUnarchiverRef;
typedef struct DKEggArchiver * DKEggArchiverRef;


// Egg Interface - Adopted by any object that supports egg storage =======================
DK_API DKDeclareInterfaceSelector( Egg );

typedef DKObjectRef (*DKInitWithEggMethod)( DKObjectRef _self, DKEggUnarchiverRef egg );
typedef void        (*DKAddToEggMethod)( DKObjectRef _self, DKEggArchiverRef egg );

struct DKEggInterface
{
    const DKInterface _interface;

    DKInitWithEggMethod initWithEgg;
    DKAddToEggMethod    addToEgg;
};

typedef const struct DKEggInterface * DKEggInterfaceRef;




// DKEggUnarchiver =======================================================================
DK_API DKClassRef DKEggUnarchiverClass( void );

#define DKEggUnarchiverWithStream( stream )      DKAutorelease( DKEggUnarchiverInitWithStream( DKAlloc( DKEggUnarchiverClass() ), stream ) )
#define DKNewEggUnarchiverWithStream( stream )   DKEggUnarchiverInitWithStream( DKAlloc( DKEggUnarchiverClass() ), stream )

#define DKEggUnarchiverWithData( data )          DKAutorelease( DKEggUnarchiverInitWithData( DKAlloc( DKEggUnarchiverClass() ), data ) )
#define DKNewEggUnarchiverWithData( data )       DKEggUnarchiverInitWithData( DKAlloc( DKEggUnarchiverClass() ), data )

DK_API DKEggUnarchiverRef DKEggUnarchiverInitWithStream( DKEggUnarchiverRef _self, DKObjectRef stream );
DK_API DKEggUnarchiverRef DKEggUnarchiverInitWithData( DKEggUnarchiverRef _self, DKDataRef data );

DK_API DKObjectRef DKEggGetRootObject( DKEggUnarchiverRef _self );

DK_API DKEncoding DKEggGetEncoding( DKEggUnarchiverRef _self, DKStringRef key );

DK_API DKObjectRef DKEggGetObject( DKEggUnarchiverRef _self, DKStringRef key );
DK_API void    DKEggGetCollection( DKEggUnarchiverRef _self, DKStringRef key, DKApplierFunction callback, void * context );
DK_API void    DKEggGetKeyedCollection( DKEggUnarchiverRef _self, DKStringRef key, DKKeyedApplierFunction callback, void * context );

DK_API const char * DKEggGetTextDataPtr( DKEggUnarchiverRef _self, DKStringRef key, size_t * length );
DK_API size_t DKEggGetTextData( DKEggUnarchiverRef _self, DKStringRef key, char * text );

DK_API const void * DKEggGetBinaryDataPtr( DKEggUnarchiverRef _self, DKStringRef key, size_t * length );
DK_API size_t DKEggGetBinaryData( DKEggUnarchiverRef _self, DKStringRef key, void * bytes, size_t length );

DK_API size_t DKEggGetNumberData( DKEggUnarchiverRef _self, DKStringRef key, void * number );




// DKEggArchiver =========================================================================
DK_API DKClassRef DKEggArchiverClass( void );

#define DKEggArchiver()                     DKAutorelease( DKEggArchiverInitWithObject( DKAlloc( DKEggArchiverClass() ), NULL ) )
#define DKNewEggArchiver()                  DKEggArchiverInitWithObject( DKAlloc( DKEggArchiverClass() ), NULL )

#define DKEggArchiverWithObject( obj )      DKAutorelease( DKEggArchiverInitWithObject( DKAlloc( DKEggArchiverClass() ), obj ) )
#define DKNewEggArchiverWithObject( obj )   DKEggArchiverInitWithObject( DKAlloc( DKEggArchiverClass() ), obj )

DK_API DKObjectRef DKEggArchiverInitWithObject( DKObjectRef _self, DKObjectRef object );

DK_API void DKEggArchiverWriteToStream( DKEggArchiverRef _self, DKObjectRef stream );
DK_API DKDataRef DKEggArchiverGetArchivedData( DKEggArchiverRef _self );

DK_API void DKEggAddObject( DKEggArchiverRef _self, DKStringRef key, DKObjectRef object );
DK_API void DKEggAddCollection( DKEggArchiverRef _self, DKStringRef key, DKObjectRef collection );
DK_API void DKEggAddKeyedCollection( DKEggArchiverRef _self, DKStringRef key, DKObjectRef collection );

DK_API void DKEggAddTextData( DKEggArchiverRef _self, DKStringRef key, const char * text, size_t length );
DK_API void DKEggAddBinaryData( DKEggArchiverRef _self, DKStringRef key, const void * bytes, size_t length );
DK_API void DKEggAddNumberData( DKEggArchiverRef _self, DKStringRef key, DKEncoding encoding, const void * number );


#ifdef __cplusplus
}
#endif

#endif // _DK_EGG_H_






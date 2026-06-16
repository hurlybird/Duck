// =======================================================================================
//
// DKFile.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_FILE_H_
#define _DK_FILE_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DKFile * DKFileRef;


DK_API DKClassRef  DKFileClass( void );

#define DKFile( filename, mode )                            DKAutorelease( DKFileOpen( filename, mode ) )
#define DKNewFile( filename, mode )                         DKFileOpen( filename, mode )

#define DKFileWithStreamPtr( stream, closeOnDealloc )       DKAutorelease( DKFileInitWithStreamPtr( DKAlloc( DKFileClass() ), stream, closeOnDealloc ) )
#define DKNewFileWithStreamPtr( stream, closeOnDealloc )    DKFileInitWithStreamPtr( DKAlloc( DKFileClass() ), stream, closeOnDealloc )

DK_API DKFileRef   DKFileInitWithStreamPtr( DKObjectRef _self, FILE * stream, bool closeOnDealloc );

// Returns true if the file exists
DK_API bool        DKFileExists( DKStringRef filename );

// Delete a file
DK_API void        DKDeleteFile( DKStringRef filename );

// Create and open a new file - the returned object must be closed or released
DK_API DKFileRef   DKFileOpen( DKStringRef filename, const char * mode );

// Close the file and release the object reference
DK_API int         DKFileClose( DKFileRef _self );

DK_API FILE *      DKFileGetStreamPtr( DKFileRef _self );

DK_API int         DKFileSeek( DKFileRef _self, long offset, int origin );
DK_API long        DKFileTell( DKFileRef _self );

DK_API int         DKFileGetStatus( DKFileRef _self );
DK_API DKIndex     DKFileGetLength( DKFileRef _self );

DK_API size_t      DKFileRead( DKFileRef _self, void * buffer, size_t size, size_t count );
DK_API size_t      DKFileWrite( DKFileRef _self, const void * buffer, size_t size, size_t count );

DK_API int         DKFileFlush( DKFileRef _self );

#ifdef __cplusplus
}
#endif

#endif // _DK_FILE_H_

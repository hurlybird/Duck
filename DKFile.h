//
//  DKFile.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_FILE_H_
#define _DK_FILE_H_

#include "DKRuntime.h"

typedef const struct DKFile * DKFileRef;


DKClassRef  DKFileClass( void );

DKFileRef   DKFileCreate( void );

int         DKFileOpen( DKFileRef ref, const char * fname, const char * mode );
int         DKFileClose( DKFileRef ref );

int         DKFileSeek( DKFileRef ref, DKIndex offset, int origin );
DKIndex     DKFileTell( DKFileRef ref );

DKIndex     DKFileRead( DKFileRef ref, void * buffer, DKIndex size, DKIndex count );
DKIndex     DKFileWrite( DKFileRef ref, const void * buffer, DKIndex size, DKIndex count );



#endif // _DK_FILE_H_

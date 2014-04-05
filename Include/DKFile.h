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

int         DKFileOpen( DKFileRef _self, const char * fname, const char * mode );
int         DKFileClose( DKFileRef _self );

int         DKFileSeek( DKFileRef _self, DKIndex offset, int origin );
DKIndex     DKFileTell( DKFileRef _self );

DKIndex     DKFileRead( DKFileRef _self, void * buffer, DKIndex size, DKIndex count );
DKIndex     DKFileWrite( DKFileRef _self, const void * buffer, DKIndex size, DKIndex count );



#endif // _DK_FILE_H_

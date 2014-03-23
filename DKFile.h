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


DKTypeRef   DKFileClass( void );

DKTypeRef   DKFileCreate( void );

int         DKFileOpen( DKTypeRef ref, const char * fname, const char * mode );
int         DKFileClose( DKTypeRef ref );

int         DKFileSeek( DKTypeRef ref, DKIndex offset, int origin );
DKIndex     DKFileTell( DKTypeRef ref );

DKIndex     DKFileRead( DKTypeRef ref, void * buffer, DKIndex size, DKIndex count );
DKIndex     DKFileWrite( DKTypeRef ref, const void * buffer, DKIndex size, DKIndex count );



#endif // _DK_FILE_H_

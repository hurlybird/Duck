/*****************************************************************************************

  DKFile.h

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

#ifndef _DK_FILE_H_
#define _DK_FILE_H_

#include "DKRuntime.h"

typedef struct DKFile * DKFileRef;


DKClassRef  DKFileClass( void );

// Create and open a new file
DKFileRef   DKFileOpen( DKStringRef filename, const char * mode );

// Close the file and release the object reference
int         DKFileClose( DKFileRef _self );

int         DKFileSeek( DKFileRef _self, DKIndex offset, int origin );
DKIndex     DKFileTell( DKFileRef _self );
DKIndex     DKFileGetLength( DKFileRef _self );

DKIndex     DKFileRead( DKFileRef _self, void * buffer, DKIndex size, DKIndex count );
DKIndex     DKFileWrite( DKFileRef _self, const void * buffer, DKIndex size, DKIndex count );




#endif // _DK_FILE_H_

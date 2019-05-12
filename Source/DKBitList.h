/*****************************************************************************************

  DKBitlist.h

  Copyright (c) 2019 Derek W. Nylen

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
#ifndef _DK_BITLIST_H_
#define _DK_BITLIST_H_

#include "DKRuntime.h"



typedef struct DKBitList * DKBitListRef;


DKClassRef DKBitListClass( void );

#define DKBitList( len, val )       DKAutorelease( DKBitListInit( DKAlloc( DKBitListClass() ), len, val ) )
#define DKNewBitList( len, val )    DKBitListInit( DKAlloc( DKBitListClass() ), len, val )

DKObjectRef DKBitListInit( DKBitListRef _self, DKIndex length, bool initialValue );

DKIndex DKBitListGetLength( DKBitListRef _self );

void DKBitListSetAllBits( DKBitListRef _self );
void DKBitListClearAllBits( DKBitListRef _self );
void DKBitListFlipAllBits( DKBitListRef _self );

void DKBitListSetBit( DKBitListRef _self, DKIndex i );
void DKBitListClearBit( DKBitListRef _self, DKIndex i );
void DKBitListFlipBit( DKBitListRef _self, DKIndex i );

bool DKBitListTestBit( DKBitListRef _self, DKIndex i );
bool DKBitListTestAndSetBit( DKBitListRef _self, DKIndex i );
bool DKBitListTestAndClearBit( DKBitListRef _self, DKIndex i );
bool DKBitListTestAndFlipBit( DKBitListRef _self, DKIndex i );

DKIndex DKBitListGetFirstSetBit( DKBitListRef _self );
DKIndex DKBitListGetFirstClearBit( DKBitListRef _self );

DKIndex DKBitListCountSetBits( DKBitListRef _self );
DKIndex DKBitListCountClearBits( DKBitListRef _self );

// Merge r = a OP b
void DKBitListMergeBitsAND( DKBitListRef a, DKBitListRef b, DKBitListRef r );
void DKBitListMergeBitsNAND( DKBitListRef a, DKBitListRef b, DKBitListRef r );
void DKBitListMergeBitsOR( DKBitListRef a, DKBitListRef b, DKBitListRef r );
void DKBitListMergeBitsNOR( DKBitListRef a, DKBitListRef b, DKBitListRef r );
void DKBitListMergeBitsXOR( DKBitListRef a, DKBitListRef b, DKBitListRef r );
void DKBitListMergeBitsNXOR( DKBitListRef a, DKBitListRef b, DKBitListRef r );



#endif // _DK_BITLIST_H_



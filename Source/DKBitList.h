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

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DKBitList * DKBitListRef;


DK_API DKClassRef DKBitListClass( void );

#define DKBitList( len, val )       DKAutorelease( DKBitListInit( DKAlloc( DKBitListClass() ), len, val ) )
#define DKNewBitList( len, val )    DKBitListInit( DKAlloc( DKBitListClass() ), len, val )

DK_API DKObjectRef DKBitListInit( DKBitListRef _self, DKIndex length, bool initialValue );

DK_API DKIndex DKBitListGetLength( DKBitListRef _self );

DK_API void DKBitListSetAllBits( DKBitListRef _self );
DK_API void DKBitListClearAllBits( DKBitListRef _self );
DK_API void DKBitListFlipAllBits( DKBitListRef _self );

DK_API void DKBitListSetBit( DKBitListRef _self, DKIndex i );
DK_API void DKBitListClearBit( DKBitListRef _self, DKIndex i );
DK_API void DKBitListFlipBit( DKBitListRef _self, DKIndex i );

DK_API bool DKBitListTestBit( DKBitListRef _self, DKIndex i );
DK_API bool DKBitListTestAndSetBit( DKBitListRef _self, DKIndex i );
DK_API bool DKBitListTestAndClearBit( DKBitListRef _self, DKIndex i );
DK_API bool DKBitListTestAndFlipBit( DKBitListRef _self, DKIndex i );

DK_API DKIndex DKBitListGetFirstSetBit( DKBitListRef _self );
DK_API DKIndex DKBitListGetFirstClearBit( DKBitListRef _self );

DK_API DKIndex DKBitListCountSetBits( DKBitListRef _self );
DK_API DKIndex DKBitListCountClearBits( DKBitListRef _self );

// Merge r = a OP b
DK_API void DKBitListMergeBitsAND( DKBitListRef a, DKBitListRef b, DKBitListRef r );
DK_API void DKBitListMergeBitsNAND( DKBitListRef a, DKBitListRef b, DKBitListRef r );
DK_API void DKBitListMergeBitsOR( DKBitListRef a, DKBitListRef b, DKBitListRef r );
DK_API void DKBitListMergeBitsNOR( DKBitListRef a, DKBitListRef b, DKBitListRef r );
DK_API void DKBitListMergeBitsXOR( DKBitListRef a, DKBitListRef b, DKBitListRef r );
DK_API void DKBitListMergeBitsNXOR( DKBitListRef a, DKBitListRef b, DKBitListRef r );


#ifdef __cplusplus
}
#endif

#endif // _DK_BITLIST_H_



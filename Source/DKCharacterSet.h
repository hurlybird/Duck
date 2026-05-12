/*****************************************************************************************

  DKCharacterSet.h

  Copyright (c) 2026 Derek W. Nylen

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

#ifndef _DK_CHARACTER_SET_H_
#define _DK_CHARACTER_SET_H_

#ifdef __cplusplus
extern "C"
{
#endif


// DKCharacterRange
typedef struct
{
    DKChar32 first;
    DKChar32 last;

} DKCharacterRange;

#define DKCharacterRangeMake( first, last )     (DKCharacterRange){ first, last }


#define DKCharacterRangeASCII                   DKCharacterRangeMake( 0, 0x7F )
#define DKCharacterRangeASCIIControlCodes       DKCharacterRangeMake( 0, 0x1F )

#define DKCharacterRangeUnicode                 DKCharacterRangeMake( 0, 0x10ffff )
#define DKCharacterRangeUnicodeSurrogates       DKCharacterRangeMake( 0xD800, 0xDFFF )
#define DKCharacterRangeUnicodePrivateUseArea   DKCharacterRangeMake( 0xE000, 0xF8FF )
#define DKCharacterRangeUnicodePrivateUseSuplA  DKCharacterRangeMake( 0xF0000, 0xFFFFF )
#define DKCharacterRangeUnicodePrivateUseSuplB  DKCharacterRangeMake( 0x100000, 0x10FFFF )



// DKCharacterSet
//typedef struct DKCharacterSet * DKCharacterSetRef; -- Declared in DKPlatform.h


DK_API DKClassRef  DKCharacterSetClass( void );

#define DKCharacterSet()        DKAutorelease( DKNew( DKCharacterSetClass() ) )
#define DKNewCharacterSet()     DKNew( DKCharacterSetClass() )

DK_API void DKCharacterSetIncludeCharactersInRange( DKCharacterSetRef _self, DKCharacterRange range );
DK_API void DKCharacterSetExcludeCharactersInRange( DKCharacterSetRef _self, DKCharacterRange range );

#define DKCharacterSetIncludeCharacter( _self, ch ) DKCharacterSetIncludeCharactersInRange( _self, DKCharacterRangeMake( ch, ch ) )
#define DKCharacterSetExcludeCharacter( _self, ch ) DKCharacterSetExcludeCharactersInRange( _self, DKCharacterRangeMake( ch, ch ) )

DK_API void DKCharacterSetExcludeUnicodeNonCharacters( DKCharacterSetRef _self );

DK_API bool DKCharacterSetContainsCharacter( DKCharacterSetRef _self, DKChar32 ch );


#ifdef __cplusplus
}
#endif

#endif // _DK_PAIR_H_

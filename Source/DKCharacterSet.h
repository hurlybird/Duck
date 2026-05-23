// =======================================================================================
//
// DKCharacterSet.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
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

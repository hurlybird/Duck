//
//  DKUnicode.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-31.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_UNICODE_H_
#define _DK_UNICODE_H_

#include "DKPlatform.h"


// UTF8 aware versions of standard string functions

const char * dk_ustrchr( const char * str, int ch );

const char * dk_ustrrchr( const char * str, int ch );

int dk_ustrcmp( const char * str1, const char * str2, int options );

size_t dk_ustrlen( const char * str );

const char * dk_ustridx( const char * str, size_t idx );

size_t dk_ustrscan( const char * str, char32_t * ch );


#endif // _DK_UNICODE_H_

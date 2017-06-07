/*****************************************************************************************

  DKShell.h

  Copyright (c) 2017 Derek W. Nylen

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

#ifndef _DK_SHELL_H_
#define _DK_SHELL_H_

#include "DKRuntime.h"
#include "DKData.h"



/*

Shell directives:
    Always start with '@'
    Always occupy a single line
    Parameters are KEY=VALUE pairs separated by spaces
    Parameters may be in any order
    Parameter keys must be lowercase
    Parameter values may be:
        Numbers
        Percent encoded strings
        One or more tokens separated by '+'

File header
@shell version=# keyed=yes|no byte-order=le:be
 
Text segment header. The encoding field is pedantic -- DKString only supports UTF-8.
@text length=BYTES encoding=UTF8 [id="NAME"] [decode=MODIFIER[+MODIFIER2[+MODIFIER3[...]]]]

Binary segment header
@binary length=BYTES [id="NAME"] [decode=MODIFIER[+MODIFIER2[+MODIFIER3[...]]]]

*/



#endif // _DK_SHELL_H_

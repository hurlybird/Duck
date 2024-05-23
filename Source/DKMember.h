/*****************************************************************************************

  DKMember.h

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

#ifndef _DK_MEMBER_H_
#define _DK_MEMBER_H_

#ifdef __cplusplus
extern "C"
{
#endif


struct DKMember
{
    DKObject _obj;
    
    DKObjectRef object;
    size_t offset;
};

typedef struct DKMember * DKMemberRef;


DK_API DKClassRef  DKMemberClass( void );

#define DKMember( object, offset, encoding )     DKAutorelease( DKMemberInit( DKAlloc( DKMemberClass() ), object, offset, encoding ) )
#define DKNewMember( object, offset, encoding )  DKMemberInit( DKAlloc( DKMemberClass() ), object, offset, encoding )

DK_API DKObjectRef DKMemberInit( DKObjectRef _self, DKObjectRef object, size_t offset, DKEncoding encoding );

DK_API DKObjectRef DKMemberGetObject( DKMemberRef _self );
DK_API size_t      DKMemberGetOffset( DKMemberRef _self );
DK_API DKEncoding  DKMemberGetEncoding( DKMemberRef _self );

DK_API const void* DKMemberGetValuePtr( DKMemberRef _self );
DK_API const void* DKMemberQueryValuePtr( DKMemberRef _self, DKEncoding * encoding );


#ifdef __cplusplus
}
#endif

#endif // _DK_MEMBER_H_

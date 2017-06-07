/*****************************************************************************************

  DKModifier.c

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

#include "DKModifier.h"
#include "DKString.h"


struct DKModifier
{
    DKObject _obj;
    
    DKStringRef name;
    DKModifierFunction forward;
    DKModifierFunction reverse;
    void * context;
};

static void DKModifierFinalize( DKObjectRef _untyped_self );

DKThreadSafeClassInit( DKModifierClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKModifier" ), DKObjectClass(), sizeof(struct DKModifier), 0, NULL, DKModifierFinalize );

    return cls;
}


///
//  DKModifierInit()
//
DKObjectRef DKModifierInit( DKObjectRef _untyped_self, DKStringRef name, DKModifierFunction forward, DKModifierFunction reverse, void * context )
{
    DKModifierRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
        _self->name = DKRetain( name );
        _self->forward = forward;
        _self->reverse = reverse;
        _self->context = context;
    }
    
    return _self;
}


///
//  DKModifierFinalize()
//
static void DKModifierFinalize( DKObjectRef _untyped_self )
{
    DKModifierRef _self = _untyped_self;
    
    DKRelease( _self->name );
}


///
//  DKModifierGetName()
//
DKStringRef DKModifierGetName( DKModifierRef _self )
{
    return _self ? _self->name : NULL;
}


///
//  DKModifierApply()
//
DKObjectRef DKModifierApply( DKModifierRef _self, DKObjectRef object, bool forward )
{
    if( forward )
        return DKModifierApplyForward( _self, object );
    
    else
        return DKModifierApplyReverse( _self, object );
}


///
//  DKModifierApplyForward()
//
DKObjectRef DKModifierApplyForward( DKModifierRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKModifierClass() );
        DKAssert( _self->forward != NULL );
        
        return _self->forward( object, _self->context );
    }
    
    return object;
}


///
//  DKModifierApplyReverse()
//
DKObjectRef DKModifierApplyReverse( DKModifierRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKModifierClass() );
        DKAssert( _self->reverse != NULL );
        
        return _self->reverse( object, _self->context );
    }
    
    return object;
}




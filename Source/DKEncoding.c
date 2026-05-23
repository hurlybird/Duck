// =======================================================================================
//
// DKEncoding.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKEncoding.h"
#include "DKString.h"



// Component Info ========================================================================
struct EncodingTypeInfo
{
    const char * name;
    size_t size;
};

static const struct EncodingTypeInfo TypeInfo[DKMaxEncodingTypes] =
{
    { "void",       0                   },
    
    { "class",      sizeof(void *)      },
    { "selector",   sizeof(void *)      },

    { "object",     sizeof(void *)      },
    { "key:object", 2*sizeof(void *)    },
    
    { "text",       1                   },
    { "binary",     1                   },

    { "reserved",   0                   },

    { "int8_t",     sizeof(int8_t)      },
    { "int16_t",    sizeof(int16_t)     },
    { "invalid",    0                   },
    { "int32_t",    sizeof(int32_t)     },
    { "invalid",    0                   },
    { "invalid",    0                   },
    { "invalid",    0                   },
    { "int64_t",    sizeof(int64_t)     },

    { "uint8_t",    sizeof(uint8_t)     },
    { "uint16_t",   sizeof(uint16_t)    },
    { "invalid",    0                   },
    { "uint32_t",   sizeof(uint32_t)    },
    { "invalid",    0                   },
    { "invalid",    0                   },
    { "invalid",    0                   },
    { "uint64_t",   sizeof(uint64_t)    },

    { "float",      sizeof(float)       },
    { "double",     sizeof(double)      }
};


///
//  DKEncodingGetSize()
//
size_t DKEncodingGetSize( DKEncoding encoding )
{
    return DKEncodingGetTypeSize( encoding ) * DKEncodingGetCount( encoding );
}


///
//  DKEncodingTypeGetSize()
//
size_t DKEncodingTypeGetSize( DKEncodingType encodingType )
{
    if( (encodingType > 0) && (encodingType < DKMaxEncodingTypes) )
        return TypeInfo[encodingType].size;
    
    return TypeInfo[0].size;
}


///
//  DKEncodingGetTypeSize()
//
size_t DKEncodingGetTypeSize( DKEncoding encoding )
{
    DKEncodingType encodingType = DKEncodingGetType( encoding );

    if( (encodingType > 0) && (encodingType < DKMaxEncodingTypes) )
        return TypeInfo[encodingType].size;
    
    return TypeInfo[0].size;
}


///
//  DKEncodingGetTypeName()
//
const char * DKEncodingGetTypeName( DKEncoding encoding )
{
    DKEncodingType encodingType = DKEncodingGetType( encoding );

    if( (encodingType > 0) && (encodingType < DKMaxEncodingTypes) )
        return TypeInfo[encodingType].name;
    
    return TypeInfo[0].name;
}


///
//  DKEncodingGetDescription()
//
DKStringRef DKEncodingGetDescription( DKEncoding encoding )
{
    const char * typeName = DKEncodingGetTypeName( encoding );
    
    if( DKEncodingIsMatrix( encoding ) )
    {
        unsigned int rows = DKEncodingGetRows( encoding );
        unsigned int cols = DKEncodingGetCols( encoding );
        
        return DKStringWithFormat( "%s[%d][%d]", typeName, rows, cols );
    }

    unsigned int count = DKEncodingGetCount( encoding );

    if( count == 1 )
    {
        return DKStringWithCStringNoCopy( typeName );
    }
    
    else
    {
        return DKStringWithFormat( "%s[%d]", typeName, count );
    }
}


///
//  DKEncodingIsNumber()
//
bool DKEncodingIsNumber( DKEncoding encoding )
{
    DKEncodingType encodingType = DKEncodingGetType( encoding );
    uint32_t count = DKEncodingGetCount( encoding );
    
    return (encodingType >= DKEncodingTypeInt8) && (encodingType <= DKEncodingTypeDouble) && (count > 0);
}


///
//  DKEncodingIsInteger()
//
bool DKEncodingIsInteger( DKEncoding encoding )
{
    DKEncodingType encodingType = DKEncodingGetType( encoding );
    uint32_t count = DKEncodingGetCount( encoding );
    
    return (encodingType >= DKEncodingTypeInt8) && (encodingType <= DKEncodingTypeUInt64) && (count > 0);
}


///
//  DKEncodingIsReal()
//
bool DKEncodingIsReal( DKEncoding encoding )
{
    DKEncodingType encodingType = DKEncodingGetType( encoding );
    uint32_t count = DKEncodingGetCount( encoding );
    
    return (encodingType >= DKEncodingTypeFloat) && (encodingType <= DKEncodingTypeDouble) && (count > 0);
}






//
//  DKPlatformPosix.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-27.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//
#include <sys/time.h>

#include "DKPlatform.h"



///
//  dk_time()
//
double dk_time( void )
{
    struct timeval t;
    
    if( gettimeofday( &t, NULL ) )
        return 0.0;
    
    return (double)t.tv_sec + ((double)t.tv_usec / 1000000.0);
}





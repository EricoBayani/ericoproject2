#ifndef POLY_H
#define POLY_H

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <cmath>

#include "basesystem.h"
#include "core.h"
#include "vpu.h"
#include "spec.h"

#include "commondemo.h"


extern void polyTest( void );
extern void renderTri( int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t col );

typedef struct 
{
    int32_t     x,y,u,v;
} TexVert;

extern void renderTexTri( TexVert* p1, TexVert* p2, TexVert* p3 );

extern void setEnvTex( uint8_t* tex );


#endif
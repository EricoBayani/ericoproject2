#ifndef _3D_H
#define _3D_H

#include <stdint.h>
#include "commondemo.h"

typedef struct {
    int32_t     x, y, z;
} ivec3;

typedef struct {
    int32_t     v[ 4 ];
} ivec4;

typedef struct {
    ivec4       rows[ 4 ];
} mtx4;

typedef struct {
    uint32_t    nidx;
    int32_t     dist;
} planeDef;

typedef struct {
    uint32_t    vidx[ 3 ];      // vertex indices
    uint32_t    vnidx[ 3 ];     // vertex-normal indices
    uint32_t    pidx;           // plane-index
    uint32_t    attr;           // attr?
} polyDef;

typedef struct {
    int32_t         vertCt;
    int32_t         normalCt;       // plane normals
    int32_t         vertexNormalCt; // vertex normals
    int32_t         planeCt;
    int32_t         polyCt;
    uint32_t        radius;

    ivec3*          verts;
    ivec3*          normals;        // plane normals
    ivec3*          vertexNormals;  // vertex normals
    planeDef*       planes;
    polyDef*        polys;
} shapeDef;



#define DEPTH_BAND_CT       ( 16 )
#define DEPTH_INVIS         ( -0x7fffffff )

typedef struct
{
    int32_t         u;
    int32_t         v;
} envMapTexCoord;

typedef struct {
    int32_t         depth;
    uint32_t        col;
    polyDef*        def;
} polyWork;

typedef struct {
    uint32_t        maxPolys;
    uint32_t        polyCt;
    polyWork**      polyPtrs;
} depthBand;

#define SW_FLAG_LINES               1
#define SW_FLAG_FORCE_POLY_COL      2
#define SW_FLAG_ENVMAP              4

typedef struct {
    shapeDef*       def;

    mtx4            shapeToWorld;           // shape transform
    mtx4            shapeToWorldNoXlate;    // shape transform
    mtx4            cameraToWorld;          // camera transform
    ivec3           lightVec;               // direction TO light
    uint8_t         lightPalette[ 256 ];

    mtx4            worldToShapeNoXlate;    // inverse shape transform
    mtx4            worldToCamera;          // inverse camera transform
    mtx4            shapeToCamera;          // transform for rendering
    mtx4            cameraToShape;          // inverse transform
    ivec3           cameraInShapeSpace;     // pt for planar vistests
    ivec3           lightInShapeSpace;      // light vector in shape-space

    int32_t         zmin;
    int32_t         zrange;

    ivec3*          transformedVerts;
    ivec3*          projectedVerts;
    int32_t*        normalsDotLight;
    int32_t*        normalsDotCamera;
    uint32_t        visPolyCt;
    depthBand       depthBands[ DEPTH_BAND_CT ];
    polyWork*       visPolys;
    polyWork**      sortedPolys;
    envMapTexCoord* envMapTexCoords;
    int32_t         currentDepthBand;

    uint8_t         lineCol;
    uint8_t         forcePolyCol;
    uint8_t         flags;
} shapeWork;


extern shapeWork* allocShapeWork( Heap* h, shapeDef* sd );

extern void renderShape( shapeWork* sw );
extern void renderShapeSimple( shapeWork* w, int a, int b, int c, int x, int y, int z, int flags = SW_FLAG_LINES | SW_FLAG_FORCE_POLY_COL );

extern int16_t g_sintab[];

static inline int16_t isin( int32_t ang )
{
    return( g_sintab[ ang & 0xff ] );
}
static inline int16_t icos( int32_t ang )
{
    return( g_sintab[ ( ang + 64 ) & 0xff ] );
}


extern void mtx4_from_angles( int16_t x, int16_t y, int16_t z, mtx4* out );
extern void ivec3_mtx4_transform( ivec3* in, mtx4* mtx, ivec3* out );
extern void mtx4_mtx4_mul( mtx4* a, mtx4* b, mtx4* out );
extern void mtx4_invert( mtx4* in, mtx4* out );
extern void mtx4_set_translate( mtx4* m, ivec3* t );
extern void mtx4_ident( mtx4* out );


static inline void ivec3_set( ivec3* a, int32_t x, int32_t y, int32_t z )
{
    a->x = x;  
    a->y = y;
    a->z = z;
}

static inline int32_t ivec3_dot_unscaled( ivec3* a, ivec3* b )
{
    return( ( (int32_t)a->x * (int32_t)b->x ) +
            ( (int32_t)a->y * (int32_t)b->y ) +
            ( (int32_t)a->z * (int32_t)b->z ) );
}

static inline int32_t ivec3_dot_scaled( ivec3* a, ivec3* b )
{
    return( ( ( (int32_t)a->x * (int32_t)b->x ) >> 14 ) +
            ( ( (int32_t)a->y * (int32_t)b->y ) >> 14 ) +
            ( ( (int32_t)a->z * (int32_t)b->z ) >> 14 ));
}

static inline void ivec3_negate( ivec3* a, ivec3* b )
{
    b->x = -( a->x );
    b->y = -( a->y );
    b->z = -( a->z );
}

static inline void ivec3_add( ivec3* a, ivec3* b, ivec3* out )
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
}

static inline void ivec3_sub( ivec3* a, ivec3* b, ivec3* out )
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
}

static inline void ivec3_scale( ivec3* a, int32_t scale, ivec3* out )
{
    out->x = ( (int32_t)( ( (int32_t)a->x * scale ) >> 14 ) );
    out->y = ( (int32_t)( ( (int32_t)a->y * scale ) >> 14 ) );
    out->z = ( (int32_t)( ( (int32_t)a->z * scale ) >> 14 ) );
}

static inline void ivec3_normalize( ivec3* a, ivec3* out )
{
    float       x = (float)a->x;
    float       y = (float)a->y;
    float       z = (float)a->z;
    float       m = ( x * x ) + ( y * y ) + ( z * z );
    m = sqrtf( m ) / 16384.0;

    if( m != 0.0 )
    {
        out->x = (int)( x / m );
        out->y = (int)( y / m );
        out->z = (int)( z / m );
    }
}

static inline void lerp_array( int32_t ct, int32_t fac, ivec3* s1, ivec3* s2, ivec3* dest )
{
    while( ct-- )
    {
        dest->x = s1->x + ( ( ( s2->x - s1->x ) * fac ) >> 8 );
        dest->y = s1->y + ( ( ( s2->y - s1->y ) * fac ) >> 8 );
        dest->z = s1->z + ( ( ( s2->z - s1->z ) * fac ) >> 8 );
        s1++;
        s2++;
        dest++;
    }
}

static inline void project_vert( ivec3* a, ivec3* b )
{
    b->z = a->z;
    if( a->z < NEAR_Z )
    {
        b->x = 0;
        b->y = 0;
    }
    else
    {
        b->x = ( SCR_X/2) + ( a->x * FOV_FACTOR / a->z );
        b->y = ( SCR_Y/2) - ( a->y * FOV_FACTOR / a->z );
    }
}


extern void print_ivec3( ivec3* a );
extern void print_mtx4( mtx4* b );

#endif


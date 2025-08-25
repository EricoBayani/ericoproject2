#ifndef COMMON_H
#define COMMON_H

#include <cmath>
#include <inttypes.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>

#include "basesystem.h"
#include "core.h"
#include "vpu.h"

#define SCR_X 320
#define SCR_Y 240
// #define SCR_X 640
// #define SCR_Y 480

#define NEAR_Z 16
#define TAN_FOV_DIV_2 0.5
#define FOV_FACTOR (int) ( ( SCR_X / 2 ) / TAN_FOV_DIV_2 )
#define CLOCKS_PER_SEC ( 10000000 )
#define CLOCKS_PER_FRAME ( CLOCKS_PER_SEC / 60 )

#define PCFONT_CX 9
#define PCFONT_CY 16
#define PCFONT_TW 32
#define PCFONT_TH 8

#define CON_W 36
#define CON_H 16

extern struct EVideoContext g_vx;
extern struct EVideoSwapContext g_sc;

extern uint32_t windowBoundary;
extern uint8_t *g_renderWindow;

extern void set320x240( void );
extern void set640x480( void );
extern void set640x480FloatColor( void );
extern void allocVideo16bit( void );

extern int waitAndSwap( int minFrames = 1 );

typedef struct
{
    void *base;
    void *data;
    uint32_t size;
    uint32_t sizeAvail;
} Heap;

extern Heap g_commonHeap;

extern void heapInit( Heap *h, void *data, uint32_t size );
extern void *heapAlloc( Heap *h, uint32_t size );
extern void heapReset( Heap *h );
extern void clearScreen( void );
extern void clearScreens( void );

extern void allocRenderWindow( void );
extern void setRenderWindowBoundary( uint8_t marker1, uint8_t marker2 );

extern void setPal( uint8_t idx, uint8_t r, uint8_t g, uint8_t b );
extern void setBright( int bright, int dir );
extern bool doneFade( void );
extern void waitFade( void );

extern void triggerFadeDown( int rate );
extern bool fadeDownDone( void );

extern void blit( uint8_t *src, int32_t sx, int32_t sy, int32_t x, int32_t y );
extern void blitXP( uint8_t *src, int32_t sx, int32_t sy, int32_t x, int32_t y );
extern void blitXPForceCol( uint8_t *src, int32_t sx, int32_t sy, int32_t x, int32_t y, uint8_t forceCol );
extern void blit640( uint8_t *src, int32_t sx, int32_t sy, int32_t x, int32_t y );

extern void blendColors( int r1, int g1, int b1, int r2, int g2, int b2, int base, int ct );
extern void loadPalette( uint8_t *pal, int ct, int base );
extern void DMA( uint32_t src, uint32_t dest, uint32_t length );

#define PROFILE_ENABLED 0

#if PROFILE_ENABLED
extern void profileStart( void );
extern void profileCol( uint8_t col );
extern void profileEnd( void );
extern void profileDisplay( void );
#else
inline void profileStart( void )
{
}
inline void profileCol( uint8_t col ) {}
inline void profileEnd( void ) {}
inline void profileDisplay( void ) {}
#endif

extern void decompress( uint8_t *src, uint32_t srcBytes, uint8_t *dest, uint32_t *destBytes );

#endif

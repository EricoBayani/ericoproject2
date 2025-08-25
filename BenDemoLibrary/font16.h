#ifndef FONT16_H
#define FONT16_H

#include "commondemo.h"

extern void set16x16Pal( uint32_t scale = 256 );
extern void renderString16x16( const char *s, int px, int py, uint8_t forceCol = 0 );
extern bool renderScrollText( const char *s, int ctr, int y );
extern void renderCentered( const char *str, int32_t y, uint8_t forceCol = 0xff );
extern bool renderWibblyScrollText( const char *s, int ctr, int y );

#endif
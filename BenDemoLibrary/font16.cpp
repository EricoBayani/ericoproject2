#include "font16.h"
#include "font16x16.inl"

void set16x16Pal( uint32_t scale )
{
    for ( int i = 0; i < 16; i++ )
    {
        setPal( i + 0xf0,
                ( scale * s_demofontPal[i * 3 + 0] ) >> 8,
                ( scale * s_demofontPal[i * 3 + 1] ) >> 8,
                ( scale * s_demofontPal[i * 3 + 2] ) >> 8 );
    }
}

void renderString16x16( const char *s, int px, int py, uint8_t forceCol )
{
    while ( char c = *( s++ ) )
    {
        if ( c >= 32 && c < 96 )
        {
            uint8_t *src = s_demofontLetterTable[c - 32];
            if ( !forceCol )
            {
                blitXP( src, 16, 16, px, py );
            }
            else
            {
                blitXPForceCol( src, 16, 16, px, py, forceCol );
            }
        }
        px += 16;
    }
}

bool renderScrollText( const char *s, int ctr, int y )
{
    int off = -ctr;
    while ( char c = *( s++ ) )
    {
        if ( off >= -16 )
        {
            uint8_t *src = s_demofontLetterTable[c - 32];
            blitXP( src, 16, 16, off, y );
        }
        off += 16;
        if ( off >= SCR_X )
        {
            return ( false );
        }
    }
    printf( "we're done?\n" );

    return ( true );
}

bool renderWibblyScrollText( const char *s, int ctr, int y )
{
    int off = -ctr;
    int l = strlen( s );

    int skip = ctr / 16;
    if ( skip >= l )
        return ( true );
    s += skip;
    off += skip * 16;

    while ( char c = *( s++ ) )
    {
        int dx = int( sinf( off / 20.0f ) * 5 );
        int dy = int( cosf( off / 25.0f ) * 5 );
        int toff = off + dx;

        if ( toff >= -16 )
        {
            uint8_t *src = s_demofontLetterTable[c - 32];
            blitXP( src, 16, 16, toff, y + dy );
        }
        off += 16;
        if ( off >= SCR_X )
        {
            return ( false );
        }
    }
    printf( "we're done?\n" );

    return ( true );
}

void renderCentered( const char *str, int32_t y, uint8_t forceCol )
{
    int l = strlen( str );
    renderString16x16( str, 160 - l * 8, y, forceCol );
}

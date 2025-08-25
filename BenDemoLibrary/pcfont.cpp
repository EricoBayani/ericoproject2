#include "commondemo.h"

#include "pcfont.inl"

uint16_t g_console[CON_W * CON_H];
uint8_t g_consoleCursorX = 0;
uint8_t g_consoleCursorY = 0;
uint8_t g_cursorEnabled = 0;
uint8_t g_consoleLeft = 0;
uint8_t g_frameCtr;

extern bool onlyPixels;
extern bool onlyText;
extern bool bothPixelsText;

void consolePrintAt( const char *str, int cx, int cy, int fg, int bg )
{
    uint16_t *w = &g_console[cx + CON_W * cy];
    uint16_t col = ( ( fg & 0xf ) << 12 ) | ( ( bg & 0xf ) << 8 );
    while ( char c = *( str++ ) )
    {
        *( w++ ) = ( col | c );
    }
}

void consoleClear( void )
{
    memset( g_console, 0, sizeof( g_console ) );
    g_consoleCursorX = 0;
    g_consoleCursorY = 0;
}

void consolePrint( const char *str, int fg = 7, int bg = 0 )
{
    uint16_t col = ( ( fg & 0xf ) << 12 ) | ( ( bg & 0xf ) << 8 );

    while ( char c = *( str++ ) )
    {
        if ( c != '\n' )
        {
            g_console[g_consoleCursorX + g_consoleCursorY * CON_W] = ( c | col );
            g_consoleCursorX += 1;
        }
        else
        {
            g_consoleCursorX = CON_W;
        }
        if ( g_consoleCursorX == CON_W )
        {
            g_consoleCursorX = g_consoleLeft;
            g_consoleCursorY += 1;
            if ( g_consoleCursorY == CON_H )
            {
                memmove( g_console, &g_console[CON_W], CON_W * ( CON_H - 1 ) * 2 );
                memset( &g_console[CON_W * ( CON_H - 1 )], 0, CON_W * 2 );
                g_consoleCursorY -= 1;
            }
        }
    }
}

void consolePrintChar( char c, int rep = 1, int fg = 7, int bg = 0 )
{
    char t[2];
    t[0] = c;
    t[1] = 0;
    while ( rep-- )
    {
        consolePrint( t, fg, bg );
    }
}

void consolePrintEsc( const char *s )
{
    char tmp[256];
    strcpy( tmp, s );
    for ( int i = 0; tmp[i]; i++ )
    {
        if ( tmp[i] == '|' )
            tmp[i] = 0xba;
        if ( tmp[i] == '@' )
            tmp[i] = 0xac;
    }
    consolePrint( tmp );
}

void renderPCFontChar( char c, int px, int py, int fgCol, int bgCol )
{
    uint8_t *src;
    uint8_t *dest;
    uint32_t stride = ( g_vx.m_strideInWords * 4 );

    src = &pcfont_raw[( ( c >> 5 ) * PCFONT_CY * ( PCFONT_TW * PCFONT_CX ) ) + ( ( c & 0x1f ) * PCFONT_CX )];
    dest = &g_sc.writepage[( py * stride ) + px];

    for ( int y = 0; y < PCFONT_CY; y++ )
    {
        for ( int x = 0; x < PCFONT_CX; x++ )
        {
            if ( *( src++ ) )
            {
                *( dest++ ) = fgCol;
            }
            else if ( bgCol >= 0 )
            {
                *( dest++ ) = bgCol;
            }
            else
            {
                dest++;
            }
        }
        src += ( PCFONT_CX * ( PCFONT_TW - 1 ) );
        dest += stride - PCFONT_CX;
    }
}

void renderPCFontStr( const char *str, int x, int y, int fgCol, int bgCol )
{
    while ( char c = *( str++ ) )
    {
        renderPCFontChar( c, x, y, fgCol, bgCol );
        x += PCFONT_CX;
    }
}

void renderConsole640( void )
{
    uint8_t *w = g_sc.writepage;
    uint8_t *bwp = w;
    uint8_t *rwp = g_renderWindow;

    uint16_t *cp = g_console;
    uint16_t bb;

    for ( int y = 0; y < CON_H; y++ )
    {
        // audioTick();
        for ( int sy = 0; sy < 16; sy++ )
        {
            for ( int x = 0; x < CON_W; x++ )
            {
                uint16_t ch = *( cp++ );
                uint8_t c = ch & 0xff;
                uint8_t *src;
                uint8_t bg = ( ch >> 8 ) & 0xf;
                uint8_t fg = ( ch >> 12 ) & 0xf;

                src = &pcfont_raw[( ( c >> 5 ) * PCFONT_CY * ( PCFONT_TW * PCFONT_CX ) ) + ( ( c & 0x1f ) * PCFONT_CX )];
                src += ( sy * ( PCFONT_TW * PCFONT_CX ) );

                for ( int sx = 0; sx < 9; sx++ )
                {
                    uint8_t col = *( src++ ) ? fg : bg;
                    int windowOffset = w - bwp;
                    if ( onlyText )
                    {
                        *( w++ ) = col;
                    }
                    else if ( onlyText )
                    {
                        w++;
                    }
                    else
                    {
                        if ( rwp[windowOffset] == 2 )
                            *( w++ ) = col;
                        else
                            w++;
                    }
                }
            }
            // w[-1] = 0xff;
            // *w = 0xff;
            cp -= CON_W;
            w += g_vx.m_graphicsWidth - ( CON_W * PCFONT_CX );
        }
        cp += CON_W;
    }

    g_frameCtr += 1;
    if ( g_cursorEnabled && !( g_frameCtr & 2 ) )
    {
        renderPCFontChar( (char) 0x16, g_consoleCursorX * PCFONT_CX, g_consoleCursorY * PCFONT_CY, 7, -1 );
    }
}

uint8_t g_pcFontPal[] = {
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0xc,
    0x0,
    0xc,
    0x0,
    0x0,
    0xc,
    0x8,
    0xc,
    0x0,
    0x0,
    0xc,
    0x0,
    0xc,
    0xc,
    0xc,
    0x0,
    0xc,
    0xc,
    0xc,

    0x8,
    0x8,
    0x8,
    0x0,
    0x0,
    0xf,
    0x0,
    0xf,
    0x0,
    0x0,
    0xf,
    0x8,
    0xf,
    0x0,
    0x0,
    0xf,
    0x0,
    0xc,
    0xf,
    0xf,
    0x0,
    0xf,
    0xf,
    0xf,
};

void wait( int ct, bool logo = false )
{
    ct = ( ct * 2 / 3 );
    if ( !ct )
        ct++;
    while ( ct-- )
    {
        renderConsole640();

        waitAndSwap( 4 );
    }
}

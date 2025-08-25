#include "poly.h"

uint8_t *g_tex;

char *g_lumText = ".,~:'+*@";

extern float g_scrToConW;
extern float g_scrToConH;
extern float g_slopecrToCon;

extern int32_t g_ctr;

extern uint16_t g_console[];
extern void consolePrintAt( const char *str, int cx, int cy, int fg, int bg );

uint32_t g_lumTextLength = 8;

extern bool onlyPixels;
extern bool onlyText;
extern bool bothPixelsText;

void setEnvTex( uint8_t *tex )
{
    g_tex = tex;
}

void renderQuadText( int32_t y1, int32_t y2, int32_t tl, int32_t tr, int32_t dl, int32_t dr, uint32_t col )
{

    char temp[2] = { 0 };

    for ( ; y1 < y2; y1++ )
    {
        int32_t ll = tl;
        int32_t rr = tr;
        if ( ll < 0 )
            ll = 0;
        if ( rr >= CON_W )
            rr = CON_W;

        while ( rr-- >= ll )
        {
            temp[0] = g_lumText[col >> 5];

            consolePrintAt( temp, rr, y1, 7, 0 );
        }
        tl += dl;
        tr += dr;
    }
}

void renderQuad( int32_t y1, int32_t y2, int32_t tl, int32_t tr, int32_t dl, int32_t dr, uint32_t col )
{

    if ( y1 < 0 )
    {
        tl += dl * ( -y1 );
        tr += dr * ( -y1 );
        y1 = 0;
    }
    if ( y2 >= SCR_Y )
    {
        y2 = SCR_Y - 1;
    }

    uint8_t *sp;
    uint8_t *rwp = NULL;

    tl += dl >> 1;
    tr += dr >> 1;

    uint8_t *gbp = g_sc.writepage;
    sp = &g_sc.writepage[y1 * SCR_X];
    rwp = g_renderWindow;
    for ( ; y1 < y2; y1++ )
    {
        int32_t ll = tl >> 16;
        int32_t rr = tr >> 16;
        if ( ll < 0 )
            ll = 0;
        if ( rr >= SCR_X )
            rr = SCR_X;
        uint8_t *p = sp + ll;
        rr -= ll;
        while ( rr-- > 0 )
        {
            if ( onlyPixels )
            {
                *( p++ ) = col;
            }
            else if ( onlyText )
            {
                p++;
            }
            else
            {
                int windowOffset = p - gbp;
                if ( rwp[windowOffset] == 1 )
                {
                    *( p++ ) = col;
                }
                else
                {
                    p++;
                }
            }
        }
        tl += dl;
        tr += dr;
        sp += SCR_X;
    }
}

void renderSortedTriText( int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t col )
{

    y3 = (int) ( y3 * g_scrToConH );
    y2 = (int) ( y2 * g_scrToConH );
    y1 = (int) ( y1 * g_scrToConH );

    x3 = (int) ( x3 * g_scrToConW );
    x2 = (int) ( x2 * g_scrToConW );
    x1 = (int) ( x1 * g_scrToConW );

    int dxl, dxr;
    int h = y3 - y1;
    if ( h == 0 )
    {
        return;
    }

    if ( y1 == y2 )
    {
        dxl = ( x3 - x1 ) / h;
        dxr = ( x3 - x2 ) / h;
        if ( x1 < x2 )
        {
            renderQuadText( y1, y3, x1, x2, dxl, dxr, col );
        }
        else
        {
            renderQuadText( y1, y3, x2, x1, dxr, dxl, col );
        }
    }
    else if ( y2 == y3 )
    {
        dxl = ( x2 - x1 ) / h;
        dxr = ( x3 - x1 ) / h;
        if ( x2 < x3 )
        {
            renderQuadText( y1, y3, x1, x1, dxl, dxr, col );
        }
        else
        {
            renderQuadText( y1, y3, x1, x1, dxr, dxl, col );
        }
    }
    else
    {
        int32_t h21 = ( y2 - y1 );
        dxl = ( x2 - x1 ) / h21;
        dxr = ( x3 - x1 ) / h;
        if ( dxl < dxr )
        {
            renderQuadText( y1, y2, x1, x1, dxl, dxr, col );
            dxl = ( x3 - x2 ) / ( y3 - y2 );
            renderQuadText( y2, y3, x2, x1 + ( dxr * h21 ), dxl, dxr, col );
        }
        else
        {
            renderQuadText( y1, y2, x1, x1, dxr, dxl, col );
            dxl = ( x3 - x2 ) / ( y3 - y2 );
            renderQuadText( y2, y3, x1 + ( dxr * h21 ), x2, dxr, dxl, col );
        }
    }
}

void renderSortedTri( int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t col )
{

    renderSortedTriText( x1, y1, x2, y2, x3, y3, col );

    int32_t dxl, dxr;
    int32_t h = y3 - y1;
    if ( h == 0 )
    {
        return;
    }
    x1 <<= 16;
    x2 <<= 16;
    x3 <<= 16;
    if ( y1 == y2 )
    {
        dxl = ( x3 - x1 ) / h;
        dxr = ( x3 - x2 ) / h;
        if ( x1 < x2 )
        {
            renderQuad( y1, y3, x1, x2, dxl, dxr, col );
        }
        else
        {
            renderQuad( y1, y3, x2, x1, dxr, dxl, col );
        }
    }
    else if ( y2 == y3 )
    {
        dxl = ( x2 - x1 ) / h;
        dxr = ( x3 - x1 ) / h;
        if ( x2 < x3 )
        {
            renderQuad( y1, y3, x1, x1, dxl, dxr, col );
        }
        else
        {
            renderQuad( y1, y3, x1, x1, dxr, dxl, col );
        }
    }
    else
    {
        int32_t h21 = ( y2 - y1 );
        dxl = ( x2 - x1 ) / h21;
        dxr = ( x3 - x1 ) / h;
        if ( dxl < dxr )
        {
            renderQuad( y1, y2, x1, x1, dxl, dxr, col );
            dxl = ( x3 - x2 ) / ( y3 - y2 );
            renderQuad( y2, y3, x2, x1 + ( dxr * h21 ), dxl, dxr, col );
        }
        else
        {
            renderQuad( y1, y2, x1, x1, dxr, dxl, col );
            dxl = ( x3 - x2 ) / ( y3 - y2 );
            renderQuad( y2, y3, x1 + ( dxr * h21 ), x2, dxr, dxl, col );
        }
    }
}

void renderTri( int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t col )
{
    if ( y1 <= y2 )
    {
        if ( y1 <= y3 )
        {
            if ( y2 <= y3 )
            {
                renderSortedTri( x1, y1, x2, y2, x3, y3, col );
            }
            else
            {
                renderSortedTri( x1, y1, x3, y3, x2, y2, col );
            }
        }
        else
        {
            renderSortedTri( x3, y3, x1, y1, x2, y2, col );
        }
    }
    else
    {
        if ( y2 <= y3 )
        {
            if ( y1 <= y3 )
            {
                renderSortedTri( x2, y2, x1, y1, x3, y3, col );
            }
            else
            {
                renderSortedTri( x2, y2, x3, y3, x1, y1, col );
            }
        }
        else
        {
            renderSortedTri( x3, y3, x2, y2, x1, y1, col );
        }
    }
}

void renderTexQuad( int32_t y1, int32_t y2,
                    int32_t xl, int32_t xr,
                    int32_t dxl, int32_t dxr,
                    int32_t u, int32_t v,
                    int32_t dux, int32_t dvx,
                    int32_t duy, int32_t dvy )
{
    if ( y1 < 0 )
    {
        xl += dxl * ( -y1 );
        xr += dxr * ( -y1 );
        u += duy * ( -y1 );
        v += dvy * ( -y1 );
        y1 = 0;
    }
    if ( y2 >= SCR_Y )
    {
        y2 = SCR_Y - 1;
    }

    uint8_t *sp;
    sp = &g_sc.writepage[y1 * SCR_X];

    xl += dxl >> 1;
    xr += dxr >> 1;
    u += duy >> 1;
    v += dvy >> 1;

    for ( ; y1 < y2; y1++ )
    {
        int32_t ll = xl >> 16;
        int32_t rr = xr >> 16;
        int32_t uu = u;
        int32_t vv = v;
        if ( ll < 0 )
        {
            uu += dux * ( -ll );
            vv += dvx * ( -ll );
        }
        if ( rr >= SCR_X )
            rr = SCR_X;
        uint8_t *p = sp + ll;
        rr -= ll;
        while ( rr-- > 0 )
        {
            uint32_t i = ( ( uu >> 8 ) & 0xff00 ) | ( ( vv >> 16 ) & 0x00ff );
            *( p++ ) = g_tex[i];
            uu += dux;
            vv += dvx;
        }
        xl += dxl;
        xr += dxr;
        u += duy;
        v += dvy;
        sp += SCR_X;
    }
}

void renderSortedTexTri( TexVert *p1, TexVert *p2, TexVert *p3 )
{
    TexVert *tmp;
    int32_t dxl, dxr;
    int32_t uu, vv;
    int32_t x1, x2, x3;
    int32_t dux, dvx;
    int32_t duy, dvy;
    int32_t w;
    int32_t h = p3->y - p1->y;

    if ( h == 0 )
    {
        return;
    }
    if ( p1->y == p2->y )
    {
        w = p2->x - p1->x;
        if ( !w )
            return;
        if ( w < 0 )
        {
            tmp = p1;
            p1 = p2;
            p2 = tmp;
            w = -w;
        }
        x1 = p1->x << 16;
        x2 = p2->x << 16;
        x3 = p3->x << 16;
        dxl = ( x3 - x1 ) / h;
        dxr = ( x3 - x2 ) / h;
        dux = ( p2->u - p1->u ) / w;
        dvx = ( p2->v - p1->v ) / w;
        duy = ( p3->u - p1->u ) / h;
        dvy = ( p3->v - p1->v ) / h;
        uu = p1->u;
        vv = p1->v;
        renderTexQuad( p1->y, p3->y, x1, x2, dxl, dxr, uu, vv, dux, dvx, duy, dvy );
    }
    else if ( p2->y == p3->y )
    {
        w = p3->x - p2->x;
        if ( !w )
            return;
        if ( w < 0 )
        {
            tmp = p3;
            p3 = p2;
            p2 = tmp;
            w = -w;
        }
        x1 = p1->x << 16;
        x2 = p2->x << 16;
        x3 = p3->x << 16;
        dxl = ( x2 - x1 ) / h;
        dxr = ( x3 - x1 ) / h;
        dux = ( p3->u - p2->u ) / w;
        dvx = ( p3->v - p2->v ) / w;
        duy = ( p2->u - p1->u ) / h;
        dvy = ( p2->v - p1->v ) / h;
        uu = p1->u;
        vv = p1->v;
        renderTexQuad( p1->y, p3->y, x1, x1, dxl, dxr, uu, vv, dux, dvx, duy, dvy );
    }
    else
    {
        int32_t x1 = p1->x << 16;
        int32_t x2 = p2->x << 16;
        int32_t x3 = p3->x << 16;
        int32_t h21 = ( p2->y - p1->y );
        int32_t nx, nu, nv;
        uu = p1->u;
        vv = p1->v;
        dxl = ( x2 - x1 ) / h21;
        dxr = ( x3 - x1 ) / h;
        if ( dxl < dxr )
        {
            nx = x1 + ( dxr * h21 );
            nu = p1->u + ( ( ( p3->u - p1->u ) * h21 ) / h );
            nv = p1->v + ( ( ( p3->v - p1->v ) * h21 ) / h );
            w = ( nx >> 16 ) - p2->x;
            dux = ( nu - p2->u ) / w;
            dvx = ( nv - p2->v ) / w;
            duy = ( p2->u - p1->u ) / h21;
            dvy = ( p2->v - p1->v ) / h21;
            renderTexQuad( p1->y, p2->y, x1, x1, dxl, dxr, uu, vv, dux, dvx, duy, dvy );
            h = p3->y - p2->y;
            duy = ( p3->u - p2->u ) / h;
            dvy = ( p3->v - p2->v ) / h;
            dxl = ( x3 - x2 ) / h;
            uu = p2->u;
            vv = p2->v;
            //            dxr = ( x3 - nx ) / h;
            renderTexQuad( p2->y, p3->y, x2, nx, dxl, dxr, uu, vv, dux, dvx, duy, dvy );
        }
        else
        {
            nx = x1 + ( dxr * h21 );
            nu = p1->u + ( ( ( p3->u - p1->u ) * h21 ) / h );
            nv = p1->v + ( ( ( p3->v - p1->v ) * h21 ) / h );
            w = p2->x - ( nx >> 16 );
            dux = ( p2->u - nu ) / w;
            dvx = ( p2->v - nv ) / w;
            duy = ( p3->u - p1->u ) / h;
            dvy = ( p3->v - p1->v ) / h;
            renderTexQuad( p1->y, p2->y, x1, x1, dxr, dxl, uu, vv, dux, dvx, duy, dvy );
            h = p3->y - p2->y;
            // duy = ( p3->u - p2->u ) / h;
            // dvy = ( p3->v - p2->v ) / h;
            uu = nu;
            vv = nv;
            dxl = ( x3 - x2 ) / h;
            //            dxr = ( x3 - nx ) / h;
            renderTexQuad( p2->y, p3->y, nx, x2, dxr, dxl, uu, vv, dux, dvx, duy, dvy );
        }
    }
}

void renderTexTri( TexVert *p1, TexVert *p2, TexVert *p3 )
{
    if ( p1->y <= p2->y )
    {
        if ( p1->y <= p3->y )
        {
            if ( p2->y <= p3->y )
            {
                renderSortedTexTri( p1, p2, p3 );
            }
            else
            {
                renderSortedTexTri( p1, p3, p2 );
            }
        }
        else
        {
            renderSortedTexTri( p3, p1, p2 );
        }
    }
    else
    {
        if ( p2->y <= p3->y )
        {
            if ( p1->y <= p3->y )
            {
                renderSortedTexTri( p2, p1, p3 );
            }
            else
            {
                renderSortedTexTri( p2, p3, p1 );
            }
        }
        else
        {
            renderSortedTexTri( p3, p2, p1 );
        }
    }
}

void polyTest( void )
{

    while ( true )
    {
        g_sc.writepage = g_sc.writepage;

        uint32_t *wptr = (uint32_t *) g_sc.writepage;
        setPal( 240, 0x0, 0xf, 0x0 );
        for ( int j = 0; j < 240; j++ )
        {
            *( wptr++ ) = 0xf0f0f0f0;
            memset( wptr, 0, 320 - 4 );
            wptr += ( 320 - 4 ) / 4;
        }
        setPal( 240, 0xf, 0x0, 0xf );

        for ( int i = 0; i < 300; i++ )
        {
            int a = ( rand() % 290 ) + 5;
            int b = ( rand() % 220 ) + 5;
            renderTri( a, b, a + 10, b + 10, a - 5, b + 5, i );
        }
        setPal( 240, 0xf, 0xf, 0xf );

        waitAndSwap();
    }
}

void texSpan( uint8_t *wptr, uint32_t uu, uint32_t vv, uint32_t du, uint32_t dv, uint8_t *tex, uint32_t len )
{
    uint8_t *end = wptr + len;
    while ( wptr != end )
    {
        uint32_t i = ( ( uu >> 8 ) & 0xff00 ) | ( ( vv >> 16 ) & 0x00ff );
        *( wptr++ ) = tex[i];
        uu += du;
        vv += dv;
    }
}

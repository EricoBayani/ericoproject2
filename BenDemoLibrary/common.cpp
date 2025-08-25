#include "commondemo.h"

static uint8_t *s_framebufferA = NULL;
static uint8_t *s_framebufferB = NULL;

struct EVideoContext g_vx;
struct EVideoSwapContext g_sc;

uint32_t windowBoundary = SCR_X / 2;

#define MAX_PROFILES 16
class Profile
{
  public:
    uint32_t profileTimes[MAX_PROFILES];
    uint8_t profileCols[MAX_PROFILES];
    int profileCt;
    uint32_t profileLastFrameTime;
};

Profile s_current;
Profile s_prev;
bool s_fadeDown = false;

Heap g_commonHeap;
uint8_t *g_renderWindow = NULL;

void allocVideo( void )
{
    if ( !s_framebufferA )
    {
        s_framebufferA = VPUAllocateBuffer( 640 * 480 );
        s_framebufferB = VPUAllocateBuffer( 640 * 480 );

        // printf( "ALLOC VIDEO %p %p\n", s_framebufferA, s_framebufferB );
    }
}

void allocVideo16bit( void )
{

    s_framebufferA = VPUAllocateBuffer( 640 * 480 * 2 );
    s_framebufferB = VPUAllocateBuffer( 640 * 480 * 2 );

    // printf( "ALLOC VIDEO %p %p\n", s_framebufferA, s_framebufferB );
}

void set320x240( void )
{
    allocVideo();

    g_vx.m_vmode = EVM_320_Wide;
    g_vx.m_cmode = ECM_8bit_Indexed;
    VPUSetVMode( &g_vx, EVS_Enable );

    g_sc.cycle = 0;
    g_sc.framebufferA = s_framebufferA;
    g_sc.framebufferB = s_framebufferB;
    VPUSwapPages( &g_vx, &g_sc );
}

void set640x480( void )
{
    allocVideo();

    g_vx.m_vmode = EVM_640_Wide;
    g_vx.m_cmode = ECM_8bit_Indexed;
    VPUSetVMode( &g_vx, EVS_Enable );

    g_sc.cycle = 0;
    g_sc.framebufferA = s_framebufferA;
    g_sc.framebufferB = s_framebufferB;
    VPUSwapPages( &g_vx, &g_sc );
}

void set640x480FloatColor( void )
{
    allocVideo16bit();

    g_vx.m_vmode = EVM_640_Wide;
    g_vx.m_cmode = ECM_16bit_RGB;
    VPUSetVMode( &g_vx, EVS_Enable );

    g_sc.cycle = 0;
    g_sc.framebufferA = s_framebufferA;
    g_sc.framebufferB = s_framebufferB;
    VPUSwapPages( &g_vx, &g_sc );
}

void heapInit( Heap *h, void *data, uint32_t size )
{
    h->base = data;
    while ( (uint32_t) h->base & 0xf )
    {
        h->base = (void *) ( (uint32_t *) h->base ) + 1;
        size -= 1;
    }

    h->size = size;
    memset( data, 0, size );
    heapReset( h );
}

void *heapAlloc( Heap *h, uint32_t size )
{
    size = ( size + 15 ) & ( -16 );

    // printf( "heapAlloc: %lu, %lu avail\n", size, h->sizeAvail );
    if ( h->sizeAvail < size )
    {
        printf( "out of heap\n" );
        exit( -1 );
    }
    void *q = h->data;
    h->data = ( (uint8_t *) h->data ) + size;
    h->sizeAvail -= size;

    memset( q, 0xaa, size );

    return ( q );
}

void heapReset( Heap *h )
{
    h->data = h->base;
    h->sizeAvail = h->size;
}

void clearScreen( void )
{
    memset( g_sc.writepage, 0x00, g_vx.m_graphicsHeight * g_vx.m_graphicsWidth );
}

void clearScreens( void )
{
    for ( int q = 0; q < 2; q++ )
    {
        memset( g_sc.writepage, 0, 640 * 480 );
        CFLUSH_D_L1;
        VPUSwapPages( &g_vx, &g_sc );
    }
}

void allocRenderWindow()
{
    g_renderWindow = (uint8_t *) heapAlloc( &g_commonHeap, 640 * 480 );
}

void setRenderWindowBoundary( uint8_t markerLeft, uint8_t markerRight )
{
    for ( int i = 0; i < SCR_Y; ++i )
    {
        for ( int j = 0; j < SCR_X; ++j )
        {
            if ( j < SCR_X / 2 )
                g_renderWindow[( i * SCR_X ) + j] = markerLeft;
            else
                g_renderWindow[( i * SCR_X ) + j] = markerRight;
        }
    }
}

static uint8_t s_pal[256][3];
static int s_bright = 256;
static int s_brightDir = 0;

void setPal( uint8_t idx, uint8_t r, uint8_t g, uint8_t b )
{
    s_pal[idx][0] = r;
    s_pal[idx][1] = g;
    s_pal[idx][2] = b;

    VPUSetPal( idx, ( r * s_bright ) >> 8, ( g * s_bright ) >> 8, ( b * s_bright ) >> 8 );
}

void setBright( int bright, int dir )
{
    s_bright = bright;
    s_brightDir = dir;
    s_fadeDown = false;
}

bool doneFade( void )
{
    return ( s_brightDir == 0 );
}

void tickPal( int steps = 1 )
{
    if ( !s_brightDir )
    {
        return;
    }
    while ( steps-- )
    {
        s_bright += s_brightDir;
        if ( s_bright < 0 )
        {
            s_bright = 0;
            s_brightDir = 0;
        }
        else if ( s_bright >= 256 )
        {
            s_bright = 256;
            s_brightDir = 0;
        }
    }
    for ( int j = 0; j < 256; j++ )
    {
        VPUSetPal( j, s_bright * s_pal[j][0] >> 8, s_bright * s_pal[j][1] >> 8, s_bright * s_pal[j][2] >> 8 );
    }
}

static uint64_t s_lastFrameTime = 0;

int waitAndSwap( int minFrames )
{
    uint64_t frameTime, elapsed;
    int32_t frameCt;
    int32_t totalFrameCt = 0;

    CFLUSH_D_L1;

    while ( minFrames > 0 )
    {
        VPUWaitVSync();

        frameTime = E32ReadTime();
        elapsed = frameTime - s_lastFrameTime;
        s_lastFrameTime = frameTime;

        // estimate # of frames elapsed
        frameCt = ( elapsed + ( CLOCKS_PER_FRAME / 10 ) ) / CLOCKS_PER_FRAME;
        if ( frameCt > 4 )
        {
            frameCt = 4;
        }
        minFrames -= frameCt;
        totalFrameCt += frameCt;
        //        printf( "  %d\n", frameCt );
    }

    //    printf( "waited %d frames\n", totalFrameCt );

    VPUSwapPages( &g_vx, &g_sc );
    s_current.profileLastFrameTime = E32ReadTime();
    tickPal( totalFrameCt );

    return ( totalFrameCt );
}

void waitFade( void )
{
    while ( !doneFade() )
    {
        waitAndSwap();
    }
}

void blit( uint8_t *src, int32_t sx, int32_t sy, int32_t x, int32_t y )
{
    int stride = sx;
    if ( y < 0 )
    {
        src -= y * sx;
        sy += y;
        y = 0;
    }
    if ( y + sy > SCR_Y )
    {
        sy = SCR_Y - y;
    }
    if ( sy <= 0 )
        return;
    if ( x < 0 )
    {
        src -= x;
        sx += x;
        x = 0;
    }
    if ( x + sx > SCR_X )
    {
        sx = SCR_X - x;
    }
    if ( sx <= 0 )
        return;
    uint8_t *d = &g_sc.writepage[y * SCR_X + x];
    while ( sy-- )
    {
        int t = sx;
        while ( t-- )
        {
            *( d++ ) = *( src++ );
        }
        d += SCR_X - sx;
        src += stride - sx;
    }
}

void blitXP( uint8_t *src, int32_t sx, int32_t sy, int32_t x, int32_t y )
{
    int stride = sx;

    if ( y < 0 )
    {
        src -= y * sx;
        sy += y;
        y = 0;
    }
    if ( y + sy > SCR_Y )
    {
        sy = SCR_Y - y;
    }
    if ( sy <= 0 )
        return;
    if ( x < 0 )
    {
        src -= x;
        sx += x;
        x = 0;
    }
    if ( x + sx > SCR_X )
    {
        sx = SCR_X - x;
    }
    if ( sx <= 0 )
        return;
    uint8_t *d = &g_sc.writepage[y * SCR_X + x];
    while ( sy-- )
    {
        int t = sx;
        while ( t-- )
        {
            uint8_t q = ( *src++ );
            if ( q )
            {
                *d = q;
            }
            d++;
        }
        d += SCR_X - sx;
        src += stride - sx;
    }
}

void blitXPForceCol( uint8_t *src, int32_t sx, int32_t sy, int32_t x, int32_t y, uint8_t forceCol )
{
    int stride = sx;

    if ( y < 0 )
    {
        src -= y * sx;
        sy += y;
        y = 0;
    }
    if ( y + sy > SCR_Y )
    {
        sy = SCR_Y - y;
    }
    if ( sy <= 0 )
        return;
    if ( x < 0 )
    {
        src -= x;
        sx += x;
        x = 0;
    }
    if ( x + sx > SCR_X )
    {
        sx = SCR_X - x;
    }
    if ( sx <= 0 )
        return;
    uint8_t *d = &g_sc.writepage[y * SCR_X + x];
    while ( sy-- )
    {
        int t = sx;
        while ( t-- )
        {
            uint8_t q = ( *src++ );
            if ( q )
            {
                *d = forceCol;
            }
            d++;
        }
        d += SCR_X - sx;
        src += stride - sx;
    }
}

void blit640( uint8_t *src, int32_t sx, int32_t sy, int32_t x, int32_t y )
{
    int stride = sx;
    if ( y < 0 )
    {
        src -= y * sx;
        sy += y;
        y = 0;
    }
    if ( y + sy > 480 )
    {
        sy = 480 - y;
    }
    if ( sy <= 0 )
        return;
    if ( x < 0 )
    {
        src -= x;
        sx += x;
        x = 0;
    }
    if ( x + sx > 640 )
    {
        sx = 640 - x;
    }
    if ( sx <= 0 )
        return;
    uint8_t *d = &g_sc.writepage[y * 640 + x];
    while ( sy-- )
    {
        int t = sx;
        while ( t-- )
        {
            *( d++ ) = *( src++ );
        }
        d += 640 - sx;
        src += stride - sx;
    }
}

void blendColors( int r1, int g1, int b1, int r2, int g2, int b2, int base, int ct )
{
    for ( int i = 0; i < ct; i++ )
    {
        int r = r1 + ( ( r2 - r1 ) * i / ( ct - 1 ) );
        int g = g1 + ( ( g2 - g1 ) * i / ( ct - 1 ) );
        int b = b1 + ( ( b2 - b1 ) * i / ( ct - 1 ) );
        setPal( base + i, r >> 4, g >> 4, b >> 4 );
    }
}

void loadPalette( uint8_t *pal, int ct, int base )
{
    for ( int i = 0; i < ct; i++ )
    {
        setPal( i + base, pal[i * 3 + 0], pal[i * 3 + 1], pal[i * 3 + 2] );
    }
}

#if PROFILE_ENABLED
void profileStart( void )
{
    s_current.profileCt = 0;
    profileCol( 0xff );
}

void profileCol( uint8_t col )
{
    if ( s_current.profileCt < MAX_PROFILES )
    {
        s_current.profileCols[s_current.profileCt] = col;
        s_current.profileTimes[s_current.profileCt] = E32ReadTime();
        s_current.profileCt++;
    }
}
void profileEnd( void )
{
    profileCol( 0 );
    s_prev = s_current;
}

void profileDisplay( void )
{
    uint32_t frameTime = CLOCKS_PER_SEC / 60;
    uint32_t tsf = 0;
    uint32_t x, px = 0;
    for ( int i = 0; i < s_prev.profileCt; i++ )
    {
        tsf = s_prev.profileTimes[i] - s_prev.profileTimes[0];
        x = tsf / ( frameTime / 160 );
        if ( x > 320 )
            x = 320;
        for ( int j = px; j < x; j++ )
        {
            g_sc.writepage[j + 320] = s_prev.profileCols[i];
            g_sc.writepage[j + 640] = s_prev.profileCols[i];
        }
        px = x;
    }
    memset( g_sc.writepage, 0, 640 );
    setPal( 0xfe, 0xf, 0xf, 0xf );
    g_sc.writepage[160] = 0xfe;
    g_sc.writepage[160 + 320] = 0xfe;
}
#endif

void triggerFadeDown( int rate )
{
    if ( !s_fadeDown )
    {
        setBright( 255, -rate );
        s_fadeDown = true;
    }
}
bool fadeDownDone( void )
{
    if ( s_fadeDown && doneFade() )
    {
        s_fadeDown = false;
        return ( true );
    }
    return ( false );
}

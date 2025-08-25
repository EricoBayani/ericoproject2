#include "3d.h"
#include "task.h"
#include "torus.inl"

#include "funnyimage.c"
#include "spinningStuff.h"

extern void consoleClear( void );
extern void consolePrint( const char *str, int fg = 7, int bg = 0 );
extern void consolePrintChar( char c, int rep = 1, int fg = 7, int bg = 0 );
extern uint8_t g_pcFontPal[];
extern void renderConsole640( void );
extern void wait( int ct, bool logo = false );

extern void renderPCFontChar( char c, int px, int py, int fgCol, int bgCol );

extern void renderQuad( int32_t y1, int32_t y2, int32_t tl, int32_t tr, int32_t dl, int32_t dr, uint32_t col );

extern void renderCentered( const char *str, int32_t y, uint8_t forceCol = 0xff );

shapeWork *g_torus;
int32_t s_frames = 0;
static int32_t s_startTime;
int32_t g_ctr;

bool onlyPixels = true;
bool onlyText = false;
bool bothPixelsText = false;

float g_scrToConW = ( (float) CON_W ) / ( (float) SCR_X );
float g_scrToConH = ( (float) CON_H ) / ( (float) SCR_Y );
float g_slopecrToCon = (float) ( ( (float) CON_H ) / ( (float) SCR_Y ) ) / ( ( (float) SCR_Y ) / ( (float) SCR_X ) );

inline uint32_t ftoui4sat( float value )
{
    uint32_t retval;
    asm(
        "mv a1, %1;"
        ".insn 0xc2058553;" // fcvtswu4sat a0, a1 // note A0==cpu.x10, A1==cpu.x11
        "mv %0, a0; "
        : "=r"( retval )
        : "r"( value )
        : "a0", "a1" );
    return retval;
}

void initTorus()
{
    g_torus = allocShapeWork( &g_commonHeap, &shape_torus );

    blendColors( 0, 0, 0, 0, 0, 0, 0, 256 );

#define BASE_PAL_SZ 16
#define ENV_PAL_SZ 128
#define LIGHT_PAL_SZ ( 240 - ENV_PAL_SZ - BASE_PAL_SZ )
#define LIGHT_PAL_BASE ( BASE_PAL_SZ + ENV_PAL_SZ )

    blendColors( 0, 0, 0, 0, 255, 0, LIGHT_PAL_BASE, LIGHT_PAL_SZ / 2 );
    blendColors( 0, 255, 0, 255, 255, 255, LIGHT_PAL_BASE + LIGHT_PAL_SZ / 2, LIGHT_PAL_SZ / 2 );
    for ( int i = 0; i < 256; i++ )
    {
        g_torus->lightPalette[i] = LIGHT_PAL_BASE + ( ( i * LIGHT_PAL_SZ ) / 256 );
    }
    ivec3_set( &g_torus->lightVec, 0, 16000, -10000 );

    blendColors( 64, 0, 128, 128, 128, 255, 240, 15 );
    setPal( 255, 0xf, 0xf, 0xf );

    int32_t xCoord = 180;

    s_startTime = E32ReadTime();
}

static int bigI = 0;

void spinDonut( int32_t ctr )
{
    uint8_t c = ( ctr & 0xff );
    consoleClear();
    int row = 20;
    char temp[2] = { 0 };
    for ( int i = 0; i < 255; ++i )
    {
        if ( i % 30 )
            row += 8;
        consolePrintChar( i + bigI );
    }
    bigI++;
}

int iW = 640;
int iH = 480;

void renderScenePoggers( int32_t ctr )
{
    onlyPixels = true;
    clearScreens();
    setBright( 255, 16 );

    set640x480FloatColor();
    uint16_t *image = (uint16_t *) g_sc.writepage;
    uint8_t *img = (uint8_t *) gimp_image.pixel_data;
    for ( int y = 0; y < iH; ++y )
    {
        for ( int x = 0; x < iW; ++x )
        {
            uint32_t red = ftoui4sat( float( img[( x + y * iW ) * 3 + 0] ) / 255.f );
            uint32_t green = ftoui4sat( float( img[( x + y * iW ) * 3 + 1] ) / 255.f );
            uint32_t blue = ftoui4sat( float( img[( x + y * iW ) * 3 + 2] ) / 255.f );
            image[x + y * 640] = MAKECOLORRGB12( red, green, blue );
        }
    }

    waitAndSwap();
}

void renderSceneTorus( int32_t ctr )
{
    consoleClear();

    int helloGuys = 700;

    int originalAngle = 1070;
    int wowDonut = 930;

    int wowItSpins = wowDonut + 407; // original timing is 1070 + 500

    int reveal = wowItSpins + 840; // original timing is 1070 + 500 + 400

    int thanks = reveal + 800;

    int ctrOffset = ( ctr + ( originalAngle - wowDonut ) );

    if ( ctr < helloGuys )
    {
        renderCentered( "HI GUYS", 70, 0 );
        renderCentered( "HOPE YOU LIKE DONUT", 150, 0 );
    }

    else if ( ctr < wowDonut )
    {
        renderShapeSimple( g_torus, 5 + originalAngle, 20 + originalAngle / 2, 30 + originalAngle * 2, 0, 0, 60000, 0 );

        renderCentered( "WOW", 15, 0 );
        renderCentered( "FLAT SHADING", 210, 0 );
    }

    else if ( ctr < wowItSpins )
    {

        renderConsole640();
        renderShapeSimple( g_torus, ( 10 + ctrOffset ), ( 20 + ctrOffset / 2 ), ( 30 + ctrOffset * 2 ), 0, 0, 60000, 0 );

        renderCentered( "IT SPINS YO", 210, 0 );
    }

    else if ( ctr < reveal )
    {
        onlyText = true;
        onlyPixels = false;

        renderShapeSimple( g_torus, ( 10 + ctrOffset ), ( 20 + ctrOffset / 2 ), ( 30 + ctrOffset * 2 ), 0, 0, 60000, 0 );
        renderConsole640();

        renderCentered( "HOW RETRO", 15, 0 );
        renderCentered( "YET MODERN", 210, 0 );
    }

    else if ( ctr < thanks )
    {
        onlyText = false;
        onlyPixels = false;
        renderShapeSimple( g_torus, ( 10 + ctrOffset ), ( 20 + ctrOffset / 2 ), ( 30 + ctrOffset * 2 ), 0, 0, 60000, 0 );
        renderConsole640();

        renderCentered( "HOLY CANOLI", ( 15 + ctr ) % SCR_Y, 0 );
    }

    else
    {
        onlyText = false;
        onlyPixels = false;
        renderShapeSimple( g_torus, ( 10 + ctrOffset ), ( 20 + ctrOffset / 2 ), ( 30 + ctrOffset * 2 ), 0, 0, 60000, 0 );
        renderConsole640();

        renderCentered( "THANK YOU", 20, 0 );
        renderCentered( "ENGIN", 50, 0 );
        renderCentered( "BEN", 70, 0 );
        renderCentered( "AKI", 90, 0 );
        renderCentered( "CHRIS", 110, 0 );
        renderCentered( "JORDAN", 130, 0 );
        renderCentered( "WADE", 150, 0 );
        renderCentered( "WILL", 170, 0 );
    }
}

typedef void ( *scenePtr )( int32_t ctr );
class SceneDef
{
  public:
    scenePtr m_renderFunc;
    int32_t m_startFrames;
    int32_t m_totalFrames;
};

SceneDef s_sceneDefs[] = {
    { renderSceneTorus, 0, 3500 }, // original is 3500
    { NULL, 0, 0 } };

void doScene( SceneDef *def, bool fade, bool doForever )
{
    int32_t i = 0;
    while ( true )
    {
        clearScreen();
        if ( i >= def->m_totalFrames )
        {
            if ( doForever )
                i = 0;
            else if ( fade )
            {
                triggerFadeDown( 8 );
                if ( fadeDownDone() )
                {
                    break;
                }
            }
        }

        else
        {
            if ( i >= def->m_totalFrames )
            {
                break;
            }
        }
        profileStart();
        profileCol( 248 );
        g_ctr = i - def->m_startFrames;

        int32_t ctr = g_ctr;
        if ( ctr < 0 )
            ctr = 0;

        profileDisplay();
        profileCol( 255 );
        def->m_renderFunc( ctr );
        profileEnd();
        i += waitAndSwap();
    }
    printf( "Scene over\n" );
}

void spinThings( void )
{
    volatile int *sharedmem = (volatile int *) TaskGetSharedMemory();
    volatile int *s_frame1 = sharedmem + 4;

    *sharedmem = 0;
    *s_frame1 = 0;

    for ( int i = 0; i < 256; i++ )
        setPal( i, i >> 4, i >> 4, i >> 4 );

    initTorus();
    setPal( 240, 2, 4, 2 );
    setPal( 255, 0x8, 0x8, 0x8 );

    allocRenderWindow();
    setRenderWindowBoundary( 1, 2 );

    clearScreens();

    for ( int i = 0; i < 16; i++ )
    {
        setPal( i, g_pcFontPal[i * 3 + 0], g_pcFontPal[i * 3 + 1], g_pcFontPal[i * 3 + 2] );
    }

    int sceneIdx = 0;
    while ( true )
    {
        bool doForever = false;
        bool fade = !s_sceneDefs[sceneIdx + 1].m_renderFunc;
        doScene( &s_sceneDefs[sceneIdx], fade, doForever );
        sceneIdx++;
        if ( !s_sceneDefs[sceneIdx].m_renderFunc )
        {
            *sharedmem = 1;
            break;
        }
    }

    renderScenePoggers( 0 );
}
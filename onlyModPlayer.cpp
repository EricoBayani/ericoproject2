#include <cmath>
#include <complex>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apu.h"
#include "basesystem.h"
#include "core.h"
#include "task.h"
#include "vpu.h"

#include "xmp.h"

#include "onlyModPlayer.h"

#include "music.c"
#include "poggers.c"
#include "silence.c"

extern uint32_t s_musicBytesCount;
extern uint8_t s_musicBytes[];

extern uint32_t s_poggersBytesCount;
extern uint8_t s_poggersBytes[];

extern uint32_t s_silenceBytesCount;
extern uint8_t s_silenceBytes[];

xmp_context ctx;
struct xmp_module_info mi;
struct xmp_frame_info fi;

void ModInit()
{
    apubuffer = (short *) APUAllocateBuffer( BUFFER_SIZE_IN_BYTES );
    memset( apubuffer, 0, BUFFER_SIZE_IN_BYTES );

    ctx = xmp_create_context();

    xmp_start_smix( ctx, 1, 1 );
    int poggersLoadError = xmp_smix_load_sample_mem( ctx, 0, s_poggersBytes, s_poggersBytesCount );
    if ( poggersLoadError < 0 )
    {
        printf( "Error: cannot load poggers in memory\n" );

        switch ( poggersLoadError )
        {
        case -XMP_ERROR_FORMAT :
            printf( "Unsupported Format Error\n" );
            break;
        case -XMP_ERROR_LOAD :
            printf( "Error laoding actual memory\n" );
            break;
        case -XMP_ERROR_INVALID :
            printf( "Error params\n" );
            break;
        case -XMP_ERROR_STATE :
            printf( "Some System Error\n" );
            break;
        default :
            printf( "Fuck if I know\n" );
            break;
        }
        return;
    }

    if ( xmp_load_module_from_memory( ctx, s_musicBytes, s_musicBytesCount ) < 0 )
    {
        printf( "Error: cannot load module in memory\n" );
        return;
    }

    APUSetBufferSize( BUFFER_WORD_COUNT ); // word count = sample count/2 (i.e.
                                           // number of stereo sample pairs)
    APUSetSampleRate( ASR_22_050_Hz );
}

static int initialVol = 60;
static int playing = 1;

void PlayPoggers()
{

    if ( xmp_load_module_from_memory( ctx, s_silenceBytes, s_silenceBytesCount ) < 0 )
    {
        printf( "Error: cannot load module in memory\n" );
        return;
    }

    uint32_t prevframe = APUFrame();
    if ( xmp_start_player( ctx, 22050, 0 ) == 0 )
    {
        xmp_get_module_info( ctx, &mi );
        printf( "%s (%s)\n", mi.mod->name, mi.mod->type );

        xmp_set_player( ctx, XMP_PLAYER_VOLUME, initialVol );

        bool playedPoggers = false;

        while ( playing ) // size == 2*BUFFER_WORD_COUNT, in bytes
        {

            playing = xmp_play_buffer( ctx, apubuffer, BUFFER_SIZE_IN_BYTES, 0 ) == 0;

            // Make sure the writes are visible by the DMA
            CFLUSH_D_L1;

            // Fill current write buffer with new mix data
            APUStartDMA( (uint32_t) apubuffer );

            // Wait for the APU to be done with current read buffer which is still
            // playing
            uint32_t currframe;
            do
            {
                // APU will return a different 'frame' as soon as the current buffer
                // reaches the end
                currframe = APUFrame();
            } while ( currframe == prevframe );

            // Once we reach this point, the APU has switched to the other buffer we
            // just filled, and playback resumes uninterrupted

            // Remember this frame
            prevframe = currframe;

            if ( !playedPoggers )
            {
                int playPoggersError = xmp_smix_play_sample( ctx, 0, 60, 100, 0 );

                if ( playPoggersError < 0 )
                {
                    printf( "Error: cannot play poggers in memory\n" );

                    switch ( playPoggersError )
                    {
                    case -XMP_ERROR_STATE :
                        printf( "Play State invalid for playing sample\n" );
                        break;
                    case -XMP_ERROR_INVALID :
                        printf( "bad params\n" );
                        break;
                    default :
                        printf( "Idk\n" );
                        break;
                    }
                    return;
                }
                playedPoggers = true;
            }
        }
    }
}

void FadeModOut()
{
    if ( initialVol > 0 )
    {
        initialVol -= 2;
        xmp_set_player( ctx, XMP_PLAYER_VOLUME, initialVol );
    }

    if ( initialVol == 0 )
    {
        xmp_stop_module( ctx );
        initialVol = 100;
        xmp_set_player( ctx, XMP_PLAYER_VOLUME, initialVol );

        xmp_release_module( ctx );
    }
}

void PlayMod()
{

    volatile int *sharedmem = (volatile int *) TaskGetSharedMemory();
    volatile int *s_frame1 = sharedmem + 4;

    *sharedmem = 0;
    *s_frame1 = 0;

    uint32_t prevframe = APUFrame();

    if ( xmp_start_player( ctx, 22050, 0 ) == 0 )
    {
        xmp_get_module_info( ctx, &mi );
        printf( "%s (%s)\n", mi.mod->name, mi.mod->type );

        xmp_set_player( ctx, XMP_PLAYER_VOLUME, initialVol );

        while ( playing ) // size == 2*BUFFER_WORD_COUNT, in bytes
        {

            playing = xmp_play_buffer( ctx, apubuffer, BUFFER_SIZE_IN_BYTES, 0 ) == 0;

            // Make sure the writes are visible by the DMA
            CFLUSH_D_L1;

            // Fill current write buffer with new mix data
            APUStartDMA( (uint32_t) apubuffer );

            // Wait for the APU to be done with current read buffer which is still
            // playing
            uint32_t currframe;
            do
            {
                // APU will return a different 'frame' as soon as the current buffer
                // reaches the end
                currframe = APUFrame();
            } while ( currframe == prevframe );

            // Once we reach this point, the APU has switched to the other buffer we
            // just filled, and playback resumes uninterrupted

            // Remember this frame
            prevframe = currframe;

            if ( *sharedmem == 1 )
            {
                FadeModOut();
                break;
            }
        }
    }

    printf( "Exitted main mod loop\n)" );
    PlayPoggers();

    xmp_end_player( ctx );

    xmp_release_module( ctx );
    xmp_free_context( ctx );
}

short *GetAPUBuffer() { return apubuffer; }
#include <cmath>
#include <stdint.h>
#include <stdio.h>

#include <unistd.h>

#include "basesystem.h"

#include "task.h"

#include "commondemo.h"

#include "onlyModPlayer.h"
#include "spinningStuff.h"

#define HEAP_SZ ( 2 * 1024 * 1024 )

void DoOtherThings()
{

    while ( 1 )
    {

        spinThings();
        break;
    }

    printf( "Task done\n" );
}

int main()
{

    void *data = (void *) malloc( HEAP_SZ );

    heapInit( &g_commonHeap, data, HEAP_SZ );

    ModInit();
    set320x240();

    // // Always do this from main thread
    struct STaskContext *taskctx1 = TaskGetContext( 1 );
    uint32_t *stackAddress = new uint32_t[20 * 1024];
    int taskID1 = TaskAdd( taskctx1, "DoOtherThings", DoOtherThings, TS_RUNNING,
                           HALF_SECOND_IN_TICKS, (uint32_t) stackAddress );
    if ( taskID1 == 0 )
    {
        printf( "Error: No room to add new task on CPU 1\n" );
    }

    PlayMod();
}

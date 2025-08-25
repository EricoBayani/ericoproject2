
#define BUFFER_WORD_COUNT 1024 // buffer size (max: 2048 bytes i.e. 1024 words)
#define BUFFER_SIZE_IN_BYTES ( BUFFER_WORD_COUNT * 2 * sizeof( short ) )

static short *apubuffer;

extern void ModInit( void );

extern void PlayMod( void );

extern short *GetAPUBuffer();
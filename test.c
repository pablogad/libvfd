#include <stdio.h>
#include <stdlib.h>

#include <pt6312.h>
#include <timing.h>

/* TEST */
int main( int argc, char *argv[] )
{
   int rc=0;
   grid_blink gb;

   u_int8_t data[ 12 ];

   /* Initialize display */
   rc = init_pt6312( data );

   //TEST: mensaje hack
   memset( data, 0, 12 );
   setStringLeft( data, (u_int8_t*)"PASA" );
   setStringRight( data, (u_int8_t*)" 2059" );
   setDotsRight( data, BOTH_DOTS );
   updateDisplay( data );

   if( rc == 0 )
   {
      memset( &gb, 0, sizeof( grid_blink ) );

      setUpDataArea( data );

      gb.segment_mask = 0x0000007F;
      gb.gridno  = 0;
      gb.periods = 2;    /* 2 * FRAME_TIME = every second */
   }

   if( rc == 0 )
   {
      rc = setUpBlink( &gb );

      gb.segment_mask = 0x0000FF7F;
      gb.gridno  = 1;
      gb.periods = 3;    /* 3 * FRAME_TIME = every 1.5 second */

      rc = setUpBlink( &gb );
   }

   if( rc ) printf( "AARGH\n" );

   while(1);

   close_pt6312();
}

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>

#include <pt6312.h>
#include <timing.h>

#define MAX_ANIMS  32      /* Max no of animation programs */
#define MAX_BLINKS 32      /* Max no of blink programs */

static volatile grid_anim anims_array[ MAX_ANIMS ];
static volatile u_int8_t num_anims;

static volatile grid_blink blink_array[ MAX_BLINKS ];
static volatile u_int8_t num_blinks;

static u_int8_t first_time = 1;

/* Display 'framebuffer': 6 grids * 16 segments */
u_int16_t* data = NULL;

/* SIGALRM handler */
void timerHandler( int signum )
{
   static u_int8_t lock = 0;

   /* Not reentrant */
   if( lock ) return;

   lock=1;

   /* Increase period counters of all elements */
   /* First perform blinks */
   int cnt=0;
   for( ; cnt != num_blinks; cnt++ )
   {
      blink_array[ cnt ].cnt_periods++;
      if( blink_array[ cnt ].cnt_periods >= blink_array[ cnt ].periods )
      {
          /* Update segments of the program */
          data[ blink_array[ cnt ].gridno ] ^= blink_array[ cnt ].segment_mask;
          blink_array[ cnt ].cnt_periods = 0;
      }
   }

   /* Second perform animations */
   for( cnt=0; cnt != num_anims; cnt++ )
   {
      anims_array[ cnt ].cnt_periods++;
      if( anims_array[ cnt ].cnt_periods >= anims_array[ cnt ].periods )
      {
         /* Copy the frame */
         grid_frame* this_frame = &anims_array[ cnt ].frames[ anims_array[ cnt ].frame ];
         memcpy( &data[ this_frame->ini_grid ], \
                 this_frame->segment_masks, \
                 this_frame->num_grids * sizeof( u_int32_t ) );

         anims_array[ cnt ].cnt_periods = 0;
         anims_array[ cnt ].frame++;
         if( anims_array[ cnt ].frame == anims_array[ cnt ].num_frames )
         {
            anims_array[ cnt ].frame = 0;
         }
      }
   }

   /* Update display */
   updateDisplay( (u_int8_t*)data );

   lock=0;
}


/* Create a timer for display refresh */
int setUpTimer( long ms )
{
   struct sigaction sa; 
   struct itimerval timer; 
   int rc = 0;

printf( "Init timer\n" );
   memset( blink_array, 0, sizeof( blink_array ) );
   num_blinks = 0;
   memset( anims_array, 0, sizeof( anims_array ) );
   num_anims = 0;

   memset( &sa, 0, sizeof (sa) ); 
   sa.sa_handler = &timerHandler; 
   rc = sigaction( SIGALRM, &sa, NULL );

   if( rc == 0 )
   {
      /* Configure the interval (ms) */
      timer.it_value.tv_sec = 0; 
      timer.it_value.tv_usec = ms * 1000L;
      timer.it_interval.tv_sec = 0; 
      timer.it_interval.tv_usec = ms * 1000L;

      /* Start timer */
      rc = setitimer( ITIMER_REAL, &timer, NULL );
   }

   return rc;
}

/* Set up the pointer to data area */
int setUpDataArea( u_int8_t* display_data_ref )
{
   data = (u_int16_t*)display_data_ref;
}

/* Create a timer for display refresh */
int setUpBlink( grid_blink* gb )
{
   int rc = 0;

   if( first_time )
   {
      if( data == NULL) return -1;

      rc = setUpTimer( FRAME_TIME );  /* Refresh rate */
      first_time = 0;
   }

   /* Copy the new grid blink program */
   if( rc == 0 )
   {
      gb->cnt_periods = 0;

      memcpy( &blink_array[ num_blinks ], gb, sizeof( grid_blink ) );

printf( "Copied blink %d\n", num_blinks );
      num_blinks++;
   }

   return rc;
}

/* Create a timer for display refresh */
int setUpAnimation( grid_anim* ga )
{
   int rc = 0;

   if( first_time )
   {
      rc = setUpTimer( FRAME_TIME );  /* Refresh rate */
      first_time = 0;
   }

   /* Copy the new grid animation program */
   if( rc == 0 )
   {
      ga->cnt_periods = 0;

      memcpy( &anims_array[ num_anims ], ga, sizeof( grid_anim ) );

printf( "Created animation %d\n", num_anims );
      num_anims++;
   }

   return rc;
}


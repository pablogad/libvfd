#ifndef __TIMING_H__
#define __TIMING_H__

#define FRAME_TIME 50   /* miliseconds between frames */
#define MAX_GRIDS  6    /* grid count of the display  */

/* Descriptor for blinking a set of segments in a grid */
typedef struct _blinky
{

   u_int32_t segment_mask;
   u_int8_t  gridno;
   u_int8_t  periods;      /* Number of periods to wait before
                            * changing state of the bits in the
                            * mask for this grid
                            */
   /* Private fields */
   u_int8_t  cnt_periods;  /* Keeps count of periods */

} grid_blink;

/* Animation of a set of grids */
typedef struct _frame
{

   u_int32_t segment_masks[ MAX_GRIDS ];  /* Contains num_grids els */
   u_int8_t  ini_grid;
   u_int8_t  num_grids;

} grid_frame;

typedef struct _animation
{

   grid_frame* frames;
   u_int8_t  num_frames;   /* Frames of animation */

   u_int8_t  periods;      /* Number of periods to wait before
                            * next frame
                            */
   /* Private fields */
   u_int8_t  frame;        /* Current frame */
   u_int8_t  cnt_periods;  /* Keeps count of periods */

} grid_anim;

#define MAX_ANIMS  32      /* Max no of animation programs */
#define MAX_BLINKS 32      /* Max no of blink programs */

#ifdef __cplusplus
extern "C" {
#endif

int setUpTimer( long ms );
int setUpDataArea( u_int8_t* display_data_ref );
int setUpBlink( grid_blink* gb );
int setUpAnimation( grid_anim* ga );

#ifdef __cplusplus
}
#endif

#endif

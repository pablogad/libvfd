#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <bcm2835.h>

#include "pt6312.h"

#define bit u_int8_t
#define false 0
#define true 1

/* CMD1 definitions */
#define DISPLAY_4GRID_16SEGMENT_MODE 0x00
#define DISPLAY_5GRID_16SEGMENT_MODE 0x01
#define DISPLAY_6GRID_16SEGMENT_MODE 0x02
#define DISPLAY_7GRID_15SEGMENT_MODE 0x03
#define DISPLAY_8GRID_14SEGMENT_MODE 0x04
#define DISPLAY_9GRID_13SEGMENT_MODE 0x05
#define DISPLAY_10GRID_12SEGMENT_MODE 0x06
#define DISPLAY_11GRID_11SEGMENT_MODE 0x07

/* CMD2 definitions */
#define CMD2_CMD 0x40
#define WR_TO_DISPLAY_CMD 0x00
#define WR_TO_LED_PORT_CMD 0x01
#define RD_KEY_DATA_CMD 0x02
#define RD_SW_DATA_CMD 0x03
#define FIXED_ADDRESS_CMD 0x04  

/* CMD3 definitions */
#define CMD3_CMD 0xC0

/* CMD4 definitions */
#define CMD4_CMD 0x80
#define DISPLAY_ON 0x08
#define DISPLAY_OFF 0x00
#define MIN_BRIGHTNESS 0x00
#define V2_16_BRIGHTNESS 0x01
#define V4_16_BRIGHTNESS 0x02
#define V10_16_BRIGHTNESS 0x03
#define V11_16_BRIGHTNESS 0x04
#define V12_16_BRIGHTNESS 0x05
#define V13_16_BRIGHTNESS 0x06
#define MAX_BRIGHTNESS 0x07

/* Pin state definitions */
#define DOWN false
#define UP true

/* GPIO pins */
#define RO_PIN  RPI_GPIO_P1_15
#define CLK_PIN RPI_GPIO_P1_19
#define STB_PIN RPI_GPIO_P1_21
#define DIO_PIN RPI_GPIO_P1_23

typedef struct _st {
   bit clk;
   bit stb;
   bit din;
   bit dout;
} state_t;

typedef struct _xlat_char {
   u_int8_t ascii;
   u_int8_t display;
} display_xlat;

static display_xlat display_xlat_table[] = \
  { { 'A', CHAR_A }, { 'B', CHAR_B }, { 'C', CHAR_C }, { 'D', CHAR_D }, \
    { 'E', CHAR_E }, { 'F', CHAR_F }, { 'G', CHAR_G }, { 'H', CHAR_H }, \
    { 'h', CHAR_h }, { 'I', CHAR_I }, { 'J', CHAR_J }, { 'L', CHAR_L }, \
    { 'M', CHAR_M }, { 'N', CHAR_N }, { 'O', CHAR_O }, { 'P', CHAR_P }, \
    { 'Q', CHAR_Q }, { 'R', CHAR_R }, { 'S', CHAR_S }, { 'T', CHAR_T }, \
    { 'U', CHAR_U }, { 'Y', CHAR_Y }, { 'Z', CHAR_Z }, { 'c', CHAR_c }, \
    { 't', CHAR_t }, { 'c', CHAR_c }, { 'u', CHAR_u }, \
    { '0', ZERO }, { '1', ONE }, { '2', TWO }, { '3', THREE }, { '4', FOUR }, \
    { '5', FIVE }, { '6', SIX }, { '7', SEVEN }, { '8', EIGHT }, {'9', NINE }, \
    { ' ', 0 }, { '_', CHAR_UNDERSCORE }, { '-', CHAR_MINUS }, { '=', CHAR_EQUAL }, \
    { 0xFF, 0xFF } };

static state_t status;
static xcur=4, ycur=0;

/* Set CLK to UP or DOWN state */
static void CLK( bit state )
{
   /* Only change CLK if STB is low */
   if( status.stb == 0 )
   {
      if( state == UP )
         bcm2835_gpio_write( CLK_PIN, HIGH );
      else
         bcm2835_gpio_write( CLK_PIN, LOW );

      usleep( 10 );  // 10us

      status.clk = state;
   }
   else
   {
      status.clk = 1;  /* When disabled UP */
   }
}

/* Wait the time before pulling STB down again */
static void TWAIT()
{
   CLK( UP );       /* Locked */
   usleep( 2000 );  /* >= 1 usec */
}

/* Send a single bit */
static void SEND_BIT( bit val )
{
   CLK( UP );

   if( val & 1 == 1 )
      bcm2835_gpio_write( DIO_PIN, HIGH );
   else
      bcm2835_gpio_write( DIO_PIN, LOW );

   status.dout = val&1;

   CLK( DOWN );    /* load */

}

/* Send data byte - does not change STB! must be DOWN */
static void DATA( u_int8_t val )
{
   int n;
   for( n=0; n<=7; n++ )
   {
      SEND_BIT( (val >> n) & 1 );
   }
   TWAIT();
}

/* Read a data byte */
static void READ( u_int8_t* in_data )
{
   int n;

   CLK( DOWN );
   TWAIT();

   *in_data = 0;

   for( n=0; n<=7; n++ )
   {
      CLK( UP );    // read
      usleep( 1 );  // Takes 300ns max

      *in_data = *in_data << 1;

      // Read bit
      if( bcm2835_gpio_lev( DIO_PIN ) == HIGH )
      {
         *in_data |= 1;
         status.din = 1;
      }
      else
      {
         status.din = 0;
      }

      CLK( DOWN );
      usleep( 1 );  // Takes 400ns min
   }
}

/* Set STB to UP or DOWN state */
static void STB( bit state )
{
   if( state == UP )
      bcm2835_gpio_write( STB_PIN, HIGH );
   else
      bcm2835_gpio_write( STB_PIN, LOW );

   usleep( 10 );   // 10 us

   status.stb = state;

   TWAIT();
}

/* Send a command 1 - sets STB to DOWN and UP after the command */
static void CMD1( u_int8_t val )
{
   STB( DOWN );
   DATA( val & 0x07 );
   STB( UP );
}

/* Send a command 2 - pulls STB to DOWN and UP after the command */
static void CMD2( u_int8_t val )
{
   STB( DOWN );
   DATA( CMD2_CMD + ( val & 0x0F ) );
   STB( UP );
   TWAIT();     /* Wait before pulling down STB again */
}

/* Send a command 3 - pulls STB to DOWN . After this command at least
 *                    one byte must be sent
 */
static void CMD3( u_int8_t val )
{
   STB( DOWN );
   DATA( CMD3_CMD + ( val & 0x1F ) );
   TWAIT();     /* Wait before data */
}

/* Send a command 4 - pulls STB to DOWN and UP after the command */
static void CMD4( u_int8_t val )
{
   STB( DOWN );
   DATA( CMD4_CMD + ( val & 0x0F ) );
   TWAIT();     /* Wait before pulling STB up */
   STB( UP );
}

/****************** EXPORTED FUNCTIONS *****************/
/* Send our data to display */
void updateDisplay( u_int8_t* data )
{
   CMD2( WR_TO_DISPLAY_CMD );
   CMD3( 0 );  /* addr=0 and autoincrement addr */
   int cnt=0;
   for( ; cnt<12; cnt++ ) DATA( data[cnt] );  /* 6 grids = 12 bytes */
   STB( UP );
   CMD1( DISPLAY_6GRID_16SEGMENT_MODE );
   CMD4( DISPLAY_ON+MAX_BRIGHTNESS );
}

/* Program LEDs */
void updateLEDs( u_int8_t which )
{
   CMD2( WR_TO_LED_PORT_CMD+FIXED_ADDRESS_CMD );
   DATA( which );
   STB( UP );
}

/* Read all the keyboard matrix. in_data must be at least 3 bytes long. */
void readKeys( u_int8_t* in_data )
{
   CMD2( RD_KEY_DATA_CMD + FIXED_ADDRESS_CMD );

   // Set pin in read mode
   bcm2835_gpio_fsel( DIO_PIN, BCM2835_GPIO_FSEL_INPT );
   // with pullup
   bcm2835_gpio_set_pud( DIO_PIN, BCM2835_GPIO_PUD_UP );

   int n=0;
   for( ; n<3; n++ )
   {
      STB( DOWN );
      READ( &in_data[ n ] );
      TWAIT();   // >= 1us
      STB( UP );
   }

   // Set in write mode again
   bcm2835_gpio_fsel( DIO_PIN, BCM2835_GPIO_FSEL_OUTP );
}


/* Translate a character to display segment format */
void xlat( u_int8_t* c )
{
   // Look for character
   int idx=0;

   while( display_xlat_table[ idx ].display != 0xFF && \
          display_xlat_table[ idx ].ascii != *c )
   {
      idx++;
   }

   // Not found, return
   if( display_xlat_table[ idx ].display == 0xFF )
      return;

   // Translate
   *c = display_xlat_table[ idx ].display;
}

/* Helper functions to set the display */

/* Sets a string for the 8888 part (4 characters) */
void setStringLeft( u_int8_t* data, u_int8_t* str )
{
   u_int8_t priv_str[ 4 ];

   memcpy( priv_str, str, 4 );

   // Translate characters:
   int i=0;
   for( ; i<4; i++ )
      xlat( &priv_str[i] );

   data[ 0 ] = ( priv_str[ 1 ] & 0x7F ) + ( priv_str[ 0 ] & 1 ) * 128;
   data[ 1 ] = priv_str[ 0 ] / 2;
   data[ 2 ] = ( priv_str[ 3 ] & 0x7F ) + ( priv_str[ 2 ] & 1 ) * 128;
   data[ 3 ] = priv_str[ 2 ] / 2;
}

/* Sets a string for the 8:88:88 part (5 characters) */
void setStringRight( u_int8_t* data, u_int8_t* str )
{
   u_int8_t priv_str[ 5 ];

   memcpy( priv_str, str, 5 );

   // Translate characters:
   int i=0;
   for( ; i<5; i++ )
      xlat( &priv_str[i] );

   // First char: for grid 3:
   data[ 2*2 ] = *priv_str;
   // Second char: for grid 4:
   data[ 3*2 ] = priv_str[1] & 0x7F;  // Respect :
   // Third char: for grid 5:
   data[ 4*2 ] = (data[ 4*2 ] & 0x80) | priv_str[2];  // Respect :
   // Fourth and fifth char: for grid 6:
   data[ 5*2 ] = ( priv_str[4] & 0x7F ) + ( priv_str[3] & 1 ) * 128;
   data[ 5*2 + 1 ] = ( priv_str[3] & 0x7F ) / 2;
}

/* Sets the : characters in the right part
 * which: bit0: first ':'
 *        bit1: second ':'
 */
void setDotsRight( u_int8_t* data, u_int8_t which )
{
   if( ( which & 1 ) != 0 )
      data[ 2*2 ] |= TWO_POINTS;
   if( ( which & 2 ) != 0 )
      data[ 4*2 ] |= TWO_POINTS;
}


/* Initialize display. Return 0 if OK */
int init_pt6312( u_int8_t* data )
{
   int rc = 0;

   if (!bcm2835_init())
   {
      printf( "ERROR initializing BCM library!\n" );
      return 1;
   }

   /* Set up pins */
   bcm2835_gpio_fsel( RO_PIN,  BCM2835_GPIO_FSEL_INPT );
   bcm2835_gpio_fsel( CLK_PIN, BCM2835_GPIO_FSEL_OUTP );
   bcm2835_gpio_fsel( STB_PIN, BCM2835_GPIO_FSEL_OUTP );
   bcm2835_gpio_fsel( DIO_PIN, BCM2835_GPIO_FSEL_OUTP );

   STB( UP );
   CLK( UP );

   /* Inicialization */

   /* Clear display */
   memset( data, 0, 12 );
   updateDisplay( data );

   return 0;
}

/* Finish display access */
void close_pt6312( )
{
   /* Inhibit display */
   CMD4( DISPLAY_OFF );
   bcm2835_close();
}


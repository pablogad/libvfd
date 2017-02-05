#ifndef __PT6312_H__
#define __PT6312_H__

#include <sys/types.h>

/* Function declarations */
#ifdef __cplusplus
extern "C" {
#endif

#include <bcm2835.h>

void updateDisplay( u_int8_t* data );
void updateLEDs( u_int8_t which );
void readKeys( u_int8_t* in_data );
void setStringLeft( u_int8_t* data, u_int8_t* str );
void setStringRight( u_int8_t* data, u_int8_t* str );
void setDotsRight( u_int8_t* data, u_int8_t which );
int init_pt6312( u_int8_t* data );
void close_pt6312( );

#ifdef __cplusplus
}
#endif

// These values multiplied x128 for left character in G6 and right in G1,G2 (bit 7)
#define CHAR_A 0x3F
#define CHAR_B 0x7F
#define CHAR_b 0x7C
#define CHAR_C 0x65
#define CHAR_c 0x68
#define CHAR_D 0x7A
#define CHAR_E 0x6D
#define CHAR_F 0x2D
#define CHAR_G 0x75
#define CHAR_H 0x3E
#define CHAR_h 0x3C
#define CHAR_I 0x24
#define CHAR_J 0x52
#define CHAR_L 0x64
#define CHAR_M 0x37
#define CHAR_N 0x37
#define CHAR_O 0x77
#define CHAR_P 0x2F
#define CHAR_Q 0x77
#define CHAR_R 0x25
#define CHAR_S 0x5D
#define CHAR_T 0x6C
#define CHAR_t 0x6C
#define CHAR_U 0x76
#define CHAR_u 0x70
#define CHAR_Y 0x4E
#define CHAR_Z 0x6B
#define CHAR_UNDERSCORE 0x40
#define CHAR_MINUS      0x08
#define CHAR_EQUAL      0x48

#define ZERO   0x77 
#define ONE    0x12
#define TWO    0x6B
#define THREE  0x5B
#define FOUR   0x1E
#define FIVE   0x5D
#define SIX    0x7D
#define SEVEN  0x13
#define EIGHT  0x7F
#define NINE   0x5F

#define TWO_POINTS 0x80  // for single char grids only (G3,G4,G5)

#define LEFT_DOTS  0x01
#define RIGHT_DOTS 0x02
#define BOTH_DOTS  0x03

#define S_TITLE   0x8000    // G1
#define S_CHAPTER 0x8000    // G2
#define S_ANGLE   0x8000    // G6

#define V_BAR     0x4000    // G1 and G2
#define S_MIN     0x4000    // G3
#define S_1L      0x4000    // G5
#define S_PROG    0x4000    // G6

#define S_VCD     0x2000    // G3
#define S_CIRC1   0x2000    // G4
#define S_SLASH   0x2000    // G5

#define S_DVDVID  0x1000    // G3
#define S_CIRC2   0x1000    // G4
#define S_2R      0x1000    // G5

#define S_LOOP    0x0800    // G3
#define S_CIRC3   0x0800    // G4
#define S_PLAY    0x0800    // G5

#define S_A       0x0400    // G3
#define S_CIRC4   0x0400    // G4
#define S_PAUSE   0x0400    // G5

#define S_B       0x0200    // G3
#define S_CIRC5   0x0200    // G4
#define S_DDD     0x0200    // G5

#define S_ALL     0x0100    // G3
#define S_CIRC6   0x0100    // G4
#define S_RAND    0x0100    // G5

/* LED */
#define I_LED0_ON    0x00
#define I_LED1_ON    0x00
#define I_LED2_ON    0x00
#define I_LED3_ON    0x00
#define I_LED0_OFF   0x01
#define I_LED1_OFF   0x02
#define I_LED2_OFF   0x04
#define I_LED3_OFF   0x08

#endif


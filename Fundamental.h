#ifndef Fundamental_h
#define Fundamental_h

#include <nmmintrin.h>
#include <string.h>
#include <stdlib.h>

#define MASK         0
#define MASK_DIM     2
#define A_SIZE       8

#define BLOCK_LENGTH 128                //128, 192, 256
#define WORD_SIZE    4
#define NB           4
#define NK			 4					//4, 6, 8	
#define NR			 10                 //10, 12, 14

#define IRP          0x1B               //irreducible polynomial, 0x011b

#define BITS		 8
#define ZERO         0x00

#define MAT_POW(cond) MAT_POW_ ## cond
#define MAT_POW_2    {0x40,0x68,0x20,0xB4,0x50,0x72,0x80,0xD1}
#define MAT_POW_4    {0x68,0x18,0x20,0xA6,0xDC,0x7C,0x40,0x4D}
#define MAT_POW_16   {0xE4,0x7A,0x20,0x74,0x76,0x3E,0x18,0xF5}
/*
^2
0	1	0	0	0	0	0	0
0	1	1	0	1	0	0	0
0	0	1	0	0	0	0	0
1	0	1	1	0	1	0	0
0	1	0	1	0	0	0	0
0	1	1	1	0	0	1	0
1	0	0	0	0	0	0	0
1	1	0	1	0	0	0	1
*/
/*
^ 4
0	1	1	0	1	0	0	0
0	0	0	1	1	0	0	0
0	0	1	0	0	0	0	0
1	0	1	0	0	1	1	0
1	1	0	1	1	1	0	0
0	1	1	1	1	1	0	0
0	1	0	0	0	0	0	0
0	1	0	0	1	1	0	1
*/
/*
^ 8
0	0	0	1	1	0	0	0
1	1	1	0	0	1	0	0
0	0	1	0	0	0	0	0
1	0	0	1	0	0	1	0
1	0	1	1	1	1	1	0
1	1	0	1	1	1	1	0
0	1	1	0	1	0	0	0
1	0	0	1	1	0	1	1
*/
/*
^ 16
1	1	1	0	0	1	0	0
0	1	1	1	1	0	1	0
0	0	1	0	0	0	0	0
0	1	1	1	0	1	0	0
0	1	1	1	0	1	1	0
0	0	1	1	1	1	1	0
0	0	0	1	1	0	0	0
1	1	1	1	0	1	0	1
*/


#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define ROTATE_RIGHT(x, s, n) ((x) >> (n)) | ((x) << ((s) - (n)))
/* only modular once
 */
#define MINUS_MOD(x, s, n)  ((x-s) >= 0 ? (x-s) : (x-s+n))
#define PLUS_MOD(x, s, n)   ((x+s) >= n ? (s+x-n) : (x+s))



typedef unsigned char BYTE;

BYTE powGF(BYTE base, int exp);

BYTE invGF(BYTE x);

BYTE *modularProduct(const BYTE *wordx, const BYTE *wordy);

BYTE multiplyGF(BYTE bytex, BYTE bytey);


#endif // !Fundamental_h
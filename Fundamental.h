//
//  Fundamental.h
//  AES
//
//  Created by benny on 16/4/22.
//

#ifndef Fundamental_h
#define Fundamental_h

#include <nmmintrin.h>
#include <string.h>
#include <stdlib.h>

/*================
 * ATTENTION:
 * - `SIZE_A` MAY NOT be `0` when `MASK` is `1`.
 * - `SIZE_A` MUST be `0` as well when `MASK` is `0`.
 * - `SIZE_A` should be either `0` or `8`,  other values to be implemented...
 *=================*/
#define MASK         2
#define SIZE_A       8

#define BLOCK_LENGTH 128                //128, 192, 256
#define WORD_SIZE    4
#define NB           4
#define NK           4                  //4, 6, 8	
#define NR           10                 //10, 12, 14



/* ---------------- MAY NOT CHANGE THE FOLLOWING SETTINGS -----------------
 */
#define IRP          0x1B               //irreducible polynomial, 0x011b

#define BITS         8
#define ZERO         0x00
#define UNIT_BYTE    0x80
#define UNIT_MAT     {  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01  }

#define MAT_POW(cond) MAT_POW_ ## cond
#define MAT_POW_2       0xC0, 0x28, 0x60, 0x94, 0xF0, 0x22, 0xD0, 0x51
#define MAT_POW_4       0xE8, 0x90, 0x48, 0x76, 0x1C, 0xB0, 0x7C, 0xED
#define MAT_POW_16      0x2C, 0x9E, 0x8C, 0x62, 0xDA, 0xD6, 0x02, 0x71

#define MAT_CT(cond) MAT_CT_ ## cond
#define MAT_CT_8	 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
#define MAT_CT_7	 0x1b, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02
#define MAT_CT_6	 0x36, 0x1b, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04
#define MAT_CT_5	 0x6c, 0x36, 0x1b, 0x80, 0x40, 0x20, 0x10, 0x08
#define MAT_CT_4	 0xd8, 0x6c, 0x36, 0x1b, 0x80, 0x40, 0x20, 0x10
#define MAT_CT_3	 0xab, 0xd8, 0x6c, 0x36, 0x1b, 0x80, 0x40, 0x20
#define MAT_CT_2	 0x4d, 0xab, 0xd8, 0x6c, 0x36, 0x1b, 0x80, 0x40
#define MAT_CT_1	 0x9a, 0x4d, 0xab, 0xd8, 0x6c, 0x36, 0x1b, 0x80

#define MAT_C(cond) MAT_C_ ## cond
#define MAT_C_8		0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
#define MAT_C_7		0x40, 0x20, 0x10, 0x88, 0x84, 0x02, 0x81, 0x80
#define MAT_C_6		0x20, 0x10, 0x88, 0xc4, 0x42, 0x81, 0xc0, 0x40
#define MAT_C_5		0x10, 0x88, 0xc4, 0x62, 0xa1, 0xc0, 0x60, 0x20
#define MAT_C_4		0x88, 0xc4, 0x62, 0xb1, 0xd0, 0x60, 0x30, 0x10
#define MAT_C_3		0xc4, 0x62, 0xb1, 0x58, 0xe8, 0x30, 0x98, 0x88
#define MAT_C_2		0x62, 0xb1, 0x58, 0x2c, 0xf4, 0x98, 0x4c, 0xc4
#define MAT_C_1		0xb1, 0x58, 0x2c, 0x96, 0xfa, 0x4c, 0xa6, 0x62

#define POP_CONT  {	0, 128, 128, 0, 128, 0, 0, 128, 128, 0, 0, 128, 0, 128, 128, 0,\
					128, 0, 0, 128, 0, 128, 128, 0, 0, 128, 128, 0, 128, 0, 0, 128,\
					128, 0, 0, 128, 0, 128, 128, 0, 0, 128, 128, 0, 128, 0, 0, 128,\
					0, 128, 128, 0, 128, 0, 0, 128, 128, 0, 0, 128, 0, 128, 128, 0,\
					128, 0, 0, 128, 0, 128, 128, 0, 0, 128, 128, 0, 128, 0, 0, 128,\
					0, 128, 128, 0, 128, 0, 0, 128, 128, 0, 0, 128, 0, 128, 128, 0,\
					0, 128, 128, 0, 128, 0, 0, 128, 128, 0, 0, 128, 0, 128, 128, 0,\
					128, 0, 0, 128, 0, 128, 128, 0, 0, 128, 128, 0, 128, 0, 0, 128,\
					128, 0, 0, 128, 0, 128, 128, 0, 0, 128, 128, 0, 128, 0, 0, 128,\
					0, 128, 128, 0, 128, 0, 0, 128, 128, 0, 0, 128, 0, 128, 128, 0,\
					0, 128, 128, 0, 128, 0, 0, 128, 128, 0, 0, 128, 0, 128, 128, 0,\
					128, 0, 0, 128, 0, 128, 128, 0, 0, 128, 128, 0, 128, 0, 0, 128,\
					0, 128, 128, 0, 128, 0, 0, 128, 128, 0, 0, 128, 0, 128, 128, 0,\
					128, 0, 0, 128, 0, 128, 128, 0, 0, 128, 128, 0, 128, 0, 0, 128,\
					128, 0, 0, 128, 0, 128, 128, 0, 0, 128, 128, 0, 128, 0, 0, 128,\
					0, 128, 128, 0, 128, 0, 0, 128, 128, 0, 0, 128, 0, 128, 128, 0,}

#define MOD		  {	0, 1, 2, 3, 4, 5, 6, 7,\
					0, 1, 2, 3, 4, 5, 6, 7,\
					0, 1, 2, 3, 4, 5, 6, 7,\
					0, 1, 2, 3, 4, 5, 6, 7,\
					0, 1, 2, 3, 4, 5, 6, 7,\
					0, 1, 2, 3, 4, 5, 6, 7,\
					0, 1, 2, 3, 4, 5, 6, 7,\
					0, 1, 2, 3, 4, 5, 6, 7,\
					0, 1, 2, 3, 4, 5, 6, 7 }

#define DIV		  {	0, 0, 0, 0, 0, 0, 0, 0,\
					1, 1, 1, 1, 1, 1, 1, 1,\
					2, 2, 2, 2, 2, 2, 2, 2,\
					3, 3, 3, 3, 3, 3, 3, 3,\
					4, 4, 4, 4, 4, 4, 4, 4,\
					5, 5, 5, 5, 5, 5, 5, 5,\
					6, 6, 6, 6, 6, 6, 6, 6,\
					7, 7, 7, 7, 7, 7, 7, 7,\
					8, 8, 8, 8, 8, 8, 8, 8 }

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define ROTATE_RIGHT(x, s, n) ((x) >> (n)) | ((x) << ((s) - (n)))
/* only modular once
 */
#define MINUS_MOD(x, s, n)  (((x)-(s)) >= 0 ? ((x)-(s)) : ((x)-(s)+(n)))
#define PLUS_MOD(x, s, n)   (((x)+(s)) >= (n) ? ((s)+(x)-(n)) : ((x)+(s)))

#define CHECK(x) if( (x) ) return (x)


typedef unsigned char BYTE;

typedef enum RESULT
{
    RES_OK = 0,
    RES_INVALID_DIMENSION,
    RES_INVALID_POINTER,
    RES_ERROR_IN_OPERATION,
    RES_INVALID_SETTINGS

}Res;

BYTE invGF(BYTE x);

Res invGFMasked(BYTE *inversed, const BYTE *x);

BYTE powGF(BYTE base, int exp);

BYTE multiplyGF(BYTE bytex, BYTE bytey);

Res multiplyMat(BYTE *mlres, const BYTE *matx, const BYTE *maty, const int *dims);

Res modularProduct(BYTE *mpRes, const BYTE *wordx, const BYTE *wordy, int index);

Res setup4Fundamental();

Res decode(BYTE *decoded, const BYTE *inputs);

Res encode(BYTE *encoded, const BYTE *inputs);

#endif // !Fundamental_h

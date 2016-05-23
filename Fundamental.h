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

#define MASK         2
#define SIZE_A       8
// if MASK == 1, SIZE_A cannot be 0

#define BLOCK_LENGTH 128                //128, 192, 256
#define WORD_SIZE    4
#define NB           4
#define NK			 4					//4, 6, 8	
#define NR			 10                 //10, 12, 14



/* ---------------- MAY NOT CHANGE THE FOLLOWING SETTINGS -----------------
 */
#define IRP          0x1B               //irreducible polynomial, 0x011b

#define BITS		 8
#define ZERO         0x00
#define UNIT_BYTE    0x80
#define UNIT_MAT	 {  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01  }

#define MAT_POW(cond) MAT_POW_ ## cond
#define MAT_POW_2    {	0x40, 0x68, 0x20, 0xB4, 0x50, 0x72, 0x80, 0xD1	}
#define MAT_POW_4    {	0x68, 0x18, 0x20, 0xA6, 0xDC, 0x7C, 0x40, 0x4D	}
#define MAT_POW_16   {	0xE4, 0x7A, 0x20, 0x74, 0x76, 0x3E, 0x18, 0xF5	}

#define MAT_CT(cond) MAT_CT_ ## cond
#define MAT_CT_1	 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
#define MAT_CT_2	 0x1d, 0x80, 0x40, 0x20, 0x10, 0x02, 0x04, 0x02
#define MAT_CT_3	 0x3a, 0x1d, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04
#define MAT_CT_4	 0x74, 0x3a, 0x1d, 0x80, 0x40, 0x20, 0x10, 0x08
#define MAT_CT_5	 0xe8, 0x74, 0x3a, 0x1d, 0x80, 0x40, 0x20, 0x10
#define MAT_CT_6	 0xcd, 0xe8, 0x74, 0x3a, 0x1d, 0x80, 0x40, 0x20
#define MAT_CT_7	 0x87, 0xcd, 0xe8, 0x74, 0x3a, 0x1d, 0x80, 0x40
#define MAT_CT_8	 0x13, 0x87, 0xcd, 0xe8, 0x74, 0x3a, 0x1d, 0x80

#define MAT_C(cond) MAT_C_ ## cond
#define MAT_C_1		0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
#define MAT_C_2		0x40, 0x20, 0x10, 0x88, 0x84, 0x82, 0x01, 0x80
#define MAT_C_3		0x20, 0x10, 0x88, 0xc4, 0xc2, 0x41, 0x80, 0x40
#define MAT_C_4		0x10, 0x88, 0xc4, 0xe2, 0x61, 0xa0, 0x40, 0x20
#define MAT_C_5		0x88, 0xc4, 0xe2, 0x71, 0xb0, 0x50, 0x20, 0x10
#define MAT_C_6		0xc4, 0xe2, 0x71, 0x38, 0xd8, 0xa8, 0x10, 0x88
#define MAT_C_7		0xe2, 0x71, 0x38, 0x1c, 0x6c, 0xd4, 0x88, 0xc4
#define MAT_C_8		0x71, 0x38, 0x1c, 0x8e, 0x36, 0x6a, 0xc4, 0xe2

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
	RES_ERROR_IN_OPERATION

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

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
 * USAGE:
 * - unmasked version :                     `MASK=0` and `SIZE_A=0`;
 * - masked(i-th order) version with A :    `MASK=i` and `SIZE_A=8`;
 * - masked(i-th order) version without A : `MASK=i` and `SIZE_A=0`;
 *--------------------------------------------------------------------------------
 *--------------------------------------------------------------------------------
 * ATTENTION:
 * -  `SIZE_A' should never be `0` when `MASK = 1';
 * -  `SIZE_A' is supposed to be 0 when `MASK = 0';
 * -  available values for `SIZE_A': 0, 8;
 *=================*/
#define MASK         0
#define SIZE_A       0

/*------------------- SETTINGS OF AES  -------------------------------------------
 */
#define BLOCK_LENGTH 128                //128 bits
#define WORD_SIZE    4					//4 bytes per word
#define NB           4					//4 words plaintext/key/ciphertext
#define NK           36                 //36 subkey space
#define NR           32                 //32 rounds



/* ---------------- MAY NOT CHANGE THE FOLLOWING SETTINGS -----------------------
 */
#define IRP          0xF5               //irreducible polynomial, 0x01F5 (for SM4)

#define BITS         8
#define RANGE        256
#define ZERO         0x00
#define UNIT_BYTE    0x80
#define UNIT_MAT     {  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01  }

    /* MAT_EE (IRP=0xf5)
	'23468C183060C080'
    '3265CA942850A040'
    '3A74E9D2A4489020'
    '3E7CF8F1E2C48810'
    'BC78F0E0C1820408'
    '5EBC78F0E0C18204'
    '8C183060C0800102'
    '468C183060C08001'
    */

#define MAT_EE {0x23, 0x46, 0x8C, 0x18, 0x30, 0x60, 0xC0, 0x80, \
		0x32, 0x65, 0xCA, 0x94, 0x28, 0x50, 0xA0, 0x40, \
		0x3A, 0x74, 0xE9, 0xD2, 0xA4, 0x48, 0x90, 0x20, \
		0x3E, 0x7C, 0xF8, 0xF1, 0xE2, 0xC4, 0x88, 0x10, \
		0xBC, 0x78, 0xF0, 0xE0, 0xC1, 0x82, 0x04, 0x08, \
		0x5E, 0xBC, 0x78, 0xF0, 0xE0, 0xC1, 0x82, 0x04, \
		0x8C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x01, 0x02, \
		0x46, 0x8C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x01, }

#define MAT_POW(cond) MAT_POW_ ## cond
#define MAT_POW_2       0x50, 0x58, 0x70, 0x74, 0xE0, 0x32, 0xA0, 0x11
#define MAT_POW_4       0x2C, 0xCC, 0x5C, 0x6E, 0x78, 0xA4, 0x20, 0x65
#define MAT_POW_16      0x80, 0x3C, 0x7E, 0x6C, 0x86, 0xD4, 0x5C, 0x51


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
#define ROTATE_LEFT(x, s, n) ((x) << (n)) | ((x) >> ((s) - (n)))
/* only modular once
 */
#define MINUS_MOD(x, s, n)  (((x)-(s)) >= 0 ? ((x)-(s)) : ((x)-(s)+(n)))
#define PLUS_MOD(x, s, n)   (((x)+(s)) >= (n) ? ((s)+(x)-(n)) : ((x)+(s)))

#define CHECK(x) if( (x) ) return (x)


typedef unsigned char BYTE;
typedef unsigned long int WORD;

typedef enum RESULT
{
    RES_OK = 0,
    RES_INVALID_DIMENSION,
    RES_INVALID_POINTER,
    RES_ERROR_IN_KEY_EXPANSION,
    RES_INVALID_SETTINGS

}Res;

BYTE invGF(BYTE x);

Res invGFMasked(BYTE *inversed, const BYTE *x);

BYTE powGF(BYTE base, int exp);

BYTE multiplyGF(BYTE bytex, BYTE bytey);

Res multiplyMat(BYTE *mlres, const BYTE *matx, const BYTE *maty, const int *dims);

Res setup4Fundamental();

Res decode(BYTE *decoded, const BYTE *inputs);

Res encode(BYTE *encoded, const BYTE *inputs);

#endif // !Fundamental_h

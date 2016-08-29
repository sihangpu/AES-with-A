//
//  AES.h
//  AES
//
//  Created by benny on 16/4/22.
//

#ifndef AES_h
#define AES_h


/* ---------------- MAY NOT CHANGE THE FOLLOWING SETTINGS -----------------
*/

#include "Fundamental.h"

#define AFFINE_MATRIX  {	0xF8, 0x7C, 0x3E, 0x1F, 0x8F, 0xC7, 0xE3, 0xF1	}
#define AFFINE_C       0x63

#define MIX_COL_A(cond) MIX_COL_A_ ## cond
#define MIX_COL_A_3	   0x03
#define MIX_COL_A_2    0x01
#define MIX_COL_A_1    0x01
#define MIX_COL_A_0    0x02

#define RCONST(cond) RCONST_ ## cond
#define RCONST_1       0x01
#define RCONST_2       0x02
#define RCONST_3       0x04
#define RCONST_4       0x08
#define RCONST_5       0x10

#define RCONST_6       0x20
#define RCONST_7       0x40
#define RCONST_8       0x80
#define RCONST_9       0x1B
#define RCONST_10      0x36


Res setup4AES();
Res keyExpansion(BYTE *expanded, const BYTE *key);
Res encrypt(BYTE *encRes, const BYTE *plain, const BYTE *key);
Res encrypt_fixed();
#endif

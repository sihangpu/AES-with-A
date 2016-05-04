//
//  AES.h
//  AES
//
//  Created by benny on 16/4/22.
//

#ifndef AES_h
#define AES_h

#include "Fundamental.h"

#define AFFINE_MATRIX  0xF8
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


BYTE *keyExpansion(const BYTE *key);
BYTE *encrypt(const BYTE *plain, const BYTE *key);
#endif
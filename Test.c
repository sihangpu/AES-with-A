//
//  Test.c
//  AES
//
//  Created by benny on 16/4/22.
//

#include "AES.h"
#include <time.h>
#include <stdio.h>

#define TIMES			100
/*
 * following arrays are used for testing
 */
#if SIZE_A
extern
BYTE  matA[SIZE_A], matInvA[SIZE_A], matTransA[SIZE_A];
extern
BYTE  matICA[SIZE_A][SIZE_A], matIC[SIZE_A][SIZE_A];

#endif

static
const BYTE plain[NB*WORD_SIZE] = { 0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, \
0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34 };
static
const BYTE key[NK*WORD_SIZE] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, \
0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
static
BYTE tem[NB*WORD_SIZE] = { 0 };
static
BYTE cipher[NB*WORD_SIZE] = { 0 };
static
BYTE keyExpanded[NB*(NR + 1)*WORD_SIZE] = { 0 };

#if MASK
    static
    BYTE encoded[MASK][NB*WORD_SIZE] = { 0 };
    static
    BYTE decoded[NB*WORD_SIZE] = { 0 };
#endif

extern const BYTE lookupTable[256];
extern const BYTE mod8[64];
extern Res transpose(BYTE *transRes, const BYTE *matOrig, const int *dims);
extern Res multiplyGFMasked(BYTE *mlRes, const BYTE *byteXs, const BYTE *byteYs);
extern BYTE  powGF(BYTE base, int expIndex);

void outputMat(const BYTE *mat, int byts, const char str[]){
    printf("\n --> %s :\n", str);
    for (int i = 0; i < byts; ++i){
        printf("%02x ", *(mat + i));
    }
    printf("\n");
}

void lookatPlainAndKey(){
    outputMat(plain, 16, "PlainText");
    outputMat(key, 16, "Key");

    keyExpansion(keyExpanded, key);
    outputMat(keyExpanded, NB*(NR + 1)*WORD_SIZE, "Key Schedule");
}


#if MASK
void encodeDecodeTest(){
    /* encode
    */
    encode((BYTE *)encoded, plain);
    outputMat((const BYTE*)encoded, MASK*NB*WORD_SIZE, "Encoded Plaintext");

    /* decode
    */
    decode(decoded, (const BYTE*)encoded);
    outputMat(decoded, NB*WORD_SIZE, "Decoded Plaintext");
}
#endif

#if SIZE_A
void lookatMatA(){
    outputMat(matA, SIZE_A, "Mat A");
    outputMat(matInvA, SIZE_A, "Mat Inversed A");
    outputMat(matTransA, SIZE_A, "Mat Transposed A");
    const int dimsM[4] = { SIZE_A, SIZE_A, SIZE_A, SIZE_A };
    multiplyMat(tem, matInvA, matTransA, dimsM);
    outputMat(tem, SIZE_A, "MatA * MatInvA");
    /* ICA, IC
     */
    outputMat((const BYTE*)matICA, SIZE_A * SIZE_A, "Matrix ICA");
    outputMat((const BYTE*)matIC, SIZE_A*SIZE_A, "Matrix IC");
}




void multiplyGFTest(){
    const int bytex = 0;
    const int bytey = 0;

    /* original multiplyGF
    */
    BYTE mgf = multiplyGF(plain[bytex], plain[bytey]);
    outputMat(&mgf, 1, "Original Multiply in GF");

    BYTE mgf2 = powGF(plain[bytex], 0);
    outputMat(&mgf2, 1, "Original Multiply in GF(square)");
    /* original multiplyGF using matrices C
     */
    const int dimsT[4] = { 1, BITS, SIZE_A, SIZE_A };
    BYTE matC[SIZE_A][SIZE_A] = { MAT_C(1), MAT_C(2), MAT_C(3), MAT_C(4), MAT_C(5), MAT_C(6), MAT_C(7), MAT_C(8) };
    BYTE CX[SIZE_A] = { 0 };
    // get CX
    int row, col;
    for (row = 0; row < BITS; ++row){
        for (col = 0; col < BITS; ++col){
            BYTE tem = lookupTable[matC[col][row] & plain[bytex]] >> mod8[col];
            if (tem)
                CX[row] ^= tem;
        }
    }
    multiplyMat(tem, &plain[bytey], (const BYTE*)CX, dimsT);
    outputMat(tem, 1, "Original But Using Matrix C");

    /* masking multiplyGF
     */
    BYTE inp[MASK] = { 0 };
    BYTE inp2[MASK] = { 0 };
    int j;
    for (j = 0; j < MASK; ++j){
        inp[j] = encoded[j][bytex];
        inp2[j] = encoded[j][bytey];
    }
#if !SIZE_A
    inp[1] = (BYTE)rand();
    inp[0] = inp[1] ^ plain[bytex];
    inp2[1] = (BYTE)rand();
    inp2[0] = inp2[1] ^ plain[bytey];

    tem[0] = multiplyGF(inp[0], inp2[0]);
    tem[1] = multiplyGF(inp[0], inp2[1]);
    tem[2] = multiplyGF(inp[1], inp2[0]);
    tem[3] = multiplyGF(inp[1], inp2[1]);

    tem[0] ^= tem[1] ^ tem[2] ^ tem[3];
    outputMat(tem, 1);
#endif
    BYTE mgfM[MASK] = { 0 };
    multiplyGFMasked(mgfM, inp, inp2);
    outputMat(mgfM, MASK, "Masking Multiply in GF");
    BYTE tem2 = 0x00;

#if !SIZE_A
    tem2 = mgfM[0];
#else

    multiplyMat(&tem2, mgfM, matA, dimsT);
#endif
    int i;
    for (i = 1; i < MASK; ++i){
        tem2 ^= mgfM[i];
    }
    outputMat(&tem2, 1, "Decoded Masking MultiplyGF Result");

}
#endif

int main(){
    srand((BYTE)time(NULL));
    lookatPlainAndKey();

#if SIZE_A
    setup4AES();
    lookatMatA();
    multiplyGFTest();
#endif

#if MASK
    encodeDecodeTest();
#endif




    /* ======== Encrytion test ==============
     */
    double timeStart = (double)clock();
    /*  ------- begin -----------------
     */
    for (int i = 0; i < TIMES; ++i){
        encrypt(cipher, plain, key);
        //encrypt_fixed();
    }
    /* -------- end -------------------
     */
    double timeEnd = (double)clock();

    outputMat(cipher, NB * WORD_SIZE, "Encryption Result, CipherText");
    printf("\n ---> Time Cost: \n %.fms\n	", (timeEnd - timeStart));

    return 0;
}



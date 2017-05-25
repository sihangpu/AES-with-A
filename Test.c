//
//  Test.c
//  AES
//
//  Created by benny on 16/4/22.
//

#include "SM4.h"
#include <time.h>
#include <stdio.h>

#define TIMES			100
/*
 * following arrays are used for testing
 */
#if SIZE_A
extern
BYTE  matA[SIZE_A], matInvA[SIZE_A], matTransA[SIZE_A];

#endif

const BYTE plain[NB*WORD_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, \
		0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 };
const BYTE key[NB*WORD_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, \
		0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 };
static
BYTE tem[NB*WORD_SIZE] = { 0 };
static
BYTE cipher[NB*WORD_SIZE] = { 0 };

#if MASK
    static
    BYTE encoded[MASK][NB*WORD_SIZE] = { 0 };
    static
    BYTE decoded[NB*WORD_SIZE] = { 0 };
#endif

extern const BYTE lookupTable[256];
extern const BYTE mod8[64];
extern Res transpose(BYTE *transRes, const BYTE *matOrig, const int *dims);
extern Res tensorProductOfMat(BYTE *tpres, const BYTE *matX, const BYTE *matY);
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
}




void multiplyGFTest(){
    const int bytex = 0;
    const int bytey = 5;


    BYTE tensorRes[64*8] = { 0 };
    tensorProductOfMat(tensorRes, matA, matA);
    outputMat(tensorRes, 64*8, "TensorProductRes");

    /* original multiplyGF
    */
    BYTE mgf = multiplyGF(plain[bytex], plain[bytey]);
    outputMat(plain+bytex, 1, "Original input bytex");
    outputMat(plain+bytey, 1, "Original input bytey");

    outputMat(&mgf, 1, "Original Multiply in GF");

    BYTE mgf2 = powGF(plain[bytex], 0);
    outputMat(&mgf2, 1, "Original square in GF");

    const int dimsT[4] = { 1, BITS, SIZE_A, SIZE_A };

    /* masking multiplyGF
     */
    BYTE inp[MASK] = { 0 };
    BYTE inp2[MASK] = { 0 };
    BYTE mgfM[MASK] = { 0 };
    BYTE tem2 = 0xFF;
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


    multiplyGFMasked(mgfM, inp, inp2);
    outputMat(mgfM, MASK, "Masking Multiply in GF");


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

#if (SIZE_A && MASK)
    setupSM4();
    encodeDecodeTest();
    lookatMatA();
    multiplyGFTest();
#elif MASK
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



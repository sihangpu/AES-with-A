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
#if SIZE_A
extern
BYTE  matA[SIZE_A], matInvA[SIZE_A] , matTransA[SIZE_A];
extern
BYTE  matICA[SIZE_A][SIZE_A], matIC[SIZE_A][SIZE_A];

#endif
extern const BYTE lookupTable[256];
extern const BYTE mod8[64];
extern Res transpose(BYTE *transRes, const BYTE *matOrig, const int *dims);
extern Res multiplyGFMasked(BYTE *mlRes, const BYTE *byteXs, const BYTE *byteYs);

void outputMat(const BYTE *mat, int byts){
	int i;
	for (i = 0; i < byts; ++i){
		printf("%02x ", *(mat+i));
	}
}

int main(){

	Res res = RES_OK;
	
	const BYTE plain[NB*WORD_SIZE] = { 0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, \
		0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34 };
	const BYTE key[NK*WORD_SIZE] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, \
		0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
	
	
	BYTE cipher[NB*WORD_SIZE] = { 0 };
	BYTE keyExpanded[NB*(NR + 1)*WORD_SIZE] = { 0 };
	BYTE tem[NB*WORD_SIZE + 3] = { 0 };
	int i;

	srand((BYTE)time(NULL));

	printf(" --> Plaintext and Key: \n");
	outputMat(plain, 16);
	printf("\n");
	outputMat(key, 16);
	printf("\n");

	printf("\n ---> Key Schedule: \n");
	res =  keyExpansion(keyExpanded, key);
	outputMat(keyExpanded, NB*(NR + 1)*WORD_SIZE);
	printf("\n");

#if SIZE_A
	/* generate matA
	 */
	res = setup4AES();
	
	printf("\n ---> Mat A: \n");
	outputMat(matA, SIZE_A);
	printf("\n ---> Mat InvA: \n");
	outputMat(matInvA, SIZE_A);
	printf("\n ---> Mat TransA : \n");
	outputMat(matTransA, SIZE_A);
	printf("\n");
	const int dimsM[4] = { SIZE_A, SIZE_A, SIZE_A, SIZE_A };
	res = multiplyMat(tem, matInvA, matTransA, dimsM);
	printf("\n ---> MatA * MatInvA: \n");
	outputMat(tem, SIZE_A);
	printf("\n");
	/* ICA, IC
	 */
	printf("\n ---> matICA: \n");
	outputMat((const BYTE*)matICA, SIZE_A * SIZE_A);
	printf("\n");
	printf("\n ---> matIC: \n");
	outputMat((const BYTE*)matIC, SIZE_A*SIZE_A);
	printf("\n");
#endif /* SIZE_A */

#if MASK
	/* encode
	*/
	BYTE encoded[MASK][NB*WORD_SIZE] = { 0 };
	res = encode((BYTE *)encoded, plain);
	printf("\n ---> encoded: \n");
	outputMat((const BYTE*)encoded, MASK*NB*WORD_SIZE);
	printf("\n");

	/* decode
	*/
	BYTE decoded[NB*WORD_SIZE] = { 0 };
	res = decode(decoded, (const BYTE*)encoded);
	printf("\n ---> decoded: \n");
	outputMat(decoded, NB*WORD_SIZE);
	printf("\n");
	
	/* multiplyGF
	*/
	const int bytex = 9;
	const int bytey = 4;
	BYTE mgf = multiplyGF(plain[bytex], plain[bytey]);
	printf("\n ---> original multiplyGF: \n");
	outputMat(&mgf,1);
	printf("\n");
	
	
	const int dimsT[4] = { 1, BITS, SIZE_A, SIZE_A };
	BYTE matCT[SIZE_A][SIZE_A] = { MAT_CT(1), MAT_CT(2), MAT_CT(3), MAT_CT(4), MAT_CT(5), MAT_CT(6), MAT_CT(7), MAT_CT(8) };
	BYTE CX[SIZE_A] = {0};
	// get CX
	int row, col;
	for (row = 0; row < BITS; ++row){
		for (col = 0; col < BITS; ++col){
			BYTE tem = lookupTable[matCT[col][row] & plain[bytex]] >> mod8[col];
			if (tem)
				CX[row] ^= tem;
		}
	}
	
	res = multiplyMat(tem, &plain[bytey], (const BYTE*)CX, dimsT);
	printf("\n ---> Using matC: \n");
	outputMat(tem, 1);
	printf("\n");
	/*---------------------------------------*/
	BYTE inp[MASK] = { 0 };
	BYTE inp2[MASK] = { 0 };
	for (i = 0; i < MASK; ++i){
		inp[i]  = encoded[i][bytex];
		inp2[i] = encoded[i][bytey];
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
	res = multiplyGFMasked(mgfM, inp, inp2);
	CHECK(res);
	printf("\n ---> masking multiplyGF: \n");
	outputMat(mgfM, MASK);
	printf("\n");
	BYTE tem2 = 0x00;
	
#if !SIZE_A
	tem2 = mgfM[0];
#else
	
	res = multiplyMat(&tem2, mgfM, matA, dimsT);
#endif
	tem2 ^= mgfM[1] ^ mgfM[2];
	printf("\n ---> decoded masking multiplyGF: \n");
	outputMat(&tem2, 1);
	printf("\n");


#endif /* MASK */


#if SIZE_A
	res = setup4AES(); CHECK(res);
#endif
	
	double timeStart = (double)clock();
	/* Encrytion -- begin -----------------
	 */
	for (i = 0; i < TIMES; ++i){
		res = encrypt(cipher, plain, key);
		//res = encrypt_fixed();
	}
	/* Encrytion -- end -------------------
	 */
	double timeEnd = (double)clock();

	printf("\n ---> Encryption Result: \n");
	for (int i = 0; i != NB * WORD_SIZE; ++i){
		printf("%02x ", cipher[i]);
	}
	printf("\n");

	printf("\n ---> Time Cost: \n %.fms\n	", (timeEnd - timeStart));
	
	return 0;
}

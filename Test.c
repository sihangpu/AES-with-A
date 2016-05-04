//
//  Test.c
//  AES
//
//  Created by benny on 16/4/22.
//

#include "AES.h"
#include <time.h>
#include <stdio.h>

#define TIMES			1
#if MASK && A_SIZE
extern
BYTE  *matA, *matInvA, *matTransA, **matICA;
#endif

int main(){

	FILE *fin, *fout;
	fin = fopen("in.txt", "r");
	fout = fopen("out.txt", "a");

	srand((BYTE)time(NULL));

	BYTE plainT[NB*WORD_SIZE] = { 0 };
	BYTE cipherK[NK*WORD_SIZE] = { 0 };
	BYTE *cipherT;

	if (fin == NULL || fout == NULL){
		printf("File Doesnt Exist\n");
		return 1;
	}
	else{
		printf("File Read Seccessfully\n");
	}

	int i;
	/* Read the plaintext
	 */
	for (i = 0; i < NB*WORD_SIZE; ++i){
		fscanf(fin, "%x", plainT + i);
		printf("%02x ", plainT[i]);
	}
	printf("\n");
	/* Read the key
	 */
	for (i = 0; i < NK*WORD_SIZE; ++i){
		fscanf(fin, "%x", cipherK + i);
		printf("%02x ", cipherK[i]);
	}
	printf("\n");


	printf("\n ---> Key Schedule: \n");
	BYTE *keysExpanded = keyExpansion(cipherK);
	for (i = 0; i != NB*(NR + 1)*WORD_SIZE; ++i){
		printf("%02x ", keysExpanded[i]);
	}
	printf("\n");

	double timeStart = (double)clock();
	/* Encrytion -- begin --------------------
	 */
	for (i = 0; i != TIMES; ++i){
		cipherT = encrypt(plainT, cipherK);
	}
	/* Encrytion -- end --------------------
	 */
	double timeEnd = (double)clock();

	printf("\n ---> Encryption Result: \n");
	for (i = 0; i < NB*WORD_SIZE; ++i){
		fscanf(fin, "%x", cipherT + i);
		printf("%02x ", cipherT[i]);
	}
	printf("\n");
	printf("\n ---> Time Cost: \n %.fms\n	", (timeEnd - timeStart));
	
	free(cipherT);
	return 0;
}
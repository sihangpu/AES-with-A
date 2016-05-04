//
//  AES.c
//  AES
//
//  Created by benny on 16/4/22.
//
#include "AES.h"


/* Affine Transformation over GF(2)
*/
static
BYTE affineTransform(BYTE b){
	int bit;
	BYTE ret = ZERO;
	BYTE row = AFFINE_MATRIX;
	for (bit = 0; bit < BITS; ++bit){
		BYTE toAdd = (BYTE)_mm_popcnt_u32(row & b) & 0x01;
		toAdd <<= (BITS - 1 - bit);
		ret ^= toAdd;
		row = ROTATE_RIGHT(row, BITS, 1);
	}
	ret ^= AFFINE_C;
	return ret;
}



static
void subBytes(BYTE *toSub){
	int stateIndex;
	int bytes = WORD_SIZE * NB;
	for (stateIndex = 0; stateIndex < bytes; ++stateIndex){
		BYTE inversed = invGF(toSub[stateIndex]);
		toSub[stateIndex] = affineTransform(inversed);
	}
}

static
void shiftRows(BYTE *toShift){
	int colIndex;
	int rowIndex;
	int bytes = WORD_SIZE * NB;
	BYTE orig[WORD_SIZE * NB];
	memcpy(orig, toShift, bytes * sizeof(BYTE));
	for (rowIndex = 0; rowIndex < WORD_SIZE; ++rowIndex){
		for (colIndex = 0; colIndex < NB; ++colIndex){
			int newIndex = rowIndex * NB + colIndex;
			int origIndex = rowIndex * NB + PLUS_MOD(colIndex, rowIndex, NB);
			toShift[newIndex] = orig[origIndex];
		}
	}
}

static
void mixColumns(BYTE *toMix){
	BYTE mixColA[WORD_SIZE] = { MIX_COL_A_0, MIX_COL_A_1, MIX_COL_A_2, MIX_COL_A_3 };
	int wordIndex;
	for (wordIndex = 0; wordIndex < NB; ++wordIndex){
		BYTE toMulti[WORD_SIZE] = { toMix[wordIndex], toMix[NB + wordIndex], toMix[NB * 2 + wordIndex], toMix[NB * 3 + wordIndex] };
		BYTE *wordTem = modularProduct(mixColA, toMulti);
		int rowIndex;
		for (rowIndex = 0; rowIndex < WORD_SIZE; ++rowIndex){
			toMix[rowIndex * NB + wordIndex] = wordTem[rowIndex];
		}
		free(wordTem);
	}
}

static
void addRoundKey(BYTE *toAdd, const BYTE* keyScheduled){
	int i, j;
	for (i = 0; i < WORD_SIZE; ++i){
		for (j = 0; j < NB; ++j){
			toAdd[i * NB + j] ^= keyScheduled[j * WORD_SIZE + i];
		}
	}
}

/* From input bytes to state words, changing the order
 */
static
BYTE *toState(const BYTE *input){
	int i, j;
	BYTE *state = (BYTE*)malloc(NB*WORD_SIZE*sizeof(BYTE));
	for (i = 0; i < WORD_SIZE; ++i){
		for (j = 0; j < NB; ++j){
			state[i * NB + j] = input[j * WORD_SIZE + i];
		}
	}
	return state;
}

/* Inversion permutation to 'toState'
 */
static
BYTE *toOutput(const BYTE *state){
	int i, j;
	BYTE *output = (BYTE*)malloc(NB*WORD_SIZE*sizeof(BYTE));
	for (i = 0; i < NB; ++i){
		for (j = 0; j < WORD_SIZE; ++j){
			output[i * WORD_SIZE + j] = state[j * NB + i];
		}
	}
	return output;
}


/* Apply 'subbyte' to a state word
 */
static
void subWord(BYTE *word){
	int i;
	for (i = 0; i < WORD_SIZE; ++i){
		BYTE inversed = invGF(word[i]);
		word[i] = affineTransform(inversed);
	}
}

static
void rotWord(BYTE *word){
	BYTE tem = word[0];
	int i;
	for (i = 0; i+1 < WORD_SIZE; ++i) word[i] = word[i + 1];
	word[WORD_SIZE - 1] = tem;
}

/* Round constant
 */
static
void rconst(BYTE *word,int i){
	BYTE rCon;
	switch (i){
	case 1: rCon = RCONST(1); break;
	case 2: rCon = RCONST(2); break;
	case 3: rCon = RCONST(3); break;
	case 4: rCon = RCONST(4); break;
	case 5: rCon = RCONST(5); break;
	case 6: rCon = RCONST(6); break;
	case 7: rCon = RCONST(7); break;
	case 8: rCon = RCONST(8); break;
	case 9: rCon = RCONST(9); break;
	case 10: rCon = RCONST(10); break;
	default: rCon = 0x00; break;
	}
	word[0] ^= rCon;
}


BYTE *keyExpansion(const BYTE *key){
	BYTE *words = (BYTE *)malloc(NB*(NR + 1)*WORD_SIZE * sizeof(BYTE));

	int keySize = WORD_SIZE * NK;
	memcpy(words, key, keySize);
	
	int i;
	BYTE *temp = (BYTE*)malloc(WORD_SIZE*sizeof(BYTE));
	for (i = NK; i < NB * (NR + 1); ++i){		
		memcpy(temp, words + (i-1)*WORD_SIZE, WORD_SIZE);

		if (i % NK == 0){
			rotWord(temp);
			subWord(temp);
			rconst(temp, i / NK);
		}
		else if (NK > 6 && i % NK == 4){
			subWord(temp);
		}
		int j;
		for (j = 0; j < WORD_SIZE; ++j){
			int index = i * WORD_SIZE + j;
			words[index] = words[index - NK * WORD_SIZE] ^ temp[j];
		}
	}
	free(temp);
	return words;
}



/* Encrypt
 */
BYTE *encrypt(const BYTE *plainText, const BYTE *cipherKey){
	BYTE *state = toState(plainText);
	BYTE *roundKey = keyExpansion(cipherKey);
	addRoundKey(state, roundKey);

	int roundIndex;
	for (roundIndex = 1; roundIndex < NR; ++roundIndex){
		subBytes(state);
		shiftRows(state);
		mixColumns(state);
		addRoundKey(state, roundKey+ roundIndex * WORD_SIZE * NB);
	}
	subBytes(state);
	shiftRows(state);
	addRoundKey(state, roundKey + NR * WORD_SIZE * NB);

	BYTE *cipherText = toOutput(state);
	
	free(state);
	free(roundKey);
	return cipherText;
}
//
//  AES.c
//  AES
//
//  Created by benny on 16/4/22.
//
#include "AES.h"

extern const
BYTE lookupTable[256];

/* Affine Transformation over GF(2)
*/
static
BYTE affineTransform(BYTE b){
	int bit;
	BYTE ret = ZERO;
	BYTE row = AFFINE_MATRIX;
	for (bit = 0; bit != BITS; ++bit){
		BYTE toAdd = lookupTable[row & b];
		toAdd >>= bit;
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
	for (stateIndex = 0; stateIndex != bytes; ++stateIndex){
		BYTE inversed = invGF(toSub[stateIndex]);
		toSub[stateIndex] = affineTransform(inversed);
	}
}

static
void shiftRows(BYTE *toShift){
	int colIndex;
	int rowIndex;
	int bytes = WORD_SIZE * NB;
	BYTE orig[WORD_SIZE * NB] = { 0 };
	memcpy(orig, toShift, bytes * sizeof(BYTE));
	for (rowIndex = 0; rowIndex != WORD_SIZE; ++rowIndex){
		for (colIndex = 0; colIndex != NB; ++colIndex){
			int newIndex = rowIndex * NB + colIndex;
			int origIndex = rowIndex * NB + PLUS_MOD(colIndex, rowIndex, NB);
			toShift[newIndex] = orig[origIndex];
		}
	}
}

static
void mixColumns(BYTE *toMix){
	int wordIndex;
	BYTE mixColA[WORD_SIZE] = { MIX_COL_A_0, MIX_COL_A_1, MIX_COL_A_2, MIX_COL_A_3 };
	BYTE wordTem[WORD_SIZE] = { 0 };
	for (wordIndex = 0; wordIndex != NB; ++wordIndex){
		BYTE toMulti[WORD_SIZE] = { toMix[wordIndex], toMix[NB + wordIndex], toMix[NB * 2 + wordIndex], toMix[NB * 3 + wordIndex] };
		memset(wordTem, 0, WORD_SIZE * sizeof(BYTE));
		if (modularProduct(wordTem, mixColA, toMulti)) return;
		int rowIndex;
		for (rowIndex = 0; rowIndex != WORD_SIZE; ++rowIndex){
			toMix[rowIndex * NB + wordIndex] = wordTem[rowIndex];
		}
	}
}

static
void addRoundKey(BYTE *toAdd, const BYTE* keyScheduled){
	int i, j;
	for (i = 0; i != WORD_SIZE; ++i){
		for (j = 0; j != NB; ++j){
			toAdd[i * NB + j] ^= keyScheduled[j * WORD_SIZE + i];
		}
	}
}

/* From input(state) bytes to state(output) words, changing the order
*/
static
Res transpose(BYTE *output, const BYTE *input){
	int i, j;
	//BYTE *state = (BYTE*)malloc(NB*WORD_SIZE*sizeof(BYTE));
	for (i = 0; i != WORD_SIZE; ++i){
		for (j = 0; j != NB; ++j){
			output[i * NB + j] = input[j * WORD_SIZE + i];
		}
	}
	return RES_OK;
}


/* Apply 'subbyte' to a state word
 */
static
void subWord(BYTE *word){
	int i;
	for (i = 0; i != WORD_SIZE; ++i){
		BYTE inversed = invGF(word[i]);
		word[i] = affineTransform(inversed);
	}
}

static
void rotWord(BYTE *word){
	int i;
	BYTE tem = word[0];
	for (i = 0; (i+1) != WORD_SIZE; ++i) word[i] = word[i + 1];
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


Res keyExpansion(BYTE *words, const BYTE *key){
	//BYTE *words = (BYTE *)malloc(NB*(NR + 1)*WORD_SIZE * sizeof(BYTE));
	int i;
	const int keySize = WORD_SIZE * NK;
	BYTE temp[WORD_SIZE] = { 0 };

	if (words == NULL) return RES_INVALID_POINTER;
	memcpy(words, key, keySize);

	for (i = NK; i != NB * (NR + 1); ++i){
		int j;
		memcpy(temp, words + (i-1)*WORD_SIZE, WORD_SIZE);

		if (i % NK == 0){
			rotWord(temp);
			subWord(temp);
			rconst(temp, i / NK);
		}
		else if (NK > 6 && i % NK == 4){
			subWord(temp);
		}
		for (j = 0; j != WORD_SIZE; ++j){
			int index = i * WORD_SIZE + j;
			words[index] = words[index - NK * WORD_SIZE] ^ temp[j];
		}
	}
	return RES_OK;
}



/* Encrypt
 */
Res encrypt(BYTE *cipherText, const BYTE *plainText, const BYTE *cipherKey){
	Res res = RES_OK;
	BYTE state[NB*WORD_SIZE] = { 0 };
	BYTE roundKey[NB*(NR + 1)*WORD_SIZE] = { 0 };

	if (res = transpose(state, plainText)) return res;
	if (res = keyExpansion(roundKey, cipherKey)) return res;
	addRoundKey(state, roundKey);

	int roundIndex;
	for (roundIndex = 1; roundIndex != NR; ++roundIndex){
		subBytes(state);
		shiftRows(state);
		mixColumns(state);
		addRoundKey(state, roundKey + roundIndex * WORD_SIZE * NB);
	}

	subBytes(state);
	shiftRows(state);
	addRoundKey(state, roundKey + NR * WORD_SIZE * NB);

	res = transpose(cipherText, state);
	return res;
}
//
//  AES.c
//  AES
//
//  Created by benny on 16/4/22.
//
#include "AES.h"

#if SIZE_A
extern
BYTE  matA[SIZE_A], matInvA[SIZE_A], matTransA[SIZE_A];

BYTE affineMatA[BITS] = { 0 };

/* Setup for AES
 */
Res setup4AES(){
	const BYTE affineMat[BITS] = AFFINE_MATRIX;
	const int dimsAffine[4] = { BITS, BITS, BITS, BITS };
	const int dimsC[4] = { 1, BITS, SIZE_A, SIZE_A };
	Res res = setup4Fundamental();
	CHECK(res);
	
	BYTE tem[BITS] = { 0 };
	res = multiplyMat(tem, matTransA, affineMat, dimsAffine);
	CHECK(res);
	res = multiplyMat(affineMatA, matInvA, tem, dimsAffine);
	return res;
}
#endif

/* Affine Transformation over GF(2)
*/
//_inline
static
BYTE affineTransform(BYTE b, int index){
	const int dimsAffine[4] = { 1, BITS, BITS, BITS };
	const BYTE affineMat[BITS] = AFFINE_MATRIX;
	BYTE ret = ZERO;
#if SIZE_A
	if (index == 0 && !multiplyMat(&ret, (const BYTE*)&b, affineMatA, dimsAffine)){
#if MASK == 1
		BYTE ac = AFFINE_C;
		BYTE tem = ZERO;
		if (!multiplyMat(&tem, &ac, matInvA, dimsAffine))
			ret ^= tem;
#endif
		return ret;
	}


#endif /* SIZE_A */	
	if (!multiplyMat(&ret, (const BYTE*)&b, affineMat, dimsAffine) && (index == MASK - 1 || index == -2) )
		ret ^= AFFINE_C;
	return ret;
}


//_inline
static
void subBytes(BYTE *toSub){
	int stateIndex;
	int bytes = WORD_SIZE * NB;
	for (stateIndex = 0; stateIndex < bytes; ++stateIndex){
#if MASK
		BYTE states[MASK] = { 0 };
		BYTE inversedBytes[MASK] = { 0 };
		int i;
		for(i = 0; i < MASK; ++i)
			states[i] = *(toSub + stateIndex + i * bytes);
		if (!invGFMasked(inversedBytes, (const BYTE*)states)){
			for (i = 0; i < MASK; ++i)
				*(toSub + stateIndex + i * bytes) = affineTransform(inversedBytes[i], i);
		}
#else
		BYTE inversed = invGF(toSub[stateIndex]);
		*(toSub + stateIndex) = affineTransform(inversed, -1);
#endif
	}
}

//_inline
static
void shiftRows(BYTE *toShift){
	int colIndex;
	int rowIndex;	
	int bytes = WORD_SIZE * NB;
	BYTE orig[WORD_SIZE * NB] = { 0 };

#if MASK
	int index;
	for (index = 0; index < MASK; ++index, toShift += bytes){		
#endif
		memcpy(orig, toShift, bytes);
		for (rowIndex = 0; rowIndex < WORD_SIZE; ++rowIndex){
			for (colIndex = 0; colIndex < NB; ++colIndex){
				int newIndex = rowIndex * NB + colIndex;
				int origIndex = rowIndex * NB + PLUS_MOD(colIndex, rowIndex, NB);
				*(toShift + newIndex) = orig[origIndex];
			}
		}
#if MASK
	}
#endif
}

//_inline
static
void mixColumns(BYTE *toMix){
	int wordIndex;
	BYTE mixColA[WORD_SIZE] = { MIX_COL_A(0), MIX_COL_A(1), MIX_COL_A(2), MIX_COL_A(3) };
	BYTE wordTem[WORD_SIZE] = { 0 };
	int index = -1;

#if MASK
	for (index = 0; index < MASK; ++index, toMix += NB * WORD_SIZE){		
#endif
		for (wordIndex = 0; wordIndex < NB; ++wordIndex){
			BYTE toMulti[WORD_SIZE] = { *(toMix + wordIndex), *(toMix + NB + wordIndex), *(toMix + NB * 2 + wordIndex), *(toMix + NB * 3 + wordIndex) };
			memset(wordTem, 0, WORD_SIZE);
			if (modularProduct(wordTem, mixColA, toMulti, index)) return;
			int rowIndex;
			for (rowIndex = 0; rowIndex < WORD_SIZE; ++rowIndex){
				*(toMix + rowIndex * NB + wordIndex) = wordTem[rowIndex];
			}
		}
#if MASK
	}
#endif
}

//_inline
static
void addRoundKey(BYTE *toAdd, const BYTE* keyScheduled){
	int i, j;
#if MASK
	toAdd += (MASK - 1) * NB* WORD_SIZE;
#endif
	for (i = 0; i < WORD_SIZE; ++i){
		for (j = 0; j < NB; ++j){
#if MASK == 1
			BYTE tem = ZERO;
			const int dims[4] = { 1, BITS, SIZE_A, SIZE_A };
			if (!multiplyMat(&tem, &keyScheduled[j * WORD_SIZE + i], matInvA, dims))
				*(toAdd + i * NB + j) ^= tem;
#else
			*(toAdd + i * NB + j) ^= keyScheduled[j * WORD_SIZE + i];
#endif
		}
	}
}

/* From input(state) bytes to state(output) words, changing the order
*/
//_inline
static
Res transpose(BYTE *output, const BYTE *input){
	int i, j;
	//NB*WORD_SIZE
	for (i = 0; i < WORD_SIZE; ++i){
		for (j = 0; j < NB; ++j){
			*(output + i * NB + j) = input[j * WORD_SIZE + i];
		}
	}
	return RES_OK;
}


/* Apply 'subbyte' to a state word
 */
//_inline
static
void subWord(BYTE *word){
	int i;
	for (i = 0; i < WORD_SIZE; ++i){
		BYTE inversed = invGF(word[i]);
		word[i] = affineTransform(inversed, -2);
	}
}

//_inline
static
void rotWord(BYTE *word){
	int i;
	BYTE tem = word[0];
	for (i = 0; (i + 1) < WORD_SIZE; ++i) word[i] = word[i + 1];
	word[WORD_SIZE - 1] = tem;
}

/* Round constant
 */
//_inline
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

//_inline
Res keyExpansion(BYTE *words, const BYTE *key){
	//NB*(NR + 1)*WORD_SIZE
	int i;
	const int keySize = WORD_SIZE * NK;
	BYTE temp[WORD_SIZE] = { 0 };

	if (words == NULL) return RES_INVALID_POINTER;
	memcpy(words, key, keySize);

	for (i = NK; i < NB * (NR + 1); ++i){
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
		for (j = 0; j < WORD_SIZE; ++j){
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

	if (transpose(state, plainText)) return RES_ERROR_IN_OPERATION;
	if (keyExpansion(roundKey, cipherKey)) return RES_ERROR_IN_OPERATION;
	
#if MASK
	BYTE states[MASK][NB*WORD_SIZE] = { 0 };
	res = encode((BYTE*)states, (const BYTE*)state); CHECK(res);
	BYTE* ptr = (BYTE *)states;
#else
	BYTE* const ptr = state;
#endif

	addRoundKey(ptr, roundKey);
			//res = decode(state, (const BYTE*)states); CHECK(res);
	int roundIndex;
	for (roundIndex = 1; roundIndex < NR; ++roundIndex){
		subBytes(ptr);
			//res = decode(state, (const BYTE*)states); CHECK(res);
		shiftRows(ptr);
			//res = decode(state, (const BYTE*)states); CHECK(res);
		mixColumns(ptr);
			//res = decode(state, (const BYTE*)states); CHECK(res);
		addRoundKey(ptr, roundKey + roundIndex * WORD_SIZE * NB);
			//res = decode(state, (const BYTE*)states); CHECK(res);
	}
	subBytes(ptr);
	shiftRows(ptr);
	addRoundKey(ptr, roundKey + NR * WORD_SIZE * NB);

#if MASK
	res = decode(state, (const BYTE*)states); CHECK(res);
#endif
	res = transpose(cipherText, state);
	return res;
}

/* Encrypt_fixed: for testing 
 */
Res encrypt_fixed(){
	const BYTE plain[NB*WORD_SIZE] = { 0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,\
										 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34 };
	const BYTE key[NK*WORD_SIZE]   = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,\
										0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
	BYTE cipher[NB*WORD_SIZE] = { 0 };
	Res res = encrypt(cipher, plain, key);
	return res;
}
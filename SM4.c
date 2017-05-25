//
//  AES.c
//  AES
//
//  Created by benny on 16/4/22.
//
#include "SM4.h"

#if SIZE_A
extern
BYTE  matA[SIZE_A], matInvA[SIZE_A], matTransA[SIZE_A];

BYTE affineMatA[BITS] = { 0 };
BYTE linearMatA[BITS*WORD_SIZE*WORD_SIZE] = { 0 };

Res setupSM4(){
	/* Setup for SM4
	 */
	int i, j, offset;
	const BYTE affineMat[BITS] = AFFINE_MATRIX;
	const BYTE linearMat[BITS*WORD_SIZE*BITS*WORD_SIZE] = SHIFT2BIT_TRANSMAT;
	const int dimsAffine[4] = { BITS, BITS, BITS, BITS };
	const int dimsLinear[4] = { BITS*WORD_SIZE, BITS*WORD_SIZE, BITS*WORD_SIZE, BITS*WORD_SIZE };

	setup4Fundamental();

	BYTE tem[BITS] = { 0 };
	multiplyMat(tem, matTransA, affineMat, dimsAffine);
	multiplyMat(affineMatA, matInvA, tem, dimsAffine);

	BYTE temL[BITS*WORD_SIZE*WORD_SIZE] = { 0 };
	BYTE temTA[BITS*WORD_SIZE*WORD_SIZE] = { 0 };
	BYTE temIA[BITS*WORD_SIZE*WORD_SIZE] = { 0 };
	for (i = 0; i < WORD_SIZE; ++i){
		offset = i*WORD_SIZE*BITS + i;
		for (j = 0; j < BITS; ++j){
			temTA[offset + j*WORD_SIZE] = matTransA[j];
			temIA[offset + j*WORD_SIZE] = matInvA[j];
		}
	}
	multiplyMat(temL, temTA, linearMat, dimsLinear);
	multiplyMat(linearMatA, temIA, temL, dimsLinear);

	return RES_OK;
}
#endif

//_inline
static
BYTE affineTransform(BYTE b, int index){
	/* Affine Transformation over GF(2)
	 */
	const int dimsAffine[4] = { 1, BITS, BITS, BITS };
	const BYTE affineMat[BITS] = AFFINE_MATRIX;
	const BYTE affineC = AFFINE_C;
	BYTE *cPtr;
	BYTE ret = ZERO;

#if SIZE_A && MASK ==1
	BYTE tem = ZERO;
	multiplyMat(&tem, &affineC, matInvA, dimsAffine);
	cPtr = &tem;
#else
	cPtr = &affineC;
#endif

#if SIZE_A
	if(index == 0)
		multiplyMat(&ret, (const BYTE*)&b, affineMatA, dimsAffine);
	else
		multiplyMat(&ret, (const BYTE*)&b, affineMat, dimsAffine);
#else
	multiplyMat(&ret, (const BYTE*)&b, affineMat, dimsAffine);
#endif

	if (index == MASK - 1 || MASK == 0)
		ret ^= (*cPtr);

	return ret;
}

//_inline
static
BYTE affineTransformKey(BYTE b){
	/* Affine Transformation over GF(2), if index = 1 use masked affine matrix.
	 */
	const BYTE affineMat[BITS] = AFFINE_MATRIX;
	const int dimsAffine[4] = { 1, BITS, BITS, BITS };
	BYTE ret = ZERO;
	multiplyMat(&ret, (const BYTE*)&b, affineMat, dimsAffine);
	ret ^= AFFINE_C;
	return ret;
}

//_inline
static
void subBytes(BYTE *toSub, int bytes){
	int byteIndex;
	for (byteIndex = 0; byteIndex < bytes; ++byteIndex){
#if MASK 
		int j;
		BYTE states[MASK] = { 0 };
		BYTE inversedBytes[MASK] = { 0 };


		for (j = 0; j < MASK; ++j)
			states[j] = affineTransform(*(toSub + byteIndex + j*bytes), j);

		invGFMasked(inversedBytes, (const BYTE*)states);

		for (j = 0; j < MASK; ++j)
			*(toSub + byteIndex + j*bytes) = affineTransform(inversedBytes[j], j);
#else

		*(toSub + byteIndex) = affineTransform(toSub[byteIndex], 0);
		BYTE inversed = invGF(toSub[byteIndex]);
		BYTE tem = multiplyGF(inversed, toSub[byteIndex]);
		*(toSub + byteIndex) = affineTransform(inversed, 0);
#endif
	}
}

//_inline
static
void subBytesKey(BYTE *toSub, int bytes){
	int byteIndex;
	for (byteIndex = 0; byteIndex < bytes; ++byteIndex){
		*(toSub + byteIndex) = affineTransformKey(toSub[byteIndex]);
		BYTE inversed = invGF(toSub[byteIndex]);
		BYTE tem = multiplyGF(inversed, toSub[byteIndex]);
		*(toSub + byteIndex) = affineTransformKey(inversed);
	}
}

//_inline
static
void linearTransform(BYTE *toTrans){
	const int dimsLinear[4] = { 1, BITS*WORD_SIZE, BITS*WORD_SIZE, BITS*WORD_SIZE };
	const int bytes = WORD_SIZE;
	const BYTE matLinear[WORD_SIZE*BITS*WORD_SIZE] = SHIFT2BIT_TRANSMAT;
	BYTE *ptr = toTrans;
	const BYTE *matTransform = matLinear;
#if MASK
	int index;
	for (index = 0; index < MASK; ++index, ptr += bytes){
#endif
#if SIZE_A
		if (index == 0)
			matTransform = linearMatA;
		else
			matTransform = matLinear;
#endif
		BYTE shiftTemp[WORD_SIZE] = { 0 };
		multiplyMat(shiftTemp, ptr, matTransform, dimsLinear);
		BYTE shiftAll[WORD_SIZE] = { shiftTemp[0] ^ shiftTemp[1] ^ shiftTemp[2], shiftTemp[1] ^ shiftTemp[2] ^ shiftTemp[3], shiftTemp[2] ^ shiftTemp[3] ^ shiftTemp[0], shiftTemp[3] ^ shiftTemp[0] ^ shiftTemp[1] };
		int byteIdx;
		memcpy(shiftTemp, ptr, WORD_SIZE);
		for (byteIdx = 0; byteIdx < WORD_SIZE; ++byteIdx){
			*(ptr + byteIdx) = shiftAll[byteIdx] ^ ptr[byteIdx] ^ shiftTemp[(byteIdx + 3) & 0x03];
		}
#if MASK
	}
#endif
}

//_inline
static
void linearTransformKey(BYTE *toTrans){
	WORD wordTemp;
	wordTemp = toTrans[3] | (toTrans[2] << 8) | (toTrans[1] << 16) | (toTrans[0] << 24);
	WORD s13 = ROTATE_LEFT(wordTemp, 32, 13);
	WORD s23 = ROTATE_LEFT(wordTemp, 32, 23);
	wordTemp ^= (s13 ^ s23);
	toTrans[3] = wordTemp & 0xFF;
	toTrans[2] = (wordTemp >> 8) & 0xFF;
	toTrans[1] = (wordTemp >> 16) & 0xFF;
	toTrans[0] = (wordTemp >> 24) & 0xFF;
}



//_inline
Res encrypt(BYTE *cipherText, const BYTE *plainText, const BYTE *masterKey){
	int i, j;
	int byteIdx, shareIdx;
	const int textSize = WORD_SIZE*NB;
	const BYTE FKConst[WORD_SIZE*NB] = FK;
	BYTE CKConst;
	BYTE rdKey[WORD_SIZE*NB] = { 0 };
	Res res = RES_OK;

	if (MASK == 1 && SIZE_A == 0) return RES_INVALID_SETTINGS;
	if (MASK == 0 && SIZE_A != 0) return RES_INVALID_SETTINGS;

#if MASK
	BYTE plains[MASK][NB*WORD_SIZE] = { 0 };
	BYTE states[MASK][NB*WORD_SIZE] = { 0 };
	BYTE temp[MASK*WORD_SIZE] = { 0 };
	BYTE tem = 0x00;
	int dimsRdkey[4] = { 1, BITS, BITS, BITS };
	encode((BYTE*)plains, (const BYTE*)plainText);
	BYTE* const ptrC = (BYTE *)states;
	BYTE* const ptrP = (BYTE *)plains;
#else
	BYTE temp[WORD_SIZE] = { 0 };
	BYTE* const ptrC = cipherText;
	const BYTE* const ptrP = plainText;
#endif

	// add FK to each key; reverse plaintext and key words
	for (i = 0; i < NB; ++i){
		for (byteIdx = 0; byteIdx < WORD_SIZE; ++byteIdx){
			*(rdKey + i*WORD_SIZE + byteIdx) = masterKey[(NB - i - 1)*WORD_SIZE + byteIdx] ^ FKConst[(NB - i - 1)*WORD_SIZE + byteIdx];
			*(ptrC + i*WORD_SIZE + byteIdx) = ptrP[byteIdx + (NB - i - 1)*WORD_SIZE];
#if MASK
			for (j = 1; j < MASK; ++j){
				shareIdx = j*WORD_SIZE*NB;
				*(ptrC + shareIdx + i*WORD_SIZE + byteIdx) = ptrP[shareIdx + byteIdx + (NB - i - 1)*WORD_SIZE];
			}
#endif
		}
	}

	// cycling each round
	for (i = 4; i < NK; ++i){
		int index0, index1, index2, index3;
		index0 = INDEX(i, 0);
		index1 = INDEX(i, 1);
		index2 = INDEX(i, 2);
		index3 = INDEX(i, 3);

		// generate roundKey
		for (byteIdx = 0; byteIdx < WORD_SIZE; ++byteIdx){
			CKConst = (((i - 4) * 4 + byteIdx) * 7) & 0xFF;
			temp[byteIdx] = rdKey[index1*WORD_SIZE + byteIdx] ^ rdKey[index2*WORD_SIZE + byteIdx] ^ rdKey[index3*WORD_SIZE + byteIdx] ^ CKConst;
		}
		subBytesKey(temp, WORD_SIZE);
		linearTransformKey(temp);
		for (byteIdx = 0; byteIdx < WORD_SIZE; ++byteIdx){
			*(rdKey + index0*WORD_SIZE + byteIdx) ^= temp[byteIdx];
		}

		// generate intermediate value

		BYTE tem2 = 0x00;

		for (byteIdx = 0; byteIdx < WORD_SIZE; ++byteIdx){
			temp[byteIdx] = ptrC[index1*WORD_SIZE + byteIdx] ^ ptrC[index2*WORD_SIZE + byteIdx] ^ ptrC[index3*WORD_SIZE + byteIdx];
#if MASK 
			for (j = 1; j < MASK; ++j){
				shareIdx = j*WORD_SIZE*NB;
				temp[j*WORD_SIZE + byteIdx] = ptrC[shareIdx + index1*WORD_SIZE + byteIdx] ^ ptrC[shareIdx + index2*WORD_SIZE + byteIdx] ^ ptrC[shareIdx + index3*WORD_SIZE + byteIdx];
			}

#if MASK == 1
			if (!multiplyMat(&tem, rdKey + index0*WORD_SIZE + byteIdx, matInvA, dimsRdkey))
				temp[byteIdx] ^= tem;
			multiplyMat(&tem2, temp, matA, dimsRdkey);
#else
			temp[(MASK-1)*WORD_SIZE + byteIdx] ^= rdKey[index0*WORD_SIZE + byteIdx];
#endif

#else
			temp[byteIdx] ^= rdKey[index0*WORD_SIZE + byteIdx];
#endif
		}


		subBytes(temp, WORD_SIZE);
		linearTransform(temp);
		for (byteIdx = 0; byteIdx < WORD_SIZE; ++byteIdx){
			*(ptrC + index0*WORD_SIZE + byteIdx) ^= temp[byteIdx];
#if MASK
			for (j = 1; j < MASK; ++j){
				shareIdx = j*WORD_SIZE*NB;
				*(ptrC + shareIdx + index0*WORD_SIZE + byteIdx) ^= temp[j*WORD_SIZE + byteIdx];
			}

#endif
		}
	}

#if MASK
	decode(cipherText, ptrC);
#endif
	return res;
}



Res encrypt_fixed(){
	/* Encrypt_fixed: used for evaluation.
	 */
	const BYTE plain[NB*WORD_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, \
		0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 };
	const BYTE key[NB*WORD_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, \
		0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 };
	BYTE cipher[NB*WORD_SIZE] = { 0 };
	Res res = encrypt(cipher, plain, key);
	return res;
}

//
//  Fundamental.c
//  AES
//
//  Created by benny on 16/4/22.
//

#include "Fundamental.h"

static
BYTE  matA[A_SIZE], matInvA[A_SIZE], matTransA[A_SIZE];
BYTE  matC[A_SIZE * A_SIZE];

const
BYTE lookupTable[256] = POP_CONT;

_inline
BYTE  powGF(BYTE base, int exp){
	int dim[4] = { 1, BITS, BITS, BITS };
	BYTE *tem = (BYTE *)malloc(dim[0] * dim[2] * sizeof(BYTE));
	memset(tem, 0, dim[0] * dim[2] * sizeof(BYTE));
	if (exp == 2){
		BYTE coeff[BITS] = MAT_POW(2);
		Res res = multiplyMat(tem, &base, coeff, dim);
	}
	else if (exp == 4){
		BYTE coeff[BITS] = MAT_POW(4);
		Res res = multiplyMat(tem, &base, coeff, dim);
	}
	else if (exp == 16){
		BYTE coeff[BITS] = MAT_POW(16);
		Res res = multiplyMat(tem, &base, coeff, dim);
	}
	else { free(tem); return 0x00; }
	BYTE rslt = *tem;
	free(tem);
	return rslt;
}


/* Multiplicative inverse of a(x) in GF(2^8)
 */
BYTE invGF(BYTE x){
	if (x == 0x00) return 0x00;
	//BYTE z = powGF(x, 2);
	BYTE z = multiplyGF(x, x);
	BYTE y = multiplyGF(z, x);

	//BYTE w = powGF(y, 4);
	BYTE w = multiplyGF(y, y);
	w = multiplyGF(w, w);

	y = multiplyGF(y, w);

	//y = powGF(y, 16);
	y = multiplyGF(y, y);
	y = multiplyGF(y, y);
	y = multiplyGF(y, y);
	y = multiplyGF(y, y);

	y = multiplyGF(y, w);
	y = multiplyGF(y, z);
	return y;
}

/* Modular product of a(x) and b(x), where a(x) and b(x) both are four-term polynomials,
 * with coefficients that are finite field elements.
 */
Res modularProduct(BYTE *mpRes, const BYTE *wordx, const BYTE *wordy){
	int i;

	if (mpRes == NULL) return RES_INVALID_POINTER;
	//BYTE *ret = (BYTE *)malloc(WORD_SIZE * sizeof(BYTE));

	for (i = 0; i < WORD_SIZE; ++i){
		int j, k;
		mpRes[i] = ZERO;
		for (j = 0, k = i; j < WORD_SIZE; ++j, k = MINUS_MOD(k, 1, WORD_SIZE)){
			if (wordx[k] == 0x01){
				mpRes[i] ^= wordy[j];
			}
			else{
				BYTE tem = multiplyGF(wordx[k], wordy[j]);
				mpRes[i] ^= tem;
			}

		}
	}
	return RES_OK;
}

/* Matrix multiply : mulitplyMat(A, B) = A * B^T
*  - *dims: points to a array containing the dimension of matrices rx,cx,ry and cy, respectively.
*  -
*/
Res multiplyMat(BYTE *mlRes, const BYTE *matx, const BYTE *maty, int *dims){
	int row, col;

	if (mlRes == NULL) return RES_INVALID_POINTER;
	if (dims[1] != dims[3]) return RES_INVALID_DIMENSION; //cx != cy?
	//BYTE *matProd = (BYTE *)malloc(dims[0] * dims[2] * sizeof(BYTE));
	//memset(matProd, 0, dims[0] * dims[2] * sizeof(BYTE));

	/* the bytes of a row
	*/
	int rowSizeX = dims[1] % BITS ? (dims[1] / BITS) + 1 : dims[1] / BITS;
	int rowSizeR = dims[2] % BITS ? (dims[2] / BITS) + 1 : dims[2] / BITS;

	for (row = 0; row != dims[0]; ++row){
		for (col = 0; col != dims[2]; ++col){
			int i;
			int index = col / BITS;
			int offset = col % BITS;
			BYTE *ptrR = mlRes + row * rowSizeR + index;
			const BYTE *ptrX = matx + row * rowSizeX;
			const BYTE *ptrY = maty + col * rowSizeX;
			for (i = 0; i != rowSizeX; ++i, ++ptrX, ++ptrY){
				/* Using
				 *	 - popcount(): the result is a num, need to cast
				 *   - lookupTable: the result bit stores at the MSB
				 */

				//BYTE tem = (BYTE)_mm_popcnt_u32((*ptrX) & (*ptrY)) & 0x01;
				//tem <<= (BITS - 1 - offset);

				BYTE tem = lookupTable[(*ptrX) & (*ptrY)];
				tem >>= offset;
				(*ptrR) ^= tem;
			}
		}
	}
	return RES_OK;
}


/* Multiplication over GF(2^8),and the irreducible polynomial is 0x011b
 */
BYTE multiplyGF(BYTE bytex, BYTE bytey){
	int bit;
	BYTE sum = ZERO;
	BYTE toAdd = bytex;
	BYTE adder = bytey;
	for (bit = 0; bit < BITS; ++bit){
		if (adder & 0x01)sum ^= toAdd;
		// do xtime(toAdd)
		if (toAdd & 0x80){
			toAdd <<= 1;
			toAdd ^= IRP;
		}
		else toAdd <<= 1;
		adder >>= 1;
	}
	return sum;
}



/*   (invA * C * A * X * A) * y
* = y^T * A^T * ICAX^T
* = y^T \mply ( ICAX \mply A^T )
*
* a row of ICAX^T : x^T \mply ICA
* (ICAX^T composing many row vectors)
*/

/*
C1 =
1     0     0     0     0     0     0     0
0     1     0     0     0     0     0     0
0     0     1     0     0     0     0     0
0     0     0     1     0     0     0     0
0     0     0     0     1     0     0     0
0     0     0     0     0     1     0     0
0     0     0     0     0     0     1     0
0     0     0     0     0     0     0     1
C2 =
0     1     0     0     0     0     0     0
0     0     1     0     0     0     0     0
0     0     0     1     0     0     0     0
1     0     0     0     1     0     0     0
1     0     0     0     0     1     0     0
1     0     0     0     0     0     1     0
0     0     0     0     0     0     0     1
1     0     0     0     0     0     0     0

C3 =
0     0     1     0     0     0     0     0
0     0     0     1     0     0     0     0
1     0     0     0     1     0     0     0
1     1     0     0     0     1     0     0
1     1     0     0     0     0     1     0
0     1     0     0     0     0     0     1
1     0     0     0     0     0     0     0
0     1     0     0     0     0     0     0

C4 =
0     0     0     1     0     0     0     0
1     0     0     0     1     0     0     0
1     1     0     0     0     1     0     0
1     1     1     0     0     0     1     0
0     1     1     0     0     0     0     1
1     0     1     0     0     0     0     0
0     1     0     0     0     0     0     0
0     0     1     0     0     0     0     0

C5 =
1     0     0     0     1     0     0     0
1     1     0     0     0     1     0     0
1     1     1     0     0     0     1     0
0     1     1     1     0     0     0     1
1     0     1     1     0     0     0     0
0     1     0     1     0     0     0     0
0     0     1     0     0     0     0     0
0     0     0     1     0     0     0     0

C6 =
1     1     0     0     0     1     0     0
1     1     1     0     0     0     1     0
0     1     1     1     0     0     0     1
0     0     1     1     1     0     0     0
1     1     0     1     1     0     0     0
1     0     1     0     1     0     0     0
0     0     0     1     0     0     0     0
1     0     0     0     1     0     0     0
C7 =
1     1     1     0     0     0     1     0
0     1     1     1     0     0     0     1
0     0     1     1     1     0     0     0
0     0     0     1     1     1     0     0
0     1     1     0     1     1     0     0
1     1     0     1     0     1     0     0
1     0     0     0     1     0     0     0
1     1     0     0     0     1     0     0
C8 =
0     1     1     1     0     0     0     1
0     0     1     1     1     0     0     0
0     0     0     1     1     1     0     0
1     0     0     0     1     1     1     0
0     0     1     1     0     1     1     0
0     1     1     0     1     0     1     0
1     1     0     0     0     1     0     0
1     1     1     0     0     0     1     0
*/

static
BYTE multiplyGFNew_AA(BYTE bytex, BYTE bytey){

}

static
BYTE multiplyGFNew_EA(BYTE bytex, BYTE bytey){

}


Res multiplyGFMasked(BYTE *mlRes, const BYTE *byteXs, const BYTE *byteYs){

	/* get matrix T
	 */
	BYTE matT[MASK_DIM * MASK_DIM] = { 0 };
	int i, j;
	for (i = 0; i < MASK_DIM; ++i){
		for (j = 0; j < MASK_DIM; ++j){
			int index = i * MASK_DIM + j;
#if A_SIZE
			if (i == 0 && j == 0){
				matT[index] = multiplyGFNew_AA(byteXs[i], byteYs[j]);
			}
			else if (i == 0){
				matT[index] = multiplyGFNew_EA(byteYs[j], byteXs[i]);
			}
			else if (j == 0){
				matT[index] = multiplyGFNew_EA(byteXs[i], byteYs[j]);
			}
			else{
				matT[index] = multiplyGF(byteXs[i], byteYs[j]);
			}
#else /* A_SIZE == 0,  i.e., masked without A
			*/
			matT[index] = multiplyGF(byteXs[i], byteYs[j]);
#endif
		}
	}
	/* get matrix R
	 */
	BYTE matR[MASK_DIM * MASK_DIM] = { 0 };
	for (i = 0; i < MASK_DIM; ++i){
		int index = i * MASK_DIM + i;
		matR[index] = matT[index];
		for (j = i + 1; j < MASK_DIM; ++j){
			index = i * MASK_DIM + j;
			matR[index] = (BYTE)rand();
			BYTE tem = matR[index] ^ matT[index];
			index = j * MASK_DIM + i;
			matR[index] = matT[index] ^ tem;
#if A_SIZE
			/* re-evaluate R(0,j)
			 */
			if (i == 0){
				int dim[4] = { 1, BITS, A_SIZE, A_SIZE };
				/* since dim[0] == 1, the production is absolutely a vector(BYTE)
				 */
				BYTE *tem = (BYTE *)malloc(dim[0] * dim[2] * sizeof(BYTE));
				memset(tem, 0, dim[0] * dim[2] * sizeof(BYTE));
				Res res = multiplyMat(tem, matR + j, matA, dim);
				matR[j] = *tem;
				free(tem);
			}
#endif
		}
	}

	/* get the final matrix
	 */
	BYTE *matRet = (BYTE *)malloc(MASK_DIM * sizeof(BYTE));
	for (i = 0; i < MASK_DIM; ++i){
		matRet[i] = 0;
		for (j = 0; j < MASK_DIM; ++j){
			int index = j * MASK_DIM + i;
			matRet[i] ^= matR[index];
		}
	}

	return RES_OK;
}

static
BYTE *encode(BYTE *inputs){

}

static
BYTE *decode(BYTE *outputs){

}

/* Generate matrix A, invA, transA
 */
static
void genA(){
	BYTE x = multiplyGF(0x53, 0x53);
}

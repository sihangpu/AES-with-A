#include "Fundamental.h"


BYTE  *matA, *matInvA, *matTransA, **matICA;



BYTE  powGF(BYTE base, int exp){
	int dim[4] = { 1, BITS, BITS, BITS };
	if (exp == 2){
		BYTE coeff[BITS] = MAT_POW(2);
		BYTE *tem = multiplyMat(base, coeff, dim);
		BYTE rslt = *tem;
		free(tem);
		return rslt;
	}
	else if (exp == 4){
		BYTE coeff[BITS] = MAT_POW(4);
		BYTE *tem = multiplyMat(base, coeff, dim);
		BYTE rslt = *tem;
		free(tem);
		return rslt;
	}
	else if (exp == 16){
		BYTE coeff[BITS] = MAT_POW(16);
		BYTE *tem = multiplyMat(base, coeff, dim);
		BYTE rslt = *tem;
		free(tem);
		return rslt;
	}

}


/* Multiplicative inverse of a(x) in GF(2^8)
 */
BYTE invGF(BYTE x){
	BYTE z = powGF(x, 2);
	BYTE y = multiplyGF(z, x);
	BYTE w = powGF(y, 4);
	y = multiplyGF(y, w);
	y = powGF(y, 16);
	y = multiplyGF(y, w);
	y = multiplyGF(y, z);
	return y;
}

/* Modular product of a(x) and b(x), where a(x) and b(x) both are four-term polynomials,
 * with coefficients that are finite field elements.
 */
BYTE *modularProduct(const BYTE *wordx, const BYTE *wordy){
	BYTE *ret = (BYTE *)malloc(WORD_SIZE * sizeof(BYTE));
	int i;
	for (i = 0; i < WORD_SIZE; ++i){
		int j, k;
		ret[i] = ZERO;
		for (j = 0, k = i; j < WORD_SIZE; ++j, k = MINUS_MOD(k, 1, WORD_SIZE)){
			if (wordx[k] == 0x01){
				ret[i] ^= wordy[j];
			}
			else{
				BYTE tem = multiplyGF(wordx[k], wordy[j]);
				ret[i] ^= tem;
			}

		}
	}
	return ret;
}

/* Multiplication over GF(2^8),and the irreducible polynomial is 0x011b
 */
BYTE multiplyGF(const BYTE bytex, const BYTE bytey){
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

/* Matrix multiply : mulitplyMat(A, B) = A * B^T
 *  - *dims: points to a array containing the dimension of matrices rx,cx,ry and cy, respectively.
 *  -
 */
BYTE *multiplyMat(const BYTE *matx, const BYTE *maty, int *dims){
	if (dims[1] != dims[3]) return 0xFF; //cx != cy?
	BYTE *matProd = (BYTE *)malloc(dims[0] * dims[2] * sizeof(BYTE));

	int row, col;
	/* the bytes of a row
	 */
	int rowSizeX = dims[1] % BITS ? dims[1] / BITS : (dims[1] / BITS) + 1;
	int rowSizeR = dims[2] % BITS ? dims[2] / BITS : (dims[2] / BITS) + 1;

	for (row = 0; row < dims[0]; ++row){
		for (col = 0; col < dims[2]; ++col){
			int index = col / BITS;
			int offset = col % BITS;
			BYTE *ptrR = matProd + row * rowSizeR + index;
			BYTE *ptrX = matx + row * rowSizeX;
			BYTE *ptrY = maty + row * rowSizeX;
			int i;
			for (i = 0; i < rowSizeX; ++i, ++ptrX, ++ptrY){
				BYTE tem = (BYTE)_mm_popcnt_u32((*ptrX) & (*ptrY)) & 0x01;
				tem <<= (BITS - 1 - offset);
				(*ptrR) ^= tem;
			}
		}
	}
	return matProd;
}


/*   (invA * C * A * X * A) * y
* = y^T * A^T * ICAX^T
* = y^T \mply ( ICAX \mply A^T )
*
* a row of ICAX^T : x^T \mply ICA
* (ICAX^T composing many row vectors)
*/

static
BYTE multiplyGFNew_AA(const BYTE bytex, const BYTE bytey){

}

static
BYTE multiplyGFNew_EA(const BYTE bytex, const BYTE bytey){

}


BYTE *multiplyGFMasked(const BYTE *byteXs, const BYTE *byteYs){

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
				BYTE *tem = multiplyMat(matR[j], matA, dim);
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

	return matRet;
}

BYTE *encode(BYTE *inputs){

}

BYTE *decode(BYTE *outputs){

}

/* Generate matrix A, invA, transA
 */
void genA(){

}

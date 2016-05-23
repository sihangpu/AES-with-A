//
//  Fundamental.c
//  AES
//
//  Created by benny on 16/4/22.
//

#include "Fundamental.h"

#if SIZE_A
BYTE  matA[SIZE_A] = { 0 }, matInvA[SIZE_A] = { 0 }, matTransA[SIZE_A] = { 0 };
BYTE  matICA[SIZE_A][SIZE_A] = { 0 }, matIC[SIZE_A][SIZE_A] = { 0 };
#endif

const
BYTE lookupTable[256] = POP_CONT;
//static 
const
BYTE mod8[] = MOD;
static const
BYTE div8[] = DIV;


/* Shift bit form j -> i */
//_inline
static
BYTE shiftBit(
BYTE orig,
int i,
int j
)
{
    BYTE tem;
    tem = (UNIT_BYTE >> j) & orig;
    if (j > i) tem <<= (j - i);
    else if (i > j) tem >>= (i - j);
    else return tem;

    return tem;
}

/* Calculate the # of bytes in one row */
//_inline
int bytesOfRow(
int col
)
{
    int bytes;
    // bytes of each row :: if dim_col < LENGTH, allocate a byte as well
    if (col < BITS) return 1;
    bytes = div8[col];
    bytes = mod8[col] ? bytes + 1 : bytes;
    return bytes;
}

/* Transposition */
//_inline
//static
Res transpose(
    BYTE *transRes,
    const BYTE *matOrig,
    const int *dims
    )
{
    int colOrig, rowOrig;
    int cntBytesOrig, cntBytesRet;
    int offsetOrig, offsetRet;
    BYTE vectTransed;
    BYTE *ptrOfVectRet;
    const BYTE *ptrOfVectOrig;

    int bytesOfRowOrig;
    int bytesOfRowRet;

    bytesOfRowOrig = bytesOfRow(dims[1]);
    bytesOfRowRet = bytesOfRow(dims[0]);
    memset(transRes, 0, dims[0] * bytesOfRowOrig);
    if (matOrig == NULL || transRes == NULL) return RES_INVALID_POINTER;
    /* Transposing */
    for (colOrig = 0; colOrig < dims[1]; ++colOrig){
        cntBytesOrig = colOrig / BITS;
        offsetOrig = colOrig % BITS;
        for (rowOrig = 0; rowOrig < dims[0]; ++rowOrig){
            /* the bit (rowOrig, colOrig) */
            cntBytesRet = rowOrig / BITS;
            offsetRet = rowOrig % BITS;

            ptrOfVectOrig = matOrig + bytesOfRowOrig * rowOrig + cntBytesOrig;
            ptrOfVectRet = transRes + bytesOfRowRet * colOrig + cntBytesRet;

            vectTransed = shiftBit(*ptrOfVectOrig, offsetRet, offsetOrig);
            (*ptrOfVectRet) ^= vectTransed;
        }
    }
    return RES_OK;

}


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
    else { free(tem); return ZERO; }
    BYTE rslt = *tem;
    free(tem);
    return rslt;
}


/* Multiplicative inverse of a(x) in GF(2^8)
 */
BYTE invGF(BYTE x){
    if (x == ZERO) return ZERO;
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
Res modularProduct(BYTE *mpRes, const BYTE *wordx, const BYTE *wordy, int index){
    int i;
    if (mpRes == NULL) return RES_INVALID_POINTER;
    for (i = 0; i < WORD_SIZE; ++i){
        int j, k;
        mpRes[i] = ZERO;
        for (j = 0, k = i; j < WORD_SIZE; ++j, k = MINUS_MOD(k, 1, WORD_SIZE)){
            if (wordx[k] == 0x01){
                mpRes[i] ^= wordy[j];
            }
            else{
                BYTE tem = ZERO;
#if SIZE_A
                if (index == 0){
                    BYTE word = ZERO;
                    const int dims[4] = { 1, BITS, SIZE_A, SIZE_A };
                    BYTE rd = (BYTE)rand();

                    if (!multiplyMat(&word, (const BYTE*)&rd, (const BYTE*)matInvA, dims))
                        word ^= wordy[j];
                    if (!multiplyMat(&tem, (const BYTE*)&word, (const BYTE*)matA, dims)){
                        word = multiplyGF(wordx[k], tem);
                        rd = multiplyGF(wordx[k], rd);
                    }
                    if (!multiplyMat(&tem, (const BYTE*)&word, (const BYTE*)matInvA, dims)
                         && !multiplyMat(&word, (const BYTE*)&rd, (const BYTE*)matInvA, dims))
                        ;
                    mpRes[i] ^= tem ^ word;
                    continue;
                }

#endif
                tem = multiplyGF(wordx[k], wordy[j]);
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
Res multiplyMat(BYTE *mlRes, const BYTE *matx, const BYTE *maty, const int *dims){
    int row, col;
    if (mlRes == NULL) return RES_INVALID_POINTER;
    if (dims[1] != dims[3]) return RES_INVALID_DIMENSION; //cx != cy?
    /* bytes of a row
    */
    int rowSizeX = bytesOfRow(dims[1]);
    int rowSizeR = bytesOfRow(dims[2]);
    memset(mlRes, 0, dims[0] * rowSizeR);
    for (row = 0; row < dims[0]; ++row){
        for (col = 0; col < dims[2]; ++col){
            int i;
            BYTE *ptrR = mlRes + row * rowSizeR + div8[col];
            const BYTE *ptrX = matx + row * rowSizeX;
            const BYTE *ptrY = maty + col * rowSizeX;
            for (i = 0; i < rowSizeX; ++i, ++ptrX, ++ptrY){
                BYTE tem = lookupTable[(*ptrX) & (*ptrY)] >> mod8[col];
                if (tem)
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

#if MASK

#if SIZE_A


/* Generate matrix A, invA, transA
*/
static
Res genA(){
    Res res = RES_OK;
    int i;
    const int dimsE[4] = { SIZE_A, SIZE_A, SIZE_A, SIZE_A };
    BYTE matP[SIZE_A] = { 0 };
    BYTE matE[SIZE_A] = UNIT_MAT;
    BYTE tem[SIZE_A] = { 0 };

    /*  get matrix A and inv(A) temporarily */
    memcpy((BYTE *)matTransA, (const BYTE *)matE, SIZE_A);
    memcpy((BYTE *)matInvA, (const BYTE *)matE, SIZE_A);

    /* get matrix P */
    for (i = 0; i < SIZE_A; ++i){

        BYTE rowToAdd;
        BYTE zeroToSet;

        rowToAdd = (BYTE)rand();
        zeroToSet = UNIT_BYTE >> i;
        rowToAdd &= (~zeroToSet);
        matE[i] ^= rowToAdd;

        /* Tanspose(P) */
        res = transpose(matP, (const BYTE *)matE, dimsE);
        CHECK(res);
        /*  A = P x A   ==>  A^T = A^T x P^T  */
        memcpy(tem, (const BYTE *)matTransA, SIZE_A);
        res = multiplyMat(matTransA, (const BYTE *)tem, (const BYTE *)matE, dimsE);
        CHECK(res);
        /*  A^{-1} = A^{-1} x P     */
        memcpy(tem, (const BYTE *)matInvA, SIZE_A);
        res = multiplyMat(matInvA, (const BYTE *)tem, (const BYTE *)matP, dimsE);
        CHECK(res);

        /* Refresh matE to Unit Matrix */
        matE[i] ^= rowToAdd;
    }

    res = transpose(matA, (const BYTE *)matTransA, dimsE);
    return res;
}

/*   (invA * C * A * X * A) * y
* = y^T * A^T * ICAX^T
* = y^T \mply ( ICAX \mply A^T )
*/

/* Generate matrices ICA and IC (after invoking 'genA()')
*/
static
Res genICA(){
    Res res = RES_OK;
    // get the transposition of matCs
    BYTE matC[SIZE_A][SIZE_A] = { MAT_C(1), MAT_C(2), MAT_C(3), MAT_C(4), MAT_C(5), MAT_C(6), MAT_C(7), MAT_C(8) };
    BYTE matCT[SIZE_A][SIZE_A] = { MAT_CT(1), MAT_CT(2), MAT_CT(3), MAT_CT(4), MAT_CT(5), MAT_CT(6), MAT_CT(7), MAT_CT(8) };
    int i;
    const int dims[4] = { SIZE_A, SIZE_A, SIZE_A, SIZE_A };
    // generate ICA and IC iteratively
    for (i = 0; i < SIZE_A; ++i){
        res = multiplyMat(matIC[i], (const BYTE*)matInvA, (const BYTE*)matCT[i], dims);
        CHECK(res);
        BYTE tem[SIZE_A] = { 0 };
        res = multiplyMat(tem, (const BYTE*)matTransA, (const BYTE*)matC[i], dims);
        CHECK(res);
        res = multiplyMat(matICA[i], (const BYTE*)matInvA, (const BYTE*)tem, dims);
        CHECK(res);
    }
    return res;
}


static
BYTE multiplyGFNew_AA(BYTE bytex, BYTE bytey){
    BYTE ICAX[BITS] = { 0 };
    BYTE tem[BITS] = { 0 };
    BYTE ret = ZERO;
    const int dims[4] = { BITS, BITS, SIZE_A, SIZE_A };
    const int dimsY[4] = { 1, BITS, BITS, BITS };
    int row, col;
    // get ICAX
    for (row = 0; row < BITS; ++row){
        for (col = 0; col < BITS; ++col){
            BYTE tem = lookupTable[matICA[col][row] & bytex] >> mod8[col];
            if (tem)
                ICAX[row] ^= tem;
        }
    }
    if (multiplyMat(tem, (const BYTE*)ICAX, (const BYTE*)matTransA, dims))
        return ZERO;
    if (multiplyMat(&ret, &bytey, (const BYTE*)tem, dimsY))
        return ZERO;
    return ret;
}

static
BYTE multiplyGFNew_EA(BYTE bytex, BYTE bytey){
    BYTE ICX[BITS] = { 0 };
    BYTE tem[BITS] = { 0 };
    BYTE ret = ZERO;
    const int dims[4] = { BITS, BITS, SIZE_A, SIZE_A };
    const int dimsY[4] = { 1, BITS, BITS, BITS };
    int row, col;
    // get ICX
    for (row = 0; row < BITS; ++row){
        for (col = 0; col < BITS; ++col){
            BYTE tem = lookupTable[matIC[col][row] & bytex] >> mod8[col];
            if (tem)
                ICX[row] ^= tem;
        }
    }
    if (multiplyMat(tem, (const BYTE*)ICX, (const BYTE*)matTransA, dims))
        return ZERO;
    if (multiplyMat(&ret, &bytey, (const BYTE*)tem, dimsY))
        return ZERO;
    return ret;

}


Res setup4Fundamental(){
    Res res = RES_OK;
    res = genA();
    CHECK(res);
    res = genICA();
    return res;
}

#endif /* SIZE_A */


/* Multiplication over GF(2^8), inputs and outputs are masking bytes
 */
Res multiplyGFMasked(BYTE *mlRes, const BYTE *byteXs, const BYTE *byteYs) {
    /* get matrix T
     */
    int i, j;
    BYTE matT[MASK][MASK] = { 0 };
    for (i = 0; i < MASK; ++i){
        for (j = 0; j < MASK; ++j){
#if SIZE_A
            if (i == 0 && j == 0){
                BYTE tem1 = ZERO, tem2 = ZERO;
                BYTE tem;
                const int dimsM[4] = {1, BITS, SIZE_A, SIZE_A};
                if (multiplyMat(&tem1, &byteXs[i], matA, dimsM)) return RES_ERROR_IN_OPERATION;
                if (multiplyMat(&tem2, &byteYs[j], matA, dimsM)) return RES_ERROR_IN_OPERATION;
                tem = multiplyGF(tem1, tem2);
                if (multiplyMat(&matT[i][j], &tem, matInvA, dimsM)) return RES_ERROR_IN_OPERATION;
                //matT[i][j] = multiplyGFNew_AA(byteXs[i], byteYs[j]);
            }
            else if (i == 0){
                BYTE tem1 = ZERO, tem2 = ZERO;
                BYTE tem;
                const int dimsM[4] = { 1, BITS, SIZE_A, SIZE_A };
                if (multiplyMat(&tem1, &byteXs[i], matA, dimsM)) return RES_ERROR_IN_OPERATION;
                tem = multiplyGF(tem1, byteYs[j]);
                if (multiplyMat(&matT[i][j], &tem, matInvA, dimsM)) return RES_ERROR_IN_OPERATION;
                //matT[i][j] = multiplyGFNew_EA(byteYs[j], byteXs[i]);
            }
            else if (j == 0){
                BYTE tem1 = ZERO, tem2 = ZERO;
                BYTE tem;
                const int dimsM[4] = { 1, BITS, SIZE_A, SIZE_A };
                if (multiplyMat(&tem2, &byteYs[j], matA, dimsM)) return RES_ERROR_IN_OPERATION;
                tem = multiplyGF(byteXs[i], tem2);
                if (multiplyMat(&matT[i][j], &tem, matInvA, dimsM)) return RES_ERROR_IN_OPERATION;
                //matT[i][j] = multiplyGFNew_EA(byteXs[i], byteYs[j]);
            }
            else{
                matT[i][j] = multiplyGF(byteXs[i], byteYs[j]);
            }
#else
            matT[i][j] = multiplyGF(byteXs[i], byteYs[j]);
#endif
        }
    }
    /* get matrix R
     */
    BYTE matR[MASK][MASK] = { 0 };
    for (i = 0; i < MASK; ++i){
        matR[i][i] = matT[i][i];
        for (j = i + 1; j < MASK; ++j){
            matR[i][j] = (BYTE)rand();
            matR[j][i] = matT[j][i] ^ matR[i][j] ^ matT[i][j];
#if SIZE_A
            /* re-evaluate R(0,j)
             */
            if (i == 0){
                int dim[4] = { 1, BITS, SIZE_A, SIZE_A };
                BYTE old = matR[0][j];
                Res res = multiplyMat(&matR[0][j], &old, matA, dim);
                CHECK(res);
            }
#endif
        }
    }

    /* get the final matrix
     */
    for (i = 0; i < MASK; ++i){
        mlRes[i] = matR[0][i];
        for (j = 1; j < MASK; ++j){
            mlRes[i] ^= matR[j][i];
        }
    }

    return RES_OK;
}

Res invGFMasked(BYTE *inversed, const BYTE *x){
    Res res = RES_OK;
    BYTE z[MASK] = { 0 };
    BYTE y[MASK] = { 0 };
    BYTE w[MASK] = { 0 };
    BYTE tem[MASK] = { 0 };
    //2
    res = multiplyGFMasked(z, x, x); CHECK(res);
    res = multiplyGFMasked(y, (const BYTE*)z, x); CHECK(res);
    //4
    res = multiplyGFMasked(tem, (const BYTE*)y, (const BYTE*)y); CHECK(res);
    res = multiplyGFMasked(w, (const BYTE*)tem, (const BYTE*)tem); CHECK(res);

    res = multiplyGFMasked(tem, (const BYTE*)y, (const BYTE*)w); CHECK(res);
    //16
    res = multiplyGFMasked(y, (const BYTE*)tem, (const BYTE*)tem); CHECK(res);
    res = multiplyGFMasked(tem, (const BYTE*)y, (const BYTE*)y); CHECK(res);
    res = multiplyGFMasked(y, (const BYTE*)tem, (const BYTE*)tem); CHECK(res);
    res = multiplyGFMasked(tem, (const BYTE*)y, (const BYTE*)y); CHECK(res);

    res = multiplyGFMasked(y, (const BYTE*)tem, (const BYTE*)w); CHECK(res);
    res = multiplyGFMasked(inversed, (const BYTE*)y, (const BYTE*)z);
    return res;
}


Res encode(BYTE *encoded, const BYTE *inputs){
    Res res = RES_OK;
    const int dims[4] = { 1, BITS, SIZE_A, SIZE_A };
    BYTE sumRest[NB*WORD_SIZE] = { 0 };
    if (encoded == NULL || inputs == NULL) return RES_INVALID_POINTER;
    int i, j;
    for (i = 1; i < MASK; ++i){
        for (j = 0; j < NB*WORD_SIZE; ++j){
            BYTE tem = (BYTE)rand();
            encoded[j + i * NB*WORD_SIZE] = tem;
            sumRest[j] ^= tem;
        }
    }

#if SIZE_A
    for (j = 0; j < NB*WORD_SIZE; ++j){
        sumRest[j] ^= inputs[j];
        res = multiplyMat(encoded+j, (const BYTE*)sumRest+j, (const BYTE*)matInvA, dims);
        CHECK(res);
    }
#else
    for (j = 0; j < NB*WORD_SIZE; ++j){
        encoded[j] = inputs[j] ^ sumRest[j];
    }
#endif
    return res;
}




Res decode(BYTE *decoded, const BYTE *inputs){
    Res res = RES_OK;
    int i, j;
    const int dims[4] = { 1, BITS, SIZE_A, SIZE_A };
    if (inputs == NULL || decoded == NULL) return RES_INVALID_POINTER;
#if SIZE_A
    for (j = 0; j < NB*WORD_SIZE; ++j){
        res = multiplyMat(decoded+j, inputs+j, (const BYTE*)matA, dims);
        CHECK(res);
    }
#else
    memcpy(decoded, inputs, NB*WORD_SIZE);
    
#endif
    for (i = 1; i < MASK; ++i){
        for (j = 0; j < NB*WORD_SIZE; ++j){
            decoded[j] ^= inputs[j + i * NB*WORD_SIZE];
        }
    }
    return res;
}


#endif /* MASK */

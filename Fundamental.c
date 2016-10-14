//
//  Fundamental.c
//  AES
//
//  Created by benny on 16/4/22.
//

#include "Fundamental.h"

#if SIZE_A
BYTE matA[SIZE_A] = { 0 }, matInvA[SIZE_A] = { 0 }, matTransA[SIZE_A] = { 0 };

static
BYTE *matHat, *matGrave, *matAcute;

static
BYTE multiBigA[SIZE_A * 3][RANGE] = { 0 };

static
BYTE multiA[RANGE] = { 0 };

#endif

const
BYTE lookupTable[RANGE] = POP_CONT;
const
BYTE mod8[] = MOD;
const
BYTE div8[] = DIV;

BYTE coeffA[3][BITS] = { 0 }, coeff[3][BITS] = { MAT_POW(2), MAT_POW(4), MAT_POW(16) };



static
BYTE shiftBit(BYTE orig, int i, int j)
/* Shift bits form j -> i
 */
{
    BYTE tem;
    tem = (UNIT_BYTE >> j) & orig;
    if (j > i) tem <<= (j - i);
    else if (i > j) tem >>= (i - j);
    else return tem;

    return tem;
}

//static
int bytesOfRow(int col)
/* Calculate the # of bytes in one row
 */
{
    int bytes;
    /* bytes of each row :: if dim_col < LENGTH, allocate a byte as well
     */
    if (col < BITS) return 1;
    bytes = div8[col];
    bytes = mod8[col] ? bytes + 1 : bytes;
    return bytes;
}


//_inline
//static
Res tensorProduct(BYTE *tpres, BYTE bytex, BYTE bytey)
/* Tensor Product(kron) of two bytes
 * Return a 1 x 64 vector of bits, i.e., a 8-byte-array
 */
{
    int j;
    if (tpres == NULL) return RES_INVALID_POINTER;
    BYTE unit = UNIT_BYTE;
    for (j = 0; j < 8; ++j){
        tpres[j] = (bytex & unit) ? bytey : 0x00;
        unit >>= 1;
    }
    return RES_OK;
}


//static
Res tensorProductOfMat(BYTE *tpres, const BYTE *matX, const BYTE *matY)
/* Tensor Product of two matrix,
 * Return a (n^2, n^2) matrix, which is the transposed matrix of the result
 */
{
    int i,j,k;
    if (tpres == NULL) return RES_INVALID_POINTER;
    BYTE unit = UNIT_BYTE;

    int dim[2] = {8, 8};

    for (i = 0; i< 8; ++i){
        for (j = 0; j < 8; ++j){
            BYTE sign = matX[j] & unit;
            for (k = 0; k < 8; ++k){
                tpres[(j*8+k) * 8 + i] = sign ? matY[k] : 0x00;
            }
        }
        unit >>= 1;
    }
    return RES_OK;
}


//_inline
//static
Res transpose(BYTE *transRes, const BYTE *matOrig, const int *dims)
/* Transposition
 */
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


BYTE  powGF(BYTE base, int expIndex){
    int dim[4] = { 1, BITS, BITS, BITS };
    Res res = RES_OK;
    BYTE tem = ZERO;
    multiplyMat(&tem, &base, coeff[expIndex], dim);
    return tem;
}


BYTE invGF(BYTE x){
/* Multiplicative inverse of a(x) in GF(2^8)
 */
    if (x == ZERO) return ZERO;
    BYTE z = powGF(x, 0);

    BYTE y = multiplyGF(z, x);

    BYTE w = powGF(y, 1);

    y = multiplyGF(y, w);

    y = powGF(y, 2);

    y = multiplyGF(y, w);
    y = multiplyGF(y, z);
    return y;
}

Res modularProduct(BYTE *mpRes, const BYTE *wordx, const BYTE *wordy, int index){
/* Modular product of a(x) and b(x), where a(x) and b(x) both are four-term polynomials,
 * with coefficients that are finite field elements.
 */
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

Res multiplyMat(BYTE *mlRes, const BYTE *matx, const BYTE *maty, const int *dims){
/* Matrix multiply : mulitplyMat(A, B) = A * B^T
 *  - *dims: points to a array containing the dimension of matrices rx,cx,ry and cy, respectively.
 *  -
 */
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


BYTE multiplyGF(BYTE bytex, BYTE bytey){
/* Multiplication over GF(2^8),and the irreducible polynomial is 0x011b
*/
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

static
Res genAs(){
    Res res = RES_OK;
    int i;
    const int dimsE[4] = { SIZE_A, SIZE_A, SIZE_A, SIZE_A };
    const int dimsB[4] = { SIZE_A, SIZE_A, SIZE_A * SIZE_A, SIZE_A };
    const int dimsT[4] = { SIZE_A * SIZE_A, SIZE_A * SIZE_A, SIZE_A, SIZE_A * SIZE_A };
    BYTE matP[SIZE_A] = { 0 };
    BYTE matE[SIZE_A] = UNIT_MAT;
    BYTE tem[SIZE_A] = { 0 };

    // n^2 * n
    BYTE matRight[SIZE_A * SIZE_A] = { 0 };
    // n^2 * n^2
    BYTE matTensor[SIZE_A * SIZE_A * SIZE_A] = { 0 };

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
        transpose(matP, (const BYTE *)matE, dimsE);
        /*  A = P x A   ==>  A^T = A^T x P^T  */
        memcpy(tem, (const BYTE *)matTransA, SIZE_A);
        multiplyMat(matTransA, (const BYTE *)tem, (const BYTE *)matE, dimsE);
        /*  A^{-1} = A^{-1} x P     */
        memcpy(tem, (const BYTE *)matInvA, SIZE_A);
        multiplyMat(matInvA, (const BYTE *)tem, (const BYTE *)matP, dimsE);

        /* Refresh matE to Unit Matrix */
        matE[i] ^= rowToAdd;
    }

    transpose(matA, (const BYTE *)matTransA, dimsE);

    /* Generate `hatA`, `graveA` and `acuteA`, which are SIZE_A * SIZE_A^2
     */
    // hatA = invA x ( EE x tp(A,A) )
    //      = invA .x ( EE x tp(A,A) )^T
    //      = invA .x ( tp(A,A)^T .x EE)
    //      = invA .x ( tp(A^T,A^T) .x EE)
    BYTE matUnit[SIZE_A] = UNIT_MAT;
    const BYTE matEE[SIZE_A * SIZE_A] = MAT_EE;

    //allocating
    matHat = (BYTE *)malloc(SIZE_A*SIZE_A*sizeof(BYTE));
    matAcute = (BYTE *)malloc(SIZE_A*SIZE_A*sizeof(BYTE));
    matGrave = (BYTE *)malloc(SIZE_A*SIZE_A*sizeof(BYTE) );

    tensorProductOfMat(matTensor, (const BYTE*)matTransA, (const BYTE*)matTransA);
    multiplyMat(matRight, (const BYTE *)matTensor, matEE, dimsT);
    multiplyMat(matHat, (const BYTE *)matInvA, (const BYTE *)matRight, dimsB);

    tensorProductOfMat(matTensor, (const BYTE*)matTransA, (const BYTE*)matUnit);
    multiplyMat(matRight, (const BYTE *)matTensor, matEE, dimsT);
    multiplyMat(matGrave, (const BYTE *)matInvA, (const BYTE *)matRight, dimsB);

    tensorProductOfMat(matTensor, (const BYTE*)matUnit, (const BYTE*)matTransA);
    multiplyMat(matRight, (const BYTE *)matTensor, matEE, dimsT);
    multiplyMat(matAcute, (const BYTE *)matInvA, (const BYTE *)matRight, dimsB);

    return res;
}





Res setup4MultiTable(
    )
{

    int tabIndex, itr;
    int matIndex;
    BYTE val;
    BYTE * const mat[3] = { matHat, matGrave, matAcute };
    const int tabs = SIZE_A;

    /* look up table for matHat, matGrave, matAcute multiplication
     */
    for (matIndex = 0; matIndex < 3; ++matIndex){
        for (tabIndex = 0; tabIndex < tabs; ++tabIndex){
            int currTab = matIndex * tabs + tabIndex;
            for (val = 0x00, itr = 0; itr < RANGE; ++val, ++itr){
                /* Multiplying */
                int row;
                BYTE *ptrMat = mat[matIndex] + tabIndex;
                for (row = 0; row < SIZE_A; ++row, ptrMat += tabs)
                {
                    BYTE temp;
                    temp = lookupTable[val & (*ptrMat)] >> row;
                    if (temp)
                        multiBigA[currTab][val] ^= temp;
                }
            }
        }
    }

    /* look up table for matA multiplication
     */
    for (val = 0x00, itr = 0; itr < RANGE; ++val, ++itr){
        int row;
        BYTE *ptrMat = matA;
        for (row = 0; row < SIZE_A; ++row, ++ptrMat)
        {
            BYTE temp;
            temp = lookupTable[val & (*ptrMat)] >> row;
            if (temp)
                multiA[val] ^= temp;
        }
    }

    return RES_OK;
}



Res setup4Fundamental(){
    int i;

    genAs();

    BYTE tem[BITS] = { 0 };
    const int dimsCoeff[4] = { BITS, BITS, SIZE_A, SIZE_A };
    for (i = 0; i < 3; ++i){
        multiplyMat(tem, matTransA, coeff[i], dimsCoeff);
        multiplyMat(coeffA[i], matInvA, tem, dimsCoeff);
    }

    setup4MultiTable();

    //free matHat, matAcute, matGrave
    free(matHat);
    free(matAcute);
    free(matGrave);

    return RES_OK;
}


BYTE multiplyTable(
    const BYTE *vectX,
    BYTE index // the index of hat(0), grave(1), or acute(2)
    )
{
    int i;
    BYTE ret = 0x00;
    const int tabs = SIZE_A;
    BYTE realIndex = index * tabs;

    for (i = 0; i < tabs; ++i){
        ret ^= multiBigA[realIndex + i][vectX[i]];
    }
    return ret;
}
#endif /* SIZE_A */


Res multiplyGFMasked(BYTE *mlRes, const BYTE *byteXs, const BYTE *byteYs) {
/* Multiplication over GF(2^8), inputs and outputs are masking bytes
*/
    /* get matrix T
    */
    int i, j;
    BYTE matT[MASK][MASK] = { 0 };
    BYTE matTensorTemp[BITS] = { 0 };
    for (i = 0; i < MASK; ++i){
        for (j = 0; j < MASK; ++j){
#if SIZE_A
            if (i == 0 && j == 0){
                tensorProduct(matTensorTemp, byteXs[i], byteYs[j]);
                matT[i][j] = multiplyTable((const BYTE *)matTensorTemp, 0);
            }
            else if (i == 0){
                tensorProduct(matTensorTemp, byteXs[i], byteYs[j]);
                matT[i][j] = multiplyTable((const BYTE *)matTensorTemp, 1);
            }
            else if (j == 0){
                tensorProduct(matTensorTemp, byteXs[i], byteYs[j]);
                matT[i][j] = multiplyTable((const BYTE *)matTensorTemp, 2);
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
                matR[0][j] = multiA[matR[0][j]];
                //int dim[4] = { 1, BITS, SIZE_A, SIZE_A };
                //BYTE old = matR[0][j];
                //multiplyMat(&matR[0][j], &old, matA, dim);
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


Res  powGFMasked(BYTE *powed, const BYTE *base, int expIndex){
    // expIndex: 0->pow(2), 1->pow(4), 2->pow(16)
    int dim[4] = { 1, BITS, BITS, BITS };

    // update the masks
#if SIZE_A
    multiplyMat(powed, base, coeffA[expIndex], dim);
#else
    multiplyMat(powed, base, coeff[expIndex], dim);
#endif
    int i;
    for (i = 1; i < MASK; ++i){
        multiplyMat(powed + i, base + i, coeff[expIndex], dim);
    }
    return RES_OK;
}

Res invGFMasked(BYTE *inversed, const BYTE *x){
    BYTE z[MASK] = { 0 };
    BYTE y[MASK] = { 0 };
    BYTE w[MASK] = { 0 };
    BYTE tem[MASK] = { 0 };
    // power 2
    powGFMasked(z, x, 0);

    multiplyGFMasked(y, (const BYTE*)z, x);

    // power 4
    powGFMasked(w, (const BYTE*)y, 1);

    multiplyGFMasked(tem, (const BYTE*)y, (const BYTE*)w);

    // power 16
    powGFMasked(y, (const BYTE*)tem, 2);
    multiplyGFMasked(tem, (const BYTE*)y, (const BYTE*)w);
    multiplyGFMasked(inversed, (const BYTE*)tem, (const BYTE*)z);

    return RES_OK;
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
        multiplyMat(encoded + j, (const BYTE*)sumRest + j, (const BYTE*)matInvA, dims);
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
        multiplyMat(decoded + j, inputs + j, (const BYTE*)matA, dims);
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

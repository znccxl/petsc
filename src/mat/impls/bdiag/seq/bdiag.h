
#if !defined(__BDIAG_H)
#define __BDIAG_H
#include "src/mat/matimpl.h"


/*
   Mat_SeqBDiag (MATSEQBDIAG) - block-diagonal format, where each diagonal
   element consists of a square block of size bs  x bs.  Dense storage
   within each block is in column-major order.  The diagonals are the
   full length of the matrix.  As a special case, blocks of size bs=1
   (scalars) are supported as well.
*/

typedef struct {
  int          mblock,nblock;    /* block rows and columns */
  int          nonew;            /* if true, no new nonzeros allowed in matrix */
  int          nonew_diag;       /* if true, no new diagonals allowed in matrix */
  int          nz,maxnz;         /* nonzeros, allocated nonzeros */
  int          nd;               /* number of block diagonals */
  int          mainbd;           /* the number of the main block diagonal */
  int          bs;               /* Each diagonal element is an bs x bs matrix */
  int          *diag;            /* value of (row-col)/bs for each diagonal */
  int          *bdlen;           /* block-length of each diagonal */
  int          ndim;             /* diagonals come from an ndim pde (if 0, ignore) */
  int          ndims[3];         /* sizes of the mesh if ndim > 0 */
  PetscTruth   user_alloc;       /* true if the user provided the diagonals */
  int          *colloc;          /* holds column locations if using MatGetRow */
  PetscScalar  **diagv;          /* The actual diagonals */
  PetscScalar  *dvalue;          /* Used to hold a row if MatGetRow is used */
  int          *pivot;           /* pivots for LU factorization (temporary loc) */
  PetscTruth   roworiented;      /* inputs to MatSetValue() are row oriented (default = 1) */
  int          reallocs;         /* number of allocations during MatSetValues */
} Mat_SeqBDiag;

EXTERN PetscErrorCode MatNorm_SeqBDiag_Columns(Mat,PetscReal*,int);
EXTERN PetscErrorCode MatMult_SeqBDiag_1(Mat,Vec,Vec);
EXTERN PetscErrorCode MatMult_SeqBDiag_2(Mat,Vec,Vec);
EXTERN PetscErrorCode MatMult_SeqBDiag_3(Mat,Vec,Vec);
EXTERN PetscErrorCode MatMult_SeqBDiag_4(Mat,Vec,Vec);
EXTERN PetscErrorCode MatMult_SeqBDiag_5(Mat,Vec,Vec);
EXTERN PetscErrorCode MatMult_SeqBDiag_N(Mat A,Vec,Vec);
EXTERN PetscErrorCode MatMultAdd_SeqBDiag_1(Mat,Vec,Vec,Vec);
EXTERN PetscErrorCode MatMultAdd_SeqBDiag_2(Mat,Vec,Vec,Vec);
EXTERN PetscErrorCode MatMultAdd_SeqBDiag_3(Mat,Vec,Vec,Vec);
EXTERN PetscErrorCode MatMultAdd_SeqBDiag_4(Mat,Vec,Vec,Vec);
EXTERN PetscErrorCode MatMultAdd_SeqBDiag_5(Mat,Vec,Vec,Vec);
EXTERN PetscErrorCode MatMultAdd_SeqBDiag_N(Mat A,Vec,Vec,Vec);
EXTERN PetscErrorCode MatMultTranspose_SeqBDiag_1(Mat,Vec,Vec);
EXTERN PetscErrorCode MatMultTranspose_SeqBDiag_N(Mat A,Vec,Vec);
EXTERN PetscErrorCode MatMultTransposeAdd_SeqBDiag_1(Mat,Vec,Vec,Vec);
EXTERN PetscErrorCode MatMultTransposeAdd_SeqBDiag_N(Mat A,Vec,Vec,Vec);
EXTERN PetscErrorCode MatSetValues_SeqBDiag_1(Mat,int,const int [],int,const int [],const PetscScalar [],InsertMode);
EXTERN PetscErrorCode MatSetValues_SeqBDiag_N(Mat,int,const int [],int,const int [],const PetscScalar [],InsertMode);
EXTERN PetscErrorCode MatGetValues_SeqBDiag_1(Mat,int,const int [],int,const int [],PetscScalar []);
EXTERN PetscErrorCode MatGetValues_SeqBDiag_N(Mat,int,const int [],int,const int [],PetscScalar []);
EXTERN PetscErrorCode MatRelax_SeqBDiag_1(Mat,Vec,PetscReal,MatSORType,PetscReal,int,int,Vec);
EXTERN PetscErrorCode MatRelax_SeqBDiag_N(Mat,Vec,PetscReal,MatSORType,PetscReal,int,int,Vec);
EXTERN PetscErrorCode MatView_SeqBDiag(Mat,PetscViewer);
EXTERN PetscErrorCode MatGetInfo_SeqBDiag(Mat,MatInfoType,MatInfo*);
EXTERN PetscErrorCode MatGetRow_SeqBDiag(Mat,int,int *,int **,PetscScalar **);
EXTERN PetscErrorCode MatRestoreRow_SeqBDiag(Mat,int,int *,int **,PetscScalar **);
EXTERN PetscErrorCode MatTranspose_SeqBDiag(Mat,Mat *);
EXTERN PetscErrorCode MatNorm_SeqBDiag(Mat,NormType,PetscReal *);
EXTERN PetscErrorCode MatLUFactorSymbolic_SeqBDiag(Mat,IS,IS,MatFactorInfo*,Mat*);
EXTERN PetscErrorCode MatILUFactorSymbolic_SeqBDiag(Mat,IS,IS,MatFactorInfo*,Mat*);
EXTERN PetscErrorCode MatILUFactor_SeqBDiag(Mat,IS,IS,MatFactorInfo*);
EXTERN PetscErrorCode MatLUFactorNumeric_SeqBDiag_N(Mat,Mat*);
EXTERN PetscErrorCode MatLUFactorNumeric_SeqBDiag_1(Mat,Mat*);
EXTERN PetscErrorCode MatSolve_SeqBDiag_1(Mat,Vec,Vec);
EXTERN PetscErrorCode MatSolve_SeqBDiag_2(Mat,Vec,Vec);
EXTERN PetscErrorCode MatSolve_SeqBDiag_3(Mat,Vec,Vec);
EXTERN PetscErrorCode MatSolve_SeqBDiag_4(Mat,Vec,Vec);
EXTERN PetscErrorCode MatSolve_SeqBDiag_5(Mat,Vec,Vec);
EXTERN PetscErrorCode MatSolve_SeqBDiag_N(Mat,Vec,Vec);
EXTERN PetscErrorCode MatLoad_SeqBDiag(PetscViewer,const MatType,Mat*);
EXTERN PetscErrorCode MatGetRow_MPIBDiag(Mat,int,int*,int**,PetscScalar**);
EXTERN PetscErrorCode MatRestoreRow_MPIBDiag(Mat,int,int*,int**,PetscScalar**);

#endif

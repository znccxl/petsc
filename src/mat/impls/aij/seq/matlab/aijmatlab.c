/* 
        Provides an interface for the Matlab engine sparse solver

*/
#include "src/mat/impls/aij/seq/aij.h"

#include "engine.h"   /* Matlab include file */
#include "mex.h"      /* Matlab include file */

typedef struct {
  int (*MatDuplicate)(Mat,MatDuplicateOption,Mat*);
  int (*MatView)(Mat,PetscViewer);
  int (*MatLUFactorSymbolic)(Mat,IS,IS,MatFactorInfo*,Mat*);
  int (*MatILUDTFactor)(Mat,MatFactorInfo*,IS,IS,Mat*);
  int (*MatDestroy)(Mat);
} Mat_Matlab;


EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "MatMatlabEnginePut_Matlab"
PetscErrorCode MatMatlabEnginePut_Matlab(PetscObject obj,void *mengine)
{
  PetscErrorCode ierr;
  Mat         B = (Mat)obj;
  mxArray     *mat; 
  Mat_SeqAIJ  *aij = (Mat_SeqAIJ*)B->data;

  PetscFunctionBegin;
  mat  = mxCreateSparse(B->n,B->m,aij->nz,mxREAL);
  ierr = PetscMemcpy(mxGetPr(mat),aij->a,aij->nz*sizeof(PetscScalar));CHKERRQ(ierr);
  /* Matlab stores by column, not row so we pass in the transpose of the matrix */
  ierr = PetscMemcpy(mxGetIr(mat),aij->j,aij->nz*sizeof(int));CHKERRQ(ierr);
  ierr = PetscMemcpy(mxGetJc(mat),aij->i,(B->m+1)*sizeof(int));CHKERRQ(ierr);

  /* Matlab indices start at 0 for sparse (what a surprise) */
  
  ierr = PetscObjectName(obj);CHKERRQ(ierr);
  engPutVariable((Engine *)mengine,obj->name,mat);
  PetscFunctionReturn(0);
}
EXTERN_C_END

EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "MatMatlabEngineGet_Matlab"
PetscErrorCode MatMatlabEngineGet_Matlab(PetscObject obj,void *mengine)
{
  PetscErrorCode ierr;
  int        ii;
  Mat        mat = (Mat)obj;
  Mat_SeqAIJ *aij = (Mat_SeqAIJ*)mat->data;
  mxArray    *mmat; 

  PetscFunctionBegin;
  ierr = PetscFree(aij->a);CHKERRQ(ierr);

  mmat = engGetVariable((Engine *)mengine,obj->name);

  aij->nz           = (mxGetJc(mmat))[mat->m];
  ierr              = PetscMalloc(((size_t) aij->nz)*(sizeof(int)+sizeof(PetscScalar))+(mat->m+1)*sizeof(int),&aij->a);CHKERRQ(ierr);
  aij->j            = (int*)(aij->a + aij->nz);
  aij->i            = aij->j + aij->nz;
  aij->singlemalloc = PETSC_TRUE;
  aij->freedata     = PETSC_TRUE;

  ierr = PetscMemcpy(aij->a,mxGetPr(mmat),aij->nz*sizeof(PetscScalar));CHKERRQ(ierr);
  /* Matlab stores by column, not row so we pass in the transpose of the matrix */
  ierr = PetscMemcpy(aij->j,mxGetIr(mmat),aij->nz*sizeof(int));CHKERRQ(ierr);
  ierr = PetscMemcpy(aij->i,mxGetJc(mmat),(mat->m+1)*sizeof(int));CHKERRQ(ierr);

  for (ii=0; ii<mat->m; ii++) {
    aij->ilen[ii] = aij->imax[ii] = aij->i[ii+1] - aij->i[ii];
  }

  ierr = MatAssemblyBegin(mat,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(mat,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}
EXTERN_C_END

EXTERN_C_BEGIN
#undef __FUNCT__
#define __FUNCT__ "MatConvert_Matlab_SeqAIJ"
PetscErrorCode MatConvert_Matlab_SeqAIJ(Mat A,const MatType type,Mat *newmat) {
  PetscErrorCode ierr;
  Mat        B=*newmat;
  Mat_Matlab *lu=(Mat_Matlab*)A->spptr;

  PetscFunctionBegin;
  if (B != A) {
    ierr = MatDuplicate(A,MAT_COPY_VALUES,&B);CHKERRQ(ierr);
  }
  A->ops->duplicate        = lu->MatDuplicate;
  A->ops->view             = lu->MatView;
  A->ops->lufactorsymbolic = lu->MatLUFactorSymbolic;
  A->ops->iludtfactor      = lu->MatILUDTFactor;
  A->ops->destroy          = lu->MatDestroy;
    
  ierr = PetscFree(lu);CHKERRQ(ierr);
  ierr = PetscObjectChangeTypeName((PetscObject)B,MATSEQAIJ);CHKERRQ(ierr);
  *newmat = B;
  PetscFunctionReturn(0);
}
EXTERN_C_END

#undef __FUNCT__  
#define __FUNCT__ "MatDestroy_Matlab"
PetscErrorCode MatDestroy_Matlab(Mat A) 
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = MatConvert_Matlab_SeqAIJ(A,MATSEQAIJ,&A);CHKERRQ(ierr);
  ierr = (*A->ops->destroy)(A);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatSolve_Matlab"
PetscErrorCode MatSolve_Matlab(Mat A,Vec b,Vec x)
{
  PetscErrorCode ierr;
  char            *_A,*_b,*_x;

  PetscFunctionBegin;
  /* make sure objects have names; use default if not */
  ierr = PetscObjectName((PetscObject)b);CHKERRQ(ierr);
  ierr = PetscObjectName((PetscObject)x);CHKERRQ(ierr);

  ierr = PetscObjectGetName((PetscObject)A,&_A);CHKERRQ(ierr);
  ierr = PetscObjectGetName((PetscObject)b,&_b);CHKERRQ(ierr);
  ierr = PetscObjectGetName((PetscObject)x,&_x);CHKERRQ(ierr);
  ierr = PetscMatlabEnginePut(PETSC_MATLAB_ENGINE_(A->comm),(PetscObject)b);CHKERRQ(ierr);
  ierr = PetscMatlabEngineEvaluate(PETSC_MATLAB_ENGINE_(A->comm),"%s = u%s\\(l%s\\(p%s*%s));",_x,_A,_A,_A,_b);CHKERRQ(ierr);
  ierr = PetscMatlabEngineEvaluate(PETSC_MATLAB_ENGINE_(A->comm),"%s = 0;",_b);CHKERRQ(ierr);
  /* ierr = PetscMatlabEnginePrintOutput(PETSC_MATLAB_ENGINE_(A->comm),stdout);CHKERRQ(ierr);  */
  ierr = PetscMatlabEngineGet(PETSC_MATLAB_ENGINE_(A->comm),(PetscObject)x);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatLUFactorNumeric_Matlab"
PetscErrorCode MatLUFactorNumeric_Matlab(Mat A,Mat *F)
{
  Mat_SeqAIJ      *f = (Mat_SeqAIJ*)(*F)->data;
  PetscErrorCode ierr;
  size_t          len;
  char            *_A,*name;

  PetscFunctionBegin;
  ierr = PetscMatlabEnginePut(PETSC_MATLAB_ENGINE_(A->comm),(PetscObject)A);CHKERRQ(ierr);
  _A   = A->name;
  ierr = PetscMatlabEngineEvaluate(PETSC_MATLAB_ENGINE_(A->comm),"[l_%s,u_%s,p_%s] = lu(%s',%g);",_A,_A,_A,_A,f->lu_dtcol);CHKERRQ(ierr);
  ierr = PetscMatlabEngineEvaluate(PETSC_MATLAB_ENGINE_(A->comm),"%s = 0;",_A);CHKERRQ(ierr);
  ierr = PetscStrlen(_A,&len);CHKERRQ(ierr);
  ierr = PetscMalloc((len+2)*sizeof(char),&name);CHKERRQ(ierr);
  sprintf(name,"_%s",_A);
  ierr = PetscObjectSetName((PetscObject)*F,name);CHKERRQ(ierr);
  ierr = PetscFree(name);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatLUFactorSymbolic_Matlab"
PetscErrorCode MatLUFactorSymbolic_Matlab(Mat A,IS r,IS c,MatFactorInfo *info,Mat *F)
{
  PetscErrorCode ierr;
  Mat_SeqAIJ *f;

  PetscFunctionBegin;
  if (A->N != A->M) SETERRQ(PETSC_ERR_ARG_SIZ,"matrix must be square"); 
  ierr                       = MatCreate(A->comm,A->m,A->n,A->m,A->n,F);CHKERRQ(ierr);
  ierr                       = MatSetType(*F,A->type_name);CHKERRQ(ierr);
  ierr                       = MatSeqAIJSetPreallocation(*F,0,PETSC_NULL);CHKERRQ(ierr);
  (*F)->ops->solve           = MatSolve_Matlab;
  (*F)->ops->lufactornumeric = MatLUFactorNumeric_Matlab;
  (*F)->factor               = FACTOR_LU;
  f                          = (Mat_SeqAIJ*)(*F)->data;
  f->lu_dtcol = info->dtcol;
  PetscFunctionReturn(0);
}

/* ---------------------------------------------------------------------------------*/
#undef __FUNCT__  
#define __FUNCT__ "MatSolve_Matlab_QR"
PetscErrorCode MatSolve_Matlab_QR(Mat A,Vec b,Vec x)
{
  PetscErrorCode ierr;
  char            *_A,*_b,*_x;

  PetscFunctionBegin;
  /* make sure objects have names; use default if not */
  ierr = PetscObjectName((PetscObject)b);CHKERRQ(ierr);
  ierr = PetscObjectName((PetscObject)x);CHKERRQ(ierr);

  ierr = PetscObjectGetName((PetscObject)A,&_A);CHKERRQ(ierr);
  ierr = PetscObjectGetName((PetscObject)b,&_b);CHKERRQ(ierr);
  ierr = PetscObjectGetName((PetscObject)x,&_x);CHKERRQ(ierr);
  ierr = PetscMatlabEnginePut(PETSC_MATLAB_ENGINE_(A->comm),(PetscObject)b);CHKERRQ(ierr);
  ierr = PetscMatlabEngineEvaluate(PETSC_MATLAB_ENGINE_(A->comm),"%s = r%s\\(r%s'\\(%s*%s));",_x,_A,_A,_A+1,_b);CHKERRQ(ierr);
  ierr = PetscMatlabEngineEvaluate(PETSC_MATLAB_ENGINE_(A->comm),"%s = 0;",_b);CHKERRQ(ierr);
  /* ierr = PetscMatlabEnginePrintOutput(PETSC_MATLAB_ENGINE_(A->comm),stdout);CHKERRQ(ierr);  */
  ierr = PetscMatlabEngineGet(PETSC_MATLAB_ENGINE_(A->comm),(PetscObject)x);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatLUFactorNumeric_Matlab_QR"
PetscErrorCode MatLUFactorNumeric_Matlab_QR(Mat A,Mat *F)
{
  PetscErrorCode ierr;
  size_t          len;
  char            *_A,*name;

  PetscFunctionBegin;
  ierr = PetscMatlabEnginePut(PETSC_MATLAB_ENGINE_(A->comm),(PetscObject)A);CHKERRQ(ierr);
  _A   = A->name;
  ierr = PetscMatlabEngineEvaluate(PETSC_MATLAB_ENGINE_(A->comm),"r_%s = qr(%s');",_A,_A);CHKERRQ(ierr);
  ierr = PetscStrlen(_A,&len);CHKERRQ(ierr);
  ierr = PetscMalloc((len+2)*sizeof(char),&name);CHKERRQ(ierr);
  sprintf(name,"_%s",_A);
  ierr = PetscObjectSetName((PetscObject)*F,name);CHKERRQ(ierr);
  ierr = PetscFree(name);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatLUFactorSymbolic_Matlab_QR"
PetscErrorCode MatLUFactorSymbolic_Matlab_QR(Mat A,IS r,IS c,MatFactorInfo *info,Mat *F)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (A->N != A->M) SETERRQ(PETSC_ERR_ARG_SIZ,"matrix must be square"); 
  ierr                       = MatCreate(A->comm,A->m,A->n,A->m,A->n,F);CHKERRQ(ierr);
  ierr                       = MatSetType(*F,A->type_name);CHKERRQ(ierr);
  ierr                       = MatSeqAIJSetPreallocation(*F,0,PETSC_NULL);CHKERRQ(ierr);
  (*F)->ops->solve           = MatSolve_Matlab_QR;
  (*F)->ops->lufactornumeric = MatLUFactorNumeric_Matlab_QR;
  (*F)->factor               = FACTOR_LU;
  (*F)->assembled            = PETSC_TRUE;  /* required by -ksp_view */

  PetscFunctionReturn(0);
}

/* --------------------------------------------------------------------------------*/
#undef __FUNCT__  
#define __FUNCT__ "MatILUDTFactor_Matlab"
PetscErrorCode MatILUDTFactor_Matlab(Mat A,MatFactorInfo *info,IS isrow,IS iscol,Mat *F)
{
  PetscErrorCode ierr;
  size_t     len;
  char       *_A,*name;

  PetscFunctionBegin;
  if (info->dt == PETSC_DEFAULT)      info->dt      = .005;
  if (info->dtcol == PETSC_DEFAULT)   info->dtcol   = .01;
  if (A->N != A->M) SETERRQ(PETSC_ERR_ARG_SIZ,"matrix must be square"); 
  ierr                       = MatCreate(A->comm,A->m,A->n,A->m,A->n,F);CHKERRQ(ierr);
  ierr                       = MatSetType(*F,A->type_name);CHKERRQ(ierr);
  ierr                       = MatSeqAIJSetPreallocation(*F,0,PETSC_NULL);CHKERRQ(ierr);
  (*F)->ops->solve           = MatSolve_Matlab;
  (*F)->factor               = FACTOR_LU;
  ierr = PetscMatlabEnginePut(PETSC_MATLAB_ENGINE_(A->comm),(PetscObject)A);CHKERRQ(ierr);
  _A   = A->name;
  ierr = PetscMatlabEngineEvaluate(PETSC_MATLAB_ENGINE_(A->comm),"info_%s = struct('droptol',%g,'thresh',%g);",_A,info->dt,info->dtcol);CHKERRQ(ierr);
  ierr = PetscMatlabEngineEvaluate(PETSC_MATLAB_ENGINE_(A->comm),"[l_%s,u_%s,p_%s] = luinc(%s',info_%s);",_A,_A,_A,_A,_A);CHKERRQ(ierr);
  ierr = PetscMatlabEngineEvaluate(PETSC_MATLAB_ENGINE_(A->comm),"%s = 0;",_A);CHKERRQ(ierr);

  ierr = PetscStrlen(_A,&len);CHKERRQ(ierr);
  ierr = PetscMalloc((len+2)*sizeof(char),&name);CHKERRQ(ierr);
  sprintf(name,"_%s",_A);
  ierr = PetscObjectSetName((PetscObject)*F,name);CHKERRQ(ierr);
  ierr = PetscFree(name);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatFactorInfo_Matlab"
PetscErrorCode MatFactorInfo_Matlab(Mat A,PetscViewer viewer)
{
  PetscErrorCode ierr;
  
  PetscFunctionBegin; 
  ierr = PetscViewerASCIIPrintf(viewer,"Matlab run parameters:  -- not written yet!\n");CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatView_Matlab"
PetscErrorCode MatView_Matlab(Mat A,PetscViewer viewer) {
  PetscErrorCode ierr;
  PetscTruth        iascii;
  PetscViewerFormat format;
  Mat_Matlab        *lu=(Mat_Matlab*)(A->spptr);

  PetscFunctionBegin;
  ierr = (*lu->MatView)(A,viewer);CHKERRQ(ierr);
  ierr = PetscTypeCompare((PetscObject)viewer,PETSC_VIEWER_ASCII,&iascii);CHKERRQ(ierr);
  if (iascii) {
    ierr = PetscViewerGetFormat(viewer,&format);CHKERRQ(ierr);
    if (format == PETSC_VIEWER_ASCII_FACTOR_INFO) {
      ierr = MatFactorInfo_Matlab(A,viewer);
    }
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatDuplicate_Matlab"
PetscErrorCode MatDuplicate_Matlab(Mat A, MatDuplicateOption op, Mat *M) {
  PetscErrorCode ierr;
  Mat_Matlab *lu=(Mat_Matlab*)A->spptr;

  PetscFunctionBegin;
  ierr = (*lu->MatDuplicate)(A,op,M);CHKERRQ(ierr);
  ierr = PetscMemcpy((*M)->spptr,lu,sizeof(Mat_Matlab));CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

EXTERN_C_BEGIN
#undef __FUNCT__
#define __FUNCT__ "MatConvert_SeqAIJ_Matlab"
PetscErrorCode MatConvert_SeqAIJ_Matlab(Mat A,const MatType type,Mat *newmat) {
  /* This routine is only called to convert to MATMATLAB */
  /* from MATSEQAIJ, so we will ignore 'MatType type'. */
  PetscErrorCode ierr;
  Mat        B=*newmat;
  Mat_Matlab *lu;
  PetscTruth qr;

  PetscFunctionBegin;
  if (B != A) {
    ierr = MatDuplicate(A,MAT_COPY_VALUES,&B);CHKERRQ(ierr);
  }

  ierr = PetscNew(Mat_Matlab,&lu);CHKERRQ(ierr);
  lu->MatDuplicate         = A->ops->duplicate;
  lu->MatView              = A->ops->view;
  lu->MatLUFactorSymbolic  = A->ops->lufactorsymbolic;
  lu->MatILUDTFactor       = A->ops->iludtfactor;
  lu->MatDestroy           = A->ops->destroy;

  B->spptr                 = (void*)lu;
  B->ops->duplicate        = MatDuplicate_Matlab;
  B->ops->view             = MatView_Matlab;
  B->ops->lufactorsymbolic = MatLUFactorSymbolic_Matlab;
  B->ops->iludtfactor      = MatILUDTFactor_Matlab;
  B->ops->destroy          = MatDestroy_Matlab;

  ierr = PetscOptionsHasName(A->prefix,"-mat_matlab_qr",&qr);CHKERRQ(ierr);
  if (qr) {
    B->ops->lufactorsymbolic = MatLUFactorSymbolic_Matlab_QR;
    PetscLogInfo(0,"Using Matlab QR with iterative refinement for LU factorization and solves");
  } else {
    PetscLogInfo(0,"Using Matlab for LU factorizations and solves.");
  }
  PetscLogInfo(0,"Using Matlab for ILUDT factorizations and solves.");

  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"MatConvert_seqaij_matlab_C",
                                           "MatConvert_SeqAIJ_Matlab",MatConvert_SeqAIJ_Matlab);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"MatConvert_matlab_seqaij_C",
                                           "MatConvert_Matlab_SeqAIJ",MatConvert_Matlab_SeqAIJ);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"PetscMatlabEnginePut_C",
                                           "MatMatlabEnginePut_Matlab",MatMatlabEnginePut_Matlab);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"PetscMatlabEngineGet_C",
                                           "MatMatlabEngineGet_Matlab",MatMatlabEngineGet_Matlab);CHKERRQ(ierr);
  ierr = PetscObjectChangeTypeName((PetscObject)B,MATMATLAB);CHKERRQ(ierr);
  *newmat = B;
  PetscFunctionReturn(0);
}
EXTERN_C_END

/*MC
  MATMATLAB - MATMATLAB = "matlab" - A matrix type providing direct solvers (LU and QR) and drop tolerance
  based ILU factorization (ILUDT) for sequential matrices via the external package Matlab.

  If Matlab is instaled (see the manual for
  instructions on how to declare the existence of external packages),
  a matrix type can be constructed which invokes Matlab solvers.
  After calling MatCreate(...,A), simply call MatSetType(A,MATMATLAB).
  This matrix type is only supported for double precision real.

  This matrix inherits from MATSEQAIJ.  As a result, MatSeqAIJSetPreallocation is 
  supported for this matrix type.  One can also call MatConvert for an inplace conversion to or from 
  the MATSEQAIJ type without data copy.

  Options Database Keys:
+ -mat_type matlab - sets the matrix type to "matlab" during a call to MatSetFromOptions()
- -mat_matlab_qr   - sets the direct solver to be QR instead of LU

  Level: beginner

.seealso: PCLU
M*/

EXTERN_C_BEGIN
#undef __FUNCT__
#define __FUNCT__ "MatCreate_Matlab"
PetscErrorCode MatCreate_Matlab(Mat A) 
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscObjectChangeTypeName((PetscObject)A,MATMATLAB);CHKERRQ(ierr);
  ierr = MatSetType(A,MATSEQAIJ);CHKERRQ(ierr);
  ierr = MatConvert_SeqAIJ_Matlab(A,MATMATLAB,&A);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END
